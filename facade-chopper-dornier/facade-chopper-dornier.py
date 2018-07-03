"""
Interface between the Dornier chopper simulation and the ESS interface
of a chopper system.
"""

import re
import threading
import time
import twisted
from twisted.internet import reactor
from twisted.internet.protocol import Protocol, connectionDone
from twisted.internet.endpoints import TCP4ClientEndpoint, connectProtocol
import pcaspy
import pcaspy.tools


def log(s):
    print(s)


def log_file(s):
    with open('/var/log/ch', 'a') as f1:
        f1.write(s + "\n")
        f1.flush()


class PickupSignalGenerator(object):
    """
    Generates the chopper pickup signal based on the current chopper frequency.
    Runs in its own thread.
    """

    def __init__(self, chopper, period):
        self.chopper = chopper
        self.period = period

    def start(self):
        self.thr_run = True
        self.thr = threading.Thread(target=self._run)
        self.thr.start()
        log('pickup started')

    def _run(self):
        ts_now = time.time()
        ts_last = ts_now
        ts_p_last = ts_last
        while self.thr_run:
            # Generate the pickup signals since ts_last:
            # Use ts_last as the offset, and generate signals based on the frequency.
            ts_now = time.time()
            f = self.chopper.discs[0].ActSpd
            # We could run into some leap second, so check:
            if ts_now - ts_last > 0 and f > 0:
                # Normalize pickup signals to an absolute base time offset.
                # It is not yet officially specified, what format this is.
                # For now, I choose UNIX time seconds.
                # The pickup signals themselves are stored as relative offsets to this
                # ts_base in microseconds.
                ts_base = int(ts_last)
                dat = [0, ts_base]
                step = 1.0 / f
                t1 = ts_p_last + step
                i_gen = 0
                while t1 < ts_now:
                    dat.append(t1)
                    t1 += step
                    i_gen += 1
                    if len(dat) == self.chopper.tdce_array_size:
                        log('ERROR not enough space for the pickup sinals')
                        break

                if i_gen > 0:
                    ts_p_last = dat[-1]

                for i1 in xrange(2, len(dat)):
                    dat[i1] = int(round(1e6 * (dat[i1] - ts_base)))

                # The first element stores the number of actual pickup signals.
                # This is the total length, minus the two fields for the number of pickup
                # signals, and the ts_base:
                dat[0] = len(dat) - 2

                self.chopper.driver.setParam('TDCE', dat)
                self.chopper.driver.updatePVs()
            else:
                # If we do not generate pickup signals, we use now() as next start:
                ts_p_last = ts_now
            ts_last = ts_now
            time.sleep(self.period)
        log('pickup generator done.')


class APIDevice(object):
    """
    Encapsulates the relaying of calls to twisted main thread.
    """

    def __init__(self, device):
        self.device = device

    def ask_device_status(self):
        reactor.callFromThread(self.device.ask_device_status)

    def command_start(self):
        reactor.callFromThread(self.device.command_start)

    def command_set_Spd(self, disc, val):
        reactor.callFromThread(self.device.command_set_Spd, disc, val)

    def command_set_Phs(self, disc, val):
        reactor.callFromThread(self.device.command_set_Phs, disc, val)

    def command_set_Ratio(self, disc, val):
        reactor.callFromThread(self.device.command_set_Ratio, disc, val)


class FacadeEpicsDriver(pcaspy.Driver):
    """
    Monitors read and write requests via Epics.
    On write, validate requests and issue commands to device.
    On read, just serve the request.
    """

    def __init__(self, chopper, pvdb):
        super(FacadeEpicsDriver, self).__init__()
        self.chopper = chopper
        self.pvdb = pvdb
        for pv in pvdb:
            self.setParamStatus(pv, 0, 0)

    def write(self, pv, value):
        log('Write: {} = {}'.format(pv, value))
        # super(FacadeEpicsDriver, self).write(pv, value)
        disc_no = int(pv.split('.')[0][2:]) if 'ch' in pv else 1
        pv_field = pv.split('.')[1] if 'ch' in pv else pv
        if pv_field == 'CmdS':
            self.update_from_epics_CmdS(value)
        if pv_field == 'Spd':
            self.chopper.discs[disc_no - 1].Spd = value
        if pv_field == 'Phs':
            self.chopper.discs[disc_no - 1].Phs = value
        if pv_field == 'Ratio':
            self.chopper.discs[disc_no - 1].Ratio = value

    def update_from_epics_CmdS(self, data):
        cmds = data.split('\n')
        for cmd in cmds:
            log('cmd: {}'.format(cmd))
            if cmd == 'start':
                self.chopper.command_start()

    def set_api_device_for_epics(self, api_device_for_epics):
        self.api_device_for_epics = api_device_for_epics

    def ask_device_status(self):
        self.api_device_for_epics.ask_device_status()


class ChopperDisc(object):
    """
    Represents data for each chopper disc
    """

    def __init__(self, disc_number):
        self.disc_number = disc_number
        self.api_device = None
        self.driver = None

    def _get_pv_prefix(self):
        return 'ch%d.' % self.disc_number

    def _read_from_driver(self, field):
        return self.driver.read(self._get_pv_prefix() + field)

    def _get_param(self, field):
        return self.driver.getParam(self._get_pv_prefix() + field)

    def _set_param(self, field, value):
        self.driver.setParam(self._get_pv_prefix() + field, value)

    def set_driver(self, driver):
        self.driver = driver

    def set_api_device(self, api_device):
        self.api_device = api_device

    def get_pvdb(self):
        db_base = {
            'State': {
                'type': 'enum',
                'enums': ['MASTER', 'SLAVE'],
                'description': 'State | In Master/Slave state | - | Read',
            },
            'Spd': {
                'type': 'int',
                'unit': 'Hz',
                'lolim': 0,
                'hilim': 20000,
                'description': 'Speed setpoint.  | Hz | Read/Write',
            },
            'Spd-RB': {
                'type': 'int',
                'unit': 'Hz',
                'lolim': 0,
                'hilim': 20000,
                'description': 'Spd-RB  |  Speed read back value | Hz | Read',
            },
            'ActSpd': {
                'type': 'int',
                'unit': 'Hz',
                'lolim': 0,
                'hilim': 20000,
                'description': 'Current rotation speed of the chopper disc. | Hz  | Read',
            },
            'Phs-RB': {
                'type': 'float',
                'unit': 'degree',
                'lolim': -180,
                'hilim': 180,
                'description': 'Phs-RB  |  Current phase of the chopper disc. | Degree | Read',
            },
            'ActPhs': {
                'type': 'float',
                'unit': 'degree',
                'lolim': -180,
                'hilim': 180,
                'description': 'ActPhs  |  Current phase of the chopper disc. | Degree | Read',
            },
            'Phs': {
                'type': 'float',
                'unit': 'degree',
                'lolim': -180,
                'hilim': 180,
                'description': 'Phs  |  Phase setpoint. | Degree | Read/Write',
            },
            'Ratio': {
                'type': 'int',
                'unit': '',
                'lolim': 0,
                'hilim': 5,
                'value': 1,
                'description': 'Ratio | Ratio between the speed of master and slave disc | - | Read'
            },
            'LossCurr': {
                'type': 'float',
                'unit': 'A',
                'description': 'LossCurr | Loss Current | A | Read',
            },
            'Vibration': {
                'type': 'float',
                'unit': 'Hz',
                'description': 'Vibration | Vibration of the chopper | Hz | Read',
            },
            'Temp': {
                'type': 'float',
                'unit': 'C',
                'description': 'Temp | Temperature of the chopper | C | Read',
            },
            'WaterFlow': {
                'type': 'float',
                'unit': 'lit/sec',
                'description': 'Water flow | Water flow | lit/sec | Read',
            },
            'Vacuum': {
                'type': 'float',
                'unit': 'mbar',
                'description': 'Vacuum | Vacuum present | mbar | Read',
            },
            'Valve': {
                'type': 'enum',
                'enums': ['CLOSED', 'OPEN'],
                'description': 'Valve | Valve open/close | - | Read',
            },
            'SumSignal': {
                'type': 'int',
                'unit': '',
                'description': 'Sumsi | Sum of the signal (bool) | - | Read',
            },
        }

        db = {}
        for field in db_base:
            db[self._get_pv_prefix() + field] = db_base[field]

        return db

    @property
    def Spd(self):
        return self._read_from_driver('Spd')

    # Called from the Epics driver write()
    @Spd.setter
    def Spd(self, val):
        if self.State == 0:
            self._set_param('Spd', val)
            self._set_param('Spd-RB', val)
            self.api_device.command_set_Spd(self.disc_number, val)

    @property
    def Phs(self):
        x = self._read_from_driver('Phs')
        log('read from DB: {}'.format(x))
        return x

    @Phs.setter
    def Phs(self, val):
        if self.State == 1:
            self._set_param('Phs', val)
            self._set_param('Phs-RB', val)
            self.api_device.command_set_Phs(self.disc_number, val)

    @property
    def Ratio(self):
        x = self._read_from_driver('Ratio')
        log('read from DB: {}'.format(x))
        return x

    @Ratio.setter
    def Ratio(self, val):
        if self.State == 1:
            self._set_param('Ratio', val)
            self.api_device.command_set_Ratio(self.disc_number, val)

    # Properties which are read-only in Epics:

    @property
    def ActSpd(self):
        return self._get_param('ActSpd')

    @ActSpd.setter
    def ActSpd(self, val):
        self._set_param('ActSpd', val)
        self.driver.updatePVs()

    @property
    def ActPhs(self):
        return self._get_param('ActPhs')

    @ActPhs.setter
    def ActPhs(self, val):
        self._set_param('ActPhs', val)
        self.driver.updatePVs()

    @property
    def State(self):
        return self._get_param('State')

    @State.setter
    def State(self, val):
        self._set_param('State', val)
        self.driver.updatePVs()

    @property
    def LossCurr(self):
        return self._get_param('LossCurr')

    @LossCurr.setter
    def LossCurr(self, val):
        self._set_param('LossCurr', val)
        self.driver.updatePVs()

    @property
    def Vibration(self):
        return self._get_param('Vibration')

    @Vibration.setter
    def Vibration(self, val):
        self._set_param('Vibration', val)
        self.driver.updatePVs()

    @property
    def Temp(self):
        return self._get_param('Temp')

    @Temp.setter
    def Temp(self, val):
        self._set_param('Temp', val)
        self.driver.updatePVs()

    @property
    def WaterFlow(self):
        return self._get_param('WaterFlow')

    @WaterFlow.setter
    def WaterFlow(self, val):
        self._set_param('WaterFlow', val)
        self.driver.updatePVs()

    @property
    def Vacuum(self):
        return self._get_param('Vacuum')

    @Vacuum.setter
    def Vacuum(self, val):
        self._set_param('Vacuum', val)
        self.driver.updatePVs()

    @property
    def Valve(self):
        return self._get_param('Valve')

    @Valve.setter
    def Valve(self, val):
        self._set_param('Valve', val)
        self.driver.updatePVs()

    @property
    def SumSignal(self):
        return self._get_param('SumSignal')

    @SumSignal.setter
    def SumSignal(self, val):
        self._set_param('SumSignal', val)
        self.driver.updatePVs()


class Chopper(object):
    """
    Sets up the Epics front-end with PVs.
    """

    # Not yet in Plankton either:
    # | TDCE*  |  Vector of TDC (top dead center) events in last accelerator pulse. | to be determined | Read |
    # | Dir-RB*  |  Enum for rotation direction (clockwise, counter clockwise). | - | Read |
    # | Dir*  |  Desired rotation direction. (clockwise, counter clockwise). | - | Read/Write |

    def __init__(self, chopper_name, no_discs, chopper_freq_max_hz,
                 pickup_update_interval_seconds):
        self.chopper_name = chopper_name
        self.chopper_freq_max_hz = chopper_freq_max_hz
        self.pickup_update_interval_seconds = pickup_update_interval_seconds
        self.tdce_array_size = int(
            1.0 * self.chopper_freq_max_hz * self.pickup_update_interval_seconds)

        pvdb = {
            'CmdS': {
                'type': 'string',
                'description': 'CmdS  |  String field to accept commands. | - | Read/Write',
            },
            'CmdL': {
                'type': 'string',
                'description': 'CmdL  |  String field with last command. | - | Read',
            },
            'TDCE': {
                # TODO:  Does Epics guarantee a bit width of its long type?
                'type': 'int',
                'count': self.tdce_array_size,
            },
        }

        self.discs = []
        for d in range(no_discs):
            disc = ChopperDisc(d + 1)
            self.discs.append(disc)
            pvdb.update(disc.get_pvdb())

        self.server = pcaspy.SimpleServer()
        self.server.createPV(self.chopper_name + ':', pvdb)
        self.driver = FacadeEpicsDriver(self, pvdb)

        for d in self.discs:
            d.set_driver(self.driver)

    def start(self):
        # process CA transactions
        self.server_thread = pcaspy.tools.ServerThread(self.server)
        self.server_thread.start()

        self.pickup_gen = PickupSignalGenerator(self,
                                                self.pickup_update_interval_seconds)
        self.pickup_gen.start()
        return

        i1 = 0
        while False:
            # driver.setParam("pv01", i1)
            # Post the update event:
            # driver.updatePVs()
            # server.process(0.1)
            i1 += 1

    def stop(self):
        log('Chopper::stop')
        self.pickup_gen.thr_run = False
        self.server_thread.stop()

    def set_api_device(self, api_device):
        self.api_device = api_device
        self.driver.set_api_device_for_epics(api_device)
        for d in self.discs:
            d.set_api_device(api_device)

    def ask_device_status(self):
        self.driver.ask_device_status()

    def command_start(self):
        self.api_device.command_start()


class DornierProtocol(twisted.internet.protocol.Protocol):
    """
    Communicates with the TCP device.
    Parses the responses and encapsulates sending of commands.
    """

    def __init__(self, chopper):
        self.chopper = chopper
        self.report_status_parse_success = True

    def connectionMade(self):
        log('GOOD Connection to chopper TCP device established.')
        reactor.callLater(5, self.chopper.ask_device_status)

    def connectionLost(self, reason=connectionDone):
        log('WARNING connectionLost: {}'.format(reason))
        return

    """
    Called for every line in the status answer of the chopper.
    """

    def parse_status_line(self, line):
        rx = ('(.+?);'
              'state (async|synch);'
              'amode([ a-zA-Z0-9]+?);'
              'nspee([ 0-9]+?);'
              'aspee([ 0-9]+?);'
              'nphas([ 0-9.]+?);'
              'dphas([ 0-9.]+?);'
              'averl([ 0-9.]+?);'
              'spver([ 0-9]+?);'
              'ratio([ 0-9]+?);'
              'no_action(.*?);'
              'monit_(\\d+?);'
              'vibra([ \\d.]+?);'
              't_cho([ \\d.]+?);'
              'durch([ \\d.]+?);'
              'vakum([ \\d.]+?);'
              'valve([ \\d]+?);'
              'sumsi([ \\d]+?);')
        m = re.search(rx, line)
        if not m:
            return None
        return m

    def parse_status(self, data):
        # Test first if this is a chopper status output:
        matched = []
        for line in data.split('\r\n'):
            m = self.parse_status_line(line.strip())
            if m:
                matched.append(m)
        if len(matched) == len(self.chopper.discs):
            if self.report_status_parse_success:
                log('GOOD parsed chopper status the first time')
                self.report_status_parse_success = False
            for d in range(len(self.chopper.discs)):
                state = matched[d].group(2).lower()
                self.chopper.discs[d].State = 0 if state == 'async' else 1
                self.chopper.discs[d].ActSpd = float(matched[d].group(5))
                self.chopper.discs[d].ActPhs = float(matched[d].group(7))
                self.chopper.discs[d].LossCurr = float(matched[d].group(8))
                self.chopper.discs[d].Vibration = float(matched[d].group(13))
                self.chopper.discs[d].Temp = float(matched[d].group(14))
                self.chopper.discs[d].WaterFlow = float(matched[d].group(15))
                self.chopper.discs[d].Vacuum = float(matched[d].group(16))
                self.chopper.discs[d].Valve = int(matched[d].group(17))
                self.chopper.discs[d].SumSignal = float(matched[d].group(18))

            reactor.callLater(5, self.chopper.ask_device_status)

    def dataReceived(self, data):
        # self.transport.loseConnection()
        self.parse_status(data)

    def command_start(self):
        self.transport.write('start\r\n')

    def command_set_Spd(self, disc, val):
        """
        Called when the Epics DB is updated with a new set point.
        """
        self.transport.write('nspee {} {:.0f}\r\n'.format(disc, val))

    def command_set_Phs(self, disc, val):
        """
        Called if new set point in Epics db, but does not currently cause
        a phase change because the simulator does not yet support it.
        """
        x = 'nphas {} {:.3f}\r\n'.format(disc, val)
        log(x)
        self.transport.write(x)
        # Only for chopper 2 in slave mode
        # TODO  check if it is in slave mode

    def command_set_Ratio(self, disc, val):
        """
        Called when Epics is set with a new ratio
        """
        x = 'ratio {} {:.0f}\r\n'.format(disc, val)
        log(x)
        self.transport.write(x)
        # Only for chopper 2 in slave mode
        # TODO  check if it is in slave mode

    def ask_device_status(self):
        """
        Regularly called to retrieve the chopper status.
        """
        self.transport.write('asyst 1\r\n')


class Facade(object):
    """
    Main entry class.
    Initializes the TCP connection to the hardware and initializes the chopper
    representation instance.
    """

    def __init__(self, chopper_name, device_host, device_port, discs, freq_max,
                 pickup_update_interval):
        chopper = Chopper(chopper_name, discs, freq_max,
                          pickup_update_interval)
        protocol = DornierProtocol(chopper)
        api_device = APIDevice(protocol)
        chopper.set_api_device(api_device)
        self.endpoint = TCP4ClientEndpoint(reactor, device_host, device_port)
        self.protocol = protocol
        self.chopper = chopper

    def start(self):
        self.chopper.start()
        self.run()

    def protocol_up(self):
        log('protocol_up')

    def run(self):
        d = connectProtocol(self.endpoint, self.protocol)
        d.addCallback(self.protocol_up)
        reactor.run()
        log('reactor done.')

    def stop(self):
        try:
            log('stopping reactor')
            reactor.stop()
        except:
            log('Reactor did not shut down cleanly, but we do not care.')
        try:
            log('stopping chopper')
            self.chopper.stop()
        except:
            log('Chopper did not shut down cleanly, but we do not care.')


"""
Signal handler to allow to stop the program with SIGINT or SIGTERM (systemd).
"""
handled_shutdown = False


def handle_shutdown():
    global handled_shutdown
    if handled_shutdown:
        log('already handled_shutdown')
        return
    facade.stop()
    handled_shutdown = True


def handle_SIGINT(signum, frame):
    log('handle_SIGINT')
    handle_shutdown()


def handle_SIGTERM(signum, frame):
    log('handle_SIGTERM')
    handle_shutdown()


def handle_SIGHUP(signum, frame):
    log('handle_SIGHUP')
    handle_shutdown()


import signal

signal.signal(signal.SIGINT, handle_SIGINT)
signal.signal(signal.SIGTERM, handle_SIGTERM)
signal.signal(signal.SIGHUP, handle_SIGHUP)

if __name__ == '__main__':
    import argparse

    ap = argparse.ArgumentParser()
    ap.add_argument('-n', '--chopper-name', required=True,
                    help='Used as prefix for the epics PVs')
    ap.add_argument('-a', '--device-addr', default='localhost:60000',
                    help='[host]:[port] of the device, default localhost:60000')
    ap.add_argument('-d', '--discs', default=1, help='Number of chopper discs')

    # Have to choose some length for the Epics array which stores the pickup signals.
    # We make this decision based on the update time period of that PV and the maximum
    # chopper frequency.
    ap.add_argument('--freq-max', default=100,
                    help='Maximum possible chopper frequency (Hz), used to size the required Epics array.  Default 100.')
    ap.add_argument('--pickup-update-interval', default=1.33,
                    help='Interval for pickup signal updates in the Epics DB (seconds, float), used to size the required Epics array.  Default 1.33')
    args = ap.parse_args()

    # Extract hostname and port from the argument
    m = re.match('^(.*?)(|:(\\d*))$', args.device_addr)
    device_host = m.group(1)
    device_port = 60000
    if m.group(3):
        device_port = int(m.group(3))

    # Get the number of discs
    discs = int(args.discs)
    if discs < 1:
        discs = 1

    facade = Facade(args.chopper_name, device_host, device_port, discs,
                    args.freq_max, args.pickup_update_interval)
    facade.start()
    log('main done.')

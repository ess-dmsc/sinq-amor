"""
Interface between the Dornier chopper simulation and the ESS interface
of a chopper system.
"""

import re
import threading
import time
import twisted
from twisted.internet import reactor
from twisted.internet.protocol import Protocol
from twisted.internet.endpoints import TCP4ClientEndpoint, connectProtocol
import pcaspy
import pcaspy.tools


"""
Generates the chopper pickup signal based on the current chopper frequency.
Runs in its own thread.
"""
class PickupSignalGenerator(object):

    def __init__(self, chopper, period):
        self.chopper = chopper
        self.period = period

    def start(self):
        self.thr_run = True
        self.thr = threading.Thread(target=self._run)
        self.thr.start()

    def _run(self):
        ts_now = time.time()
        ts_last = ts_now
        ts_p_last = ts_last
        while self.thr_run:
            # Generate the pickup signals since ts_last:
            # Use ts_last as the offset, and generate signals based on the frequency.
            ts_now = time.time()
            f = self.chopper.driver.getParam('ActSpd')
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
                        print('ERROR not enough space for the pickup sinals')
                        break

                if i_gen > 0:
                    ts_p_last = dat[-1]

                for i1 in xrange(2, len(dat)):
                    dat[i1] = int(round(1e6*(dat[i1] - ts_base)))

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
        print('pickup generator done.')

            


"""
Encapsulates the relaying of calls to twisted main thread.
"""
class APIDevice(object):

    def __init__(self, device):
        self.device = device

    def ask_device_status(self):
        reactor.callFromThread(self.device.ask_device_status)

    def command_start(self):
        reactor.callFromThread(self.device.command_start)

    def command_set_Spd(self, val):
        reactor.callFromThread(self.device.command_set_Spd, val)

    def command_set_Phs(self, val):
        reactor.callFromThread(self.device.command_set_Phs, val)



# Monitors read and write requests via Epics.
# On write, validate requests and issue commands to device.
# On read, just serve the request.
class FacadeEpicsDriver(pcaspy.Driver):

    def  __init__(self, chopper, pvdb):
        super(FacadeEpicsDriver, self).__init__()
        self.chopper = chopper
        self.pvdb = pvdb

    def write(self, pv, value):
        print('Write: {} = {}'.format(pv, value))
        #super(FacadeEpicsDriver, self).write(pv, value)
        if pv == 'CmdS':
            self.update_from_epics_CmdS(value)
        if pv == 'Spd':
            self.chopper.Spd = value
        if pv == 'Phs':
            self.chopper.Phs = value

    def update_from_epics_CmdS(self, data):
        cmds = data.split('\n')
        for cmd in cmds:
            print('cmd: {}'.format(cmd))
            if cmd == 'start':
                self.chopper.command_start()

    def set_api_device_for_epics(self, api_device_for_epics):
        self.api_device_for_epics = api_device_for_epics

    def ask_device_status(self):
         self.api_device_for_epics.ask_device_status()



"""
Sets up the Epics front-end with PVs.
"""
class Chopper(object):

# Not yet in Plankton either:
#| TDCE*  |  Vector of TDC (top dead center) events in last accelerator pulse. | to be determined | Read |
#| Dir-RB*  |  Enum for rotation direction (clockwise, counter clockwise). | - | Read |
#| Dir*  |  Desired rotation direction. (clockwise, counter clockwise). | - | Read/Write |

    def __init__(self, chopper_name, chopper_freq_max_hz, pickup_update_interval_seconds):
        self.chopper_name = chopper_name
        self.chopper_freq_max_hz = chopper_freq_max_hz
        self.pickup_update_interval_seconds = pickup_update_interval_seconds
        self.tdce_array_size = int(1.0 * self.chopper_freq_max_hz * self.pickup_update_interval_seconds)

        self.server = pcaspy.SimpleServer()
        prefix = self.chopper_name + '.'
        # Types
        # Are probably the same as in pyepics package?
        # http://cars9.uchicago.edu/software/python/pyepics3/ca.html#dbrtype-table
        # PVs are compatible with the chopper in Plankton
        # *-RB types are read-back values
        pvdb = {
            'Spd-RB': {
                'type': 'float',
                'description': 'Current rotation speed of the chopper disc. | Hz  | Read',
            },
            # TODO
            # Plankton README defines this again, is it really the same variable?
            'ActSpd': {
                'type': 'float',
                'description': 'Current rotation speed of the chopper disc. | Hz  | Read',
            },
            'Spd': {
                'type': 'float',
                'description': 'Speed setpoint.  | Hz | Read/Write',
            },
            'Phs-RB': {
                'type': 'float',
                'description': 'Phs-RB  |  Current phase of the chopper disc. | Degree | Read',
            },
            'ActPhs': {
                'type': 'float',
                'description': 'ActPhs  |  Current phase of the chopper disc. | Degree | Read',
            },
            'Phs': {
                'type': 'float',
                'description': 'Phs  |  Phase setpoint. | Degree | Read/Write',
            },
            'ParkAng-RB': {
                'type': 'float',
                'description': 'ParkAng-RB  |  Current position of chopper disc if in parked state. | Degree | Read',
            },
            'ParkAng': {
                'type': 'float',
                'description': 'ParkAng  |  Position to which the disc should rotate in parked state. | Degree | Read/Write',
            },
            'AutoPark': {
                'type': 'float',
                'description': 'AutoPark | Enum `false`/`true` (or 0/1). If enabled, the chopper will move to the parking state when the stop state is reached. | - | Read/Write',
            },
            'State': {
                'type': 'float',
                'description': 'State  |  Enum for chopper state. | - | Read',
            },
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
            'pv01': {
                'prec': 4,
                'some-of-my-own-keys': 'does that work?',
            },
            "chr": {
                "type": "char",
                "count": 1024,
                "value": "The big message but longer than the string limit of 40 characters in epics base",
            },
        }
        self.server.createPV(prefix, pvdb)
        self.driver = FacadeEpicsDriver(self, pvdb)

    def start(self):
        # process CA transactions
        self.server_thread = pcaspy.tools.ServerThread(self.server)
        self.server_thread.start()

        self.pickup_gen = PickupSignalGenerator(self, self.pickup_update_interval_seconds)
        self.pickup_gen.start()
        return

        i1 = 0
        while False:
            #driver.setParam("pv01", i1)
            # Post the update event:
            #driver.updatePVs()
            #server.process(0.1)
            i1 += 1

    def stop(self):
        self.pickup_gen.thr_run = False
        self.server_thread.stop()

    def set_api_device(self, api_device):
        self.api_device = api_device
        self.driver.set_api_device_for_epics(api_device)

    def ask_device_status(self):
        self.driver.ask_device_status()

    def command_start(self):
        self.api_device.command_start()

    @property
    def Spd(self):
        x = self.driver.read('Spd')
        print('read from DB: {}'.format(x))
        return x

    # Called from the Epics driver write()
    @Spd.setter
    def Spd(self, val):
        self.driver.setParam('Spd', val)
        self.driver.setParam('Spd-RB', val)
        self.api_device.command_set_Spd(val)

    @property
    def Phs(self):
        x = self.driver.read('Phs')
        print('read from DB: {}'.format(x))
        return x

    @Phs.setter
    def Phs(self, val):
        self.driver.setParam('Phs', val)
        self.driver.setParam('Phs-RB', val)
        self.api_device.command_set_Phs(val)

    # Properties which are read-only in Epics:

    @property
    def ActSpd(self):
        self.driver.getParam('ActSpd')

    @ActSpd.setter
    def ActSpd(self, val):
        self.driver.setParam('ActSpd', val)
        self.driver.updatePVs()

    @property
    def ActSpd(self):
        self.driver.getParam('ActPhs')

    @ActSpd.setter
    def ActSpd(self, val):
        self.driver.setParam('ActPhs', val)
        self.driver.updatePVs()





"""
Communicates with the TCP device.
Parses the responses and encapsulates sending of commands.
"""
class DornierProtocol(twisted.internet.protocol.Protocol):

    def __init__(self, chopper):
        self.chopper = chopper
        self.report_status_parse_success = True

    def connectionMade(self):
        print('GOOD Connection to chopper TCP device established.')
        reactor.callLater(5, self.chopper.ask_device_status)

    def connectionLost(self, x):
        return
        print('WARNING connectionLost: {}'.format(x))
        #reactor.stop()

    """
    Called for every line in the status answer of the chopper.
    """
    def parse_status_line(self, line):
        rx = '(.+?);state (async|synch);amode([ a-zA-Z0-9]+?);nspee([ 0-9]+?);aspee([ 0-9]+?);'
        'nphas([ 0-9.]+?);dphas([ 0-9.]+?);averl([ 0-9.]+?);spver([ 0-9]+?);ratio([ 0-9]+?);'
        'no_action(.*?);monit_(\\d+?);vibra([ \\d.]+?);t_cho([ \\d.]+?);durch([ \\d.]+?);'
        'vakum([ \\d.]+?);valve([ \\d]+?);sumsi([ \\d]+?);'
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
        if len(matched) == 2:
            if self.report_status_parse_success:
                print('GOOD parsed chopper status the first time')
                self.report_status_parse_success = False
            self.chopper.ActSpd = float(matched[0].group(5))
            self.chopper.ActPhs = float(matched[1].group(7))
            reactor.callLater(5, self.chopper.ask_device_status)

    def dataReceived(self, data):
        #self.transport.loseConnection()
        self.parse_status(data)

    def command_start(self):
        self.transport.write('start\r\n')

    """
    Called when the Epics DB is updated with a new set point.
    """
    def command_set_Spd(self, val):
        self.transport.write('nspee 1 {:.0f}\r\n'.format(val))
        # TODO
        # Currently, disc 2 is locked to disc 1, but in the future maybe not?

    """
    Called if new set point in Epics db, but does not currently cause
    a phase change because the simulator does not yet support it.
    """
    def command_set_Phs(self, val):
        x = 'nphas 2 {:.3f}\r\n'.format(val)
        print(x)
        self.transport.write(x)
        # Only for chopper 2 in slave mode
        # TODO  check if it is in slave mode

    """
    Regularly called to retrieve the chopper status.
    """
    def ask_device_status(self):
        self.transport.write('asyst 1\r\n')



"""
Main entry class.
Initializes the TCP connection to the hardware and initializes the chopper
representation instance.
"""
class Facade(object):

    def __init__(self, chopper_name, device_host, device_port, freq_max, pickup_update_interval):
        chopper = Chopper(chopper_name, freq_max, pickup_update_interval)
        protocol = DornierProtocol(chopper)
        api_device = APIDevice(protocol)
        chopper.set_api_device(api_device)
        self.endpoint = TCP4ClientEndpoint(reactor, device_host, device_port)
        self.protocol = protocol
        self.chopper = chopper

    def start(self):
        self.chopper.start()
        self.run()

    def protocol_up(p):
        print('protocol_up')

    def run(self):
        d = connectProtocol(self.endpoint, self.protocol)
        d.addCallback(self.protocol_up)
        reactor.run()
        print('reactor done.')

    def stop(self):
        reactor.stop()
        self.chopper.stop()



"""
Signal handler to allow to stop the program with SIGINT or SIGTERM (systemd).
"""
def handle_SIGINT(signum, frame):
    facade.stop()
def handle_SIGTERM(signum, frame):
    facade.stop()
import signal
signal.signal(signal.SIGTERM, handle_SIGTERM)




if __name__ == '__main__':
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('-n', '--chopper-name', required=True, help='Used as prefix for the epics PVs')
    ap.add_argument('-a', '--device-addr', default='localhost:60000', help='[host]:[port] of the device, default localhost:60000')

    # Have to choose some length for the Epics array which stores the pickup signals.
    # We make this decision based on the update time period of that PV and the maximum
    # chopper frequency.
    ap.add_argument('--freq-max', default=100, help='Maximum possible chopper frequency (Hz), used to size the required Epics array.  Default 100.')
    ap.add_argument('--pickup-update-interval', default=1.33, help='Interval for pickup signal updates in the Epics DB (seconds, float), used to size the required Epics array.  Default 1.33')
    args = ap.parse_args()

    # Extract hostname and port from the argument
    m = re.match('^(.*?)(|:(\\d*))$', args.device_addr)
    device_host = m.group(1)
    device_port = 60000
    if m.group(3):
        device_port = int(m.group(3))

    facade = Facade(args.chopper_name, device_host, device_port, args.freq_max, args.pickup_update_interval)
    facade.start()
    print('main done.')

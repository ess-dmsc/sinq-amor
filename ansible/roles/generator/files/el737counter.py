#!/usr/bin/python -p
#
# fake SINQ EL737 counter box
#
# I am using 1000cts/sec for m1, 500 for m2, 300 for m3 and 2000 for m4
#
# Mark Koennecke, July 2015
#----------------------------------------------------------------------
from twisted.internet import reactor, protocol
from twisted.protocols.basic import LineReceiver
import time
import sys
import subprocess as sp
import os

import generator


class EL737Controller(LineReceiver):
    def __init__(self):
        self.remotestate = 0
        self.delimiter = '\r'
        self.mode = 'timer'
        self.preset = .0
        self.starttime = time.time()
        self.endtime = time.time()
        self.counting = False
        self.mypaused = False
        self.pausestart = 0
        self.pausedTime = 0.
        self.threshold = 0
        self.thresholdcounter = 1
        self.startFailure = False
        self.runFailure = False
        self.atRunFailure = False
        self.readFailure = False
        self.recover = False
        self.proc = MyPIPE()
        self.generator = generator.Generator()

    def write(self, data):
        if self.transport is not None:
            self.transport.write(data)

    def to_process(self, data):
        if self.transport is not None:
            self.proc.transport.write(data)

    def calculateCountStatus(self):
        if self.counting and not self.mypaused:
            runtime = time.time() - self.starttime - self.pausedTime
            print(str(runtime) + ' versus ' + str(self.preset))
            if self.mode == 'timer':
                if runtime >= self.preset:
                    self.counting = False
                    self.endtime = self.starttime + self.preset
                    self.pausedTime = 0
                elif self.runFailure and not self.recover and runtime >= self.preset*0.2:
                    self.atRunFailure = True

            else:
                if runtime*1000 >= self.preset:
                    self.counting = False
                    self.endtime = self.starttime + self.preset/1000
                elif self.runFailure and not self.recover and runtime*1000 >= self.preset*0.2:
                    self.atRunFailure = True

        if self.atRunFailure:
            self.write('?92\r')
            self.counting = False
            self.endtime = time.time()

        print('count flag after calculateCountStatus ' + str(self.counting))

    def calculateFailureStatus(self):
        if not self.recover:
            if self.startFailure:
                self.write('?91\r')
                return True
            elif self.atRunFailure:
                self.write('?92\r')
                return True
            elif self.readFailure:
                self.write('?93\r')
                return True
        else:
            self.startFailure = False
            self.runFailure = False
            self.atRunFailure = False
            self.readFailure = False
            self.recover = False
            return False

    def lineReceived(self, data):
        orig = data.strip()
        data = data.lower().strip()
        g = self.generator

        if self.remotestate == 0:
            if data.startswith('rmt 1'):
                self.remotestate = 1
                self.write("\r")
            else:
                self.write("?loc\r")
            return

        if self.remotestate == 1:
            if data.startswith('echo 2'):
                g.find('./build')
                reactor.spawnProcess(self.proc,g.exe[0],args=orig.split()[2:],env=os.environ)
                self.remotestate = 2
                self.write("\r")
            else:
                self.write("?loc\r")
            return

        if self.remotestate == 2:

           if data.startswith('rmt 1') or data.startswith('echo'):
               self.write('\r')
               return

           if data.startswith('mp'):

               if self.startFailure and not self.recover:
                   self.write('?91\r')
                   return

               l = data.split()
               if len(l) < 2:
                   self.write("argument required\r\n")
                   return
               self.mode = 'monitor'
               self.preset = float(l[1])
               self.starttime = time.time()
               self.mypaused = False
               self.pausedTime = 0.
               self.counting = True
               self.write('\r')
               self.to_process('run\r')
               return

           if data.startswith('tp'):

               if self.startFailure and not self.recover:
                   self.write('?91\r')
                   return

               l = data.split()
               self.mode = 'timer'
               if len(l) < 2:
                   self.write("argument required\r\n")
                   return
               self.preset = float(l[1])
               self.starttime = time.time()
               self.mypaused = False
               self.pausedTime = 0.
               self.counting = True
               self.write('\r')
               self.to_process('run\r')
               return

           if data.startswith('s'):
               self.counting = False
               self.endtime = time.time()
               self.write('\r')
               self.to_process('stop\r')
               #self.remotestate = 1
               return

           if data.startswith('ps'):
               self.counting = True
               self.mypaused = True
               self.pausestart = time.time()
               self.write('\r')
               self.to_process('pause\r')
               return

           if data.startswith('rt'):
               l = data.split()
               if len(l) < 2:
                   self.write("argument required\r\n")
                   return
               self.to_process('rate\r')
               self.to_process(l[1])
               return

           if data.startswith('co'):
               if not self.mypaused:
                   pass
               else:
                   self.mypaused = False
                   self.pausedTime  += time.time() - self.pausestart
               self.to_process('run\r')
               self.write('\r')
               return

           if data.startswith('dl'):
               l = data.split()
               if len(l) >= 3:
                   self.threshold = float(l[2])
                   self.write('\r')
               else:
                   self.write(str(self.threshold) + '\r')

           if data.startswith('dr'):
               l = data.split()
               if len(l) >= 2:
                   self.thresholdcounter = int(l[1])
                   self.write('\r')
               else:
                   self.write(str(self.thresholdcounter) + '\r')

           if data.startswith('rs'):
               if self.calculateFailureStatus():
                   return
               self.calculateCountStatus()
               if self.counting:
                   if self.mypaused:
                       if self.mode == 'timer':
                           self.write('9\r')
                       else:
                           self.write('10\r')
                   else:
                       if self.mode == 'timer':
                           self.write('1\r')
                       else:
                           self.write('2\r')
               else:
                   self.write('0\r')
               return

           if data.startswith('ra'):
               if self.calculateFailureStatus():
                   return
               self.calculateCountStatus()
               if self.counting:
                   if self.mypaused:
                       pausetime = time.time() - self.pausestart
                   else:
                       pausetime = self.pausedTime
                   diff = time.time() - self.starttime - pausetime
               else:
                   diff = self.endtime - self.starttime
               rlist = []
               rlist.append(str(diff))
               rlist.append(str(int(diff*1000)))
               rlist.append(str(int(diff*1500)))
               rlist.append(str(int(diff*500)))
               rlist.append(str(int(diff*300)))
               rlist.append(str(int(diff*2000)))
               rlist.append('0')
               rlist.append('0')
               rlist.append('0')
               rastring = ' '.join(rlist)
               self.write(rastring +'\r')
               return

           if data.startswith('xs'):
               self.startFailure = True
               self.write("Start failure activated\r")
               return

           if data.startswith('xr'):
               self.runFailure = True
               self.write("Run failure activated\r")
               return

           if data.startswith('xd'):
               self.readFailure = True
               self.write("Read failure activated\r")
               return

           if data.startswith('rc'):
               self.recover = True
               self.write("Recover activated\r")
               return

           if data.startswith('id'):
               self.write('EL737 Neutron Counter V8.02\r')
               return

           self.write('?2\r')

    def connectionLost(self, reason):
        self.to_process('exit\r')
        time.sleep(1)
        self.proc.transport.signalProcess('KILL')
        print "Goodbye..."
        print reason


class MyPIPE(protocol.ProcessProtocol):
    def connectionMade(self):
        print "  Connection made"

    def childDataReceived(self, childFD, data):
        print "Message from ",childFD,":"
        print data

    def connectionLost(self, reason):
        print "Goodbye..."

    def outReceived(self,data):
        print "out : ",data

    def errReceived(self,data):
        print "err : ",data

    def processExited(self,reason):
        if reason.value.exitCode != 0 :
            print "exit : ",reason.value
        else :
            print "exit"

    def processEnded(self,reason):
        print "process ended : ",reason.value
        print "quitting"



def main(argv):
    if len(argv) > 1:
        port = int(argv[1])
    else:
        port = 62000

    factory = protocol.ServerFactory()
    factory.protocol = EL737Controller
    reactor.listenTCP(port, factory)
    reactor.run()

if __name__ == "__main__":
    main(sys.argv)

# --------------------------------------------------------------
# Fake EL734 motor. I do not bother with acceleration and 
# deacceleration, just run linearly. All the parameters I 
# ignore are held in a dictionary
#
# Mark Koennecke, June 2015
# -------------------------------------------------------------
import time
import math


class EL734Motor(object):
    """
    PSI EL734 fake motor 
    """

    def __init__(self):
        self.currentstep = 0
        self.startstep = 0
        self.gear = 1000
        self.speed = 100
        self.targetstep = 0
        self.starttime = time.time()
        self.moving = False
        self.stop = False
        self.sign = 1
        self.lowlim = -180.
        self.hitlow = False
        self.highlim = 360.
        self.hithigh = False
        self.refrun = False
        self.reftarget = self.highlim * self.gear
        self.target = .0
        self.stopping = False
        self.acfail = False
        self.runfail = False
        self.posfail = False
        self.startfail = False
        self.hwfail = False
        self.mispos = False
        self.readfail = False
        self.limitfail = False
        self.recover = False
        self.dolog = False
        self.par = {"a": "3", "ec": "1 2", "ep": "1", "fd": "500 1", \
                    "d": "0.1", "e": "20", "f": "1", "g": "300", \
                    "k": "1", "l": "0", "m": "3", "q": "0.0", \
                    "t": "0", "w": "0", "z": "0", 'mn': 'xyz'}

    def setpar(self, key, val):
        if self.refrun:
            self.iterate()
            return '*BSY'
        if key in self.par.keys():
            self.par[key] = val
            return ""
        else:
            if key == "j":
                self.speed = int(val)
                return ""
            elif key == "fm":
                l = val.split()
                self.gear = int(l[0])
                return ""
            elif key == "u":
                pos = float(val)
                self.currentstep = pos * self.gear
                return ""
            elif key == "p":
                target = float(val)
                self.startdrive(target)
                return ""
            elif key == "v":
                self.reftarget = int(val)
                return ""
            elif key == "r":
                self.dorefrun()
                return ""
            elif key == 's':
                self.stop = True
                self.iterate()
                return ""
            else:
                return "?CMD"

    def getpar(self, key):
        if self.refrun:
            self.iterate()
            return "*BSY"
        if key in self.par.keys():
            return self.par[key]
        else:
            if key == "j":
                return str(self.speed)
            elif key == "fm":
                return "%d 1" % self.gear
            elif key == "u":
                return self.readpos()
            elif key == "msr":
                return self.calcmsr()
            elif key == "ss":
                return self.calcss()
            elif key == "v":
                return "%d" % self.reftarget
            elif key == 's':
                self.stop = True
                self.iterate()
                return ""
            elif key == 'xa':
                self.acfail = True
                self.log('Air cushion failure')
                return self.getpar('mn') + " - air cushion failure activated"
            elif key == 'xp':
                self.posfail = True
                self.log('Position failure')
                return self.getpar('mn') + " - position failure activated"
            elif key == 'xr':
                self.runfail = True
                self.log('Run failure')
                return self.getpar('mn') + " - run failure activated"
            elif key == 'xs':
                self.startfail = True
                self.log('Start failure')
                return self.getpar('mn') + " - start failure activated"
            elif key == 'xh':
                self.hwfail = True
                self.log('Hardware failure')
                return self.getpar('mn') + " - hardware failure activated"
            elif key == 'xm':
                self.mispos = True
                self.log('Misposition error')
                return self.getpar('mn') + " - misposition error activated"
            elif key == 'xd':
                self.readfail = True
                self.log('Read failure')
                return self.getpar('mn') + " - read failure activated"
            elif key == 'xl':
                self.limitfail = True
                self.log('Limit failure')
                return self.getpar('mn') + " - limit failure activated"
            elif key == 'rc':
                self.recover = True
                self.log('Recover activated')
                return self.getpar('mn') + " - recover activated"
            elif key == 'lg':
                self.dolog = True
                return self.getpar('mn') + " - log in /opt/amor/log.txt"
            else:
                return "?CMD"

    def setlimits(self, l, h):
        if self.refrun:
            return "*BSY"
        self.highlim = h
        self.lowlim = l
        return ""

    def getlimits(self):
        if self.refrun:
            return "*BSY"
        return (self.lowlim, self.highlim)

    def iterate(self):
        if self.moving:
            tdiff = time.time() - self.starttime
            stepsDone = tdiff * self.speed
            # print('tdiff, stepsDone, startstep, targetstep, starttime: ' + str(tdiff) + ', ' + str(stepsDone) + ', ' +
            # str(self.startstep) + ', ' + str(self.targetstep) + ', ' + str(self.starttime) )
            if self.sign == 1:
                # moving positive
                curpos = self.startstep + stepsDone
                if curpos >= self.targetstep:
                    self.moving = False
                    self.refrun = False
                    if self.target > self.highlim:
                        self.currentstep = self.highlim * self.gear - 10
                        self.hithigh = True
                        self.log("Hithigh tar %d hl %d" % (self.target, self.highlim))
                    else:
                        self.currentstep = self.target * self.gear

                    self.stopping = True
                else:
                    self.currentstep = self.startstep + stepsDone
                    if self.runfail or self.acfail:
                        self.moving = False
                        self.stopping = True
            else:
                # moving negative
                curpos = self.startstep - stepsDone
                if curpos <= self.targetstep:
                    self.moving = False
                    self.refrun = False
                    if self.target < self.lowlim:
                        self.currentstep = self.lowlim * self.gear + 10
                        self.hitlow = True
                        self.log("Hitlow tar %d ll %d" % (self.target, self.lowlim))
                    else:
                        self.currentstep = self.target * self.gear
                    self.stopping = True
                else:
                    self.currentstep = self.startstep - stepsDone
                    if self.runfail or self.acfail:
                        self.moving = False
                        self.stopping = True

            if self.stop:
                self.moving = False
                self.refrun = False

    def readpos(self):
        self.iterate()
        pos = self.currentstep / self.gear
        self.log("Read the position - " + str(pos))
        if self.readfail and not self.recover:
            pos = pos * 0.6
            self.log("Providing alternative position - " + str(pos))
        return "%6.3f" % (pos)

    def startdrive(self, target):
        self.log("Driving to target - " + str(target))

        if self.startfail and not self.recover:
            self.moving = False
            self.stop = True
            return

        if self.posfail or self.hwfail or self.mispos:
            if not self.recover:
                target = target * 0.9
                self.log("Moving to alternative target - " + str(target))

        self.startstep = self.currentstep
        self.starttime = time.time()
        self.targetstep = target * self.gear
        pos = self.currentstep / self.gear
        if target < pos:
            self.sign = -1
        else:
            self.sign = 1
        self.hithigh = False
        self.hitlow = False
        self.moving = True
        self.stop = False
        self.refrun = False
        self.target = target
        self.stopping = False

        if self.recover:
            self.log("Resetting errors and recover")
            self.acfail = False
            self.posfail = False
            self.runfail = False
            self.startfail = False
            self.hwfail = False
            self.mispos = False
            self.readfail = False
            self.limitfail = False
            self.recover = False

    def calcmsr(self):
        """
        Calculates the moving status of the motor. Following bits are set
        for various type of moving status
        Return   Status
        value
        1       : Currently moving
        2       : Target reached and stopping
        4       : -
        8       : Stop
        16      : Hit low
        32      : Hit high
        64      : -
        128     : Run failure
        256     : Start failure
        512     : Position failure
        1024    : Hardware failure
        2048    : Misposition error
        4096    : Air cushion failure
        """
        self.log("Calculating MSR")
        self.iterate()

        # Moving?
        if self.moving:
            msr = '1'
        else:
            msr = '0'

        # Stopping ?
        if self.stopping and self.currentstep == self.targetstep:
            msr += '1'
            self.stopping = False
        else:
            msr += '0'

        msr += '0'

        # Stop
        if self.stop:
            msr += '1'
            self.stop = False
        else:
            msr += '0'

        # Hit high/low
        if self.hitlow:
            msr += '1'
            self.hitlow = False
        else:
            msr += '0'
        if self.hithigh:
            msr += '1'
            self.hithigh = False
        else:
            msr += '0'

        msr += '0'

        if self.runfail:
            msr += '1'
            if self.recover:
                self.runfail = False
        else:
            msr += '0'

        if self.startfail:
            msr += '1'
            if self.recover:
                self.startfail = False
        else:
            msr += '0'

        if self.posfail:
            msr += '1'
            if self.recover:
                self.posfail = False
        else:
            msr += '0'

        if self.hwfail:
            msr += '1'
            if self.recover:
                self.hwfail = False
        else:
            msr += '0'

        if self.mispos:
            msr += '1'
            if self.recover:
                self.mispos = False
        else:
            msr += '0'

        if self.acfail:
            msr += '1'
            if self.recover:
                self.acfail = False
        else:
            msr += '0'

        msr += '0'

        self.log("MSR - " + hex(int(msr[::-1], 2)))
        return hex(int(msr[::-1], 2))

    def calcss(self):
        self.log("Calculating SS")
        self.iterate()
        ss = '00'
        if self.stop:
            ss += '1'
        else:
            ss += '0'
        if self.hitlow:
            ss += '1'
        else:
            ss += '0'
        if self.hithigh:
            ss += '1'
        else:
            ss += '0'
        ss += '0'
        self.log("SS - " + hex(int(ss[::-1], 2)))
        return hex(int(ss[::-1], 2))

    def dorefrun(self):
        self.startdrive(self.reftarget / self.gear)
        self.refrun = True

    def log(self, message):
        if self.dolog:
            try:
                with open("/opt/amor/log.txt", "a") as logfile:
                    import datetime
                    logfile.write(
                        datetime.datetime.now().strftime("%Y-%B-%d %I:%M%p") + " " + self.getpar(
                            "mn") + " : " + message + "\n")

            except IOError:
                print('Cannot open for log: /opt/amor/log.txt')

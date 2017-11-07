SINQ AMOR Simulation
=================

This aims to be a complete simulation of the reflectometer AMOR at SINQ, PSI. It should serve as
a base for how such a simulation can look like and as a base for developing BrightnESS and ECP software
without access to the real thing.


## Content

* Twisted based simulation for three motor control units at AMOR:
  mota, motb, motc. They implement the EL734 motor controller
  protocol. EL734 is a PSI developed motor controller developed in the
  1990-ies and is still the main stay motor controller in SINQ and at
  AMOR
  * An EPICS-IOC to talk to all these motors
  * A twisted based neutron event simulation. It implements the EL737 counterbox protocol.

## Prerequisites

* A machine or VM with the ESS ICS software installed
* Python
* Ansible
* Extra repositories, cloned in the same directory as this one
  * forward-epics-to-kafka
  * essiip-fakesinqhw

## Setting up the simulation
In the `ansible` directory, edit the `staging` file, changing `support` to the
name of the target machine where the simulation is to be installed, if
necessary (you may also use the IP address of the target machine). Then run

    $ ansible-playbook -i staging amor.yml

## Running the simulation
To change the state of the simulation, run one of the following:

    $ ansible-playbook -i staging start_simulation.yml
    $ ansible-playbook -i staging restart_simulation.yml
    $ ansible-playbook -i staging stop_simulation.yml

## Self-contained simulation on dev-env
Similarily, we can set up a self-contained simulation including Kafka broker on the
development environment.  An example inventory is provided:
```
cd sinq-amorsim/ansible/
ansible-playbook -i inv-selfcontained.txt amor.yml
ansible-playbook -i inv-selfcontained.txt restart_simulation.yml
```
After starting this single-machine setup, one should see for example chopper TDCE
signals on the Kafka topic `chopper1.TDCE` from the example `chopper1` which
is enabled by default.

## Accessing the IOCs
The IOCs can be accessed using `telnet`. For this, run

    $ telnet <host> <port>

The port for each IOC is defined in the corresponding `.service` file. The
default values are

- amor_ioc:  localhost `20001`

## Getting rid of it

The ansible playbook installs all files into /opt/amor. Thus getting
rid of it means removing that directory. Then you have to disable and
remove the installed services in  /usr/lib/systemd/system. As of now:
mota, motb, motc and amor_ioc, generator.


## Documentation

Theres is some documentation on the instrument in the doc
directory.

*amorPub.pdf* is a recent publication on the AMOR instrument.

*hand_out.ps* is a short user manual for the instrument. It also
 contains a list of available motors and of their meaning in AMOR.
 The EPICS name of the motor can be derived from the list in the
 following way: There are three motor controller named A, B and
 C. This maps to motor controllers SQ:AMOR:mota, SQ:AMOR:motb and
 SQ:AMOR:motc. You get the full name of the motor record by appending
 the motor name to this. Example: coz is SQ:AMOR:mota:coz. 

*distance.tex* is a description of how the AMOR geometry is calculated
 from virtual motor positions. 
 
## Documentation for Additional Hardware

There is a scaler record called SQ:AMOR:cter1 for counting. This
record controls counting. In the SINQ and simulation implementation it
also starts streaming neutron events. The standard EPICS scaler record does not support all status states of a
neutron counter. I therefore put additional status information into
the monitor S10. If S10 is 2, this means no beam, if it is 3, this
means paused.


AMOR also has three magnets:

* *PBY*  the polarizer magnet
* *ABY*  the analyzer magnet
* *FMA* the sample magnet

Polarizer and analyzer magnets can be driven in the range between -40
to 40 A; the sample magnet FMA between -100 to 100 A. In EPICS, these
magnets are represented by prefixes SQ:AMOR:PBY, SQ:AMOR:FMA and
SQ:AMOR:ABY. Each magnet has sub records:

* *HighLim*  The upper limit for setting the current
* *LowLim*  The lower limit for setting the current
* *ErrCode*  The error code of the magnet
* *ErrText*  A human readable representation of the magnet error
* *PowerStatusRBV* For reading back if the magnet is on or off
* *PowerStatus* A switch for switching the magnet on or off
* *CurSet* For setting the magnet current
* *CurRBV* For reading back the current value

The AMOR magnets are simulated by SLSVME.py. The EPICS support happens
via a streamdevice driver living in slsvme.proto and an EPICS database
slsvme.db.


AMOR also has a Siemens programmable logic unit for controlling the shutter and a switch for 
the alignment laser and the spin flipper RF. This is SPS-S5 which is connected to the world as 
such via a custom RS232 interface and a terminal server. In EPICS the following PV's are provided:

* *SQ:AMOR:SPS1:DigitalInput* a waveform record with 16 bytes giving the state of the SPS digital inputs
* *SQ:AMOR:SPS1:AnalogInput* a waveform record holding 8 analog input values. This is not used at AMOR. Anyway, 
  how a physical value is calculated from any of these values depends on the connected sensor and the 
  application. 
* *SQ:AMOR:SPS1:Push* This is a string output for sending the commands to toggle SPS buttons to have something happen 
  in the SPS. For AMOR, the following commands are valid:
   + S0000, toggles the shutter, state is in byte 5  of the DigitalInput
   + S0001, toggles the laser light, state is in  byte 16, bit 7 of DigitalInput
   + S0007, toggles the spin flipper RF, state is in byte 13, bit 7 of DigitalInput

The AMOR SPS is simulated via a twisted server implement in spss5.py. EPICS support is realised as streamdevice 
driver implemented in spss5.proto and spsamor.db. 


AMOR also has a laser distance measurement device called dimetix. In
order to set the various beamline components properly when changing
omega or two theta or else, the position of these beamline components
on the optical bench needs to be known very precisely. In order to
find the positions automatically, each component is equipped with a
little mirror. The height at which this mirror is attached is unique
to the component. Then there is the dimetix laser distance measurement
device which can be moved up and down with the XLZ motor. With this
setup, the position of all the beamline components can be determined
by driving XLZ to predefined positions and taking distance
measurements with the dimetix.  The dimetix device is simulated with
dimetix.py. There is a streamdevice driver for the dimetix for EPICS,
expressed in dimetix.proto and dimetix.db.
In EPICS, the following PV are available for the dimetix:

* *SQ:AMOR:DIMETIX:DIST*  The distance readback. Please note that this
 is 0, and the value in an alarm state when the laser is off.
* *SQ:AMOR:DIMETIX:LASER* is a binary output which switches the laser
  on and off. The device does not support reading back of the laser
  state.
* *SQ:AMOR:DIMETIX:SimVal*  is a output value which allows to set the
  readback value for the simulation. This is only useful in the
  simulation, the real device will not honour this value.

### Run Ansible

The AMOR simulation can be installed using the ``amor.yml`` playbook.
To run in the dm-dev-env virtual machine use:

```shell
ansible-playbook -i hosts --private-key <machine private key> amor.yml [-c paramiko] --extra-vars "repo_loc_essiip_fakesinqhw=$<local essiip-fakesinqhw folder> [generator_branch=<branch>]"
```

Note:
* the ``-c paramiko`` option can be required to solve some connection issues
* using ``generator_branch=<branch>`` one can specify a specific branch for the ``AMORgenerator``
* see the different options for ``essiip_fakesinqhw``

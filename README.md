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

## Prerequisites

* A machine or VM with the ESS ICS software installed
* Python
* Ansible

## Setting up the simulation
In the `ansible` directory, edit the `staging` file, changing `support` to the
name of the target machine where the simulation is to be installed, if
necessary (you may also use the IP address of the target machine). Then run

    $ ansible-playbook -i staging site.yml

## Running the simulation
To change the state of the simulation, run one of the following:

    $ ansible-playbook -i staging start_simulation.yml
    $ ansible-playbook -i staging restart_simulation.yml
    $ ansible-playbook -i staging stop_simulation.yml

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
mota, motb, motc and amor_ioc.


## Documentation

Theres is some documentation on the instrument in the doc
directory.

*amorPub.pdf* is a recent publication on the AMOR instrument.

*hand_out.ps* is a short user manual for the instrument. It also
 contains a list of available motors and of their meaning in AMOR.
 The EPICS name of the motor can be derived from the list in the
 following way: There are three motor controller named A, B and
 C. This maps to motor controllers SQ:AMOR:mota, SQ:AMOR:motb and
 SQ:AMOR:motc. You get the full name of the motor recod by appending
 the motor name to this. Exampe coz is SQ:AMOR:mota:coz. In addition
 there is a scaler record called SQ:AMOR:cter1 for counting. The
 standard EPICS scaler record does not support all status states of a
 neutron counter. I therefore put additional status information into
 the monitor S10. If S10 is 2, this means no beam, if it is 3, this
 means paused.

*distance.tex* is a description of how the AMOR geometry is calculated
 from virtual motor positions. 
 

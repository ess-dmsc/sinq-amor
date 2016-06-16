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

- amor_ioc: `20001`

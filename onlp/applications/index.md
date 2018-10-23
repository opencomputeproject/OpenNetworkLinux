# ONLP APIs for Applications

## Overview

The ONLP API provides system applications with a consistent functional abstraction for accessing platform objects.
The ONLP abstraction layer allows a dataplane, system, or NMS application to be written for any supported Open Networking platform.

A dataplane application typically needs to ask the following platform-dependent questions:

* Is an SFP present or absent?
* If an SFP is present how can I determine its type, capabilities, and make operational decisions? Is it a 100G LR4 module or 1G Copper SFP connected through an MSA adapter?
* How can I communicate with the specific hardware once I have identified it? If its a 1G Copper SFP containing a Marvell PHY how do I enable or disable autonegotiation?
* How can I access any Digital Optical Monitoring information that may be present in the module that reportst he L1 quality of a port or ports?

An NMS agent typically needs to ask the following platform-dependent questions:

* What are the available temperature sensors and what are their values?
* Has a PSU been inserted or removed? If present is it unplugged, operational, or in a failure state?
* What identification information is available for any given FRU?
* Are all the fans present in the system working and do they all have the same airflow direction?


Accessing any of this information depends intimately on the design of the hardware platform. A number of custom FPGAs, CPLDs, and I2C busses are typically used and there is no agreed upon standard for arranging functionality or components in any particular way.
If you don't know the board design and you don't have the documentation and you don't know which CPLD register bit or GPIO indicates an SFP has been inserted into a particular port then you can't answer any of the above questions.

Dataplane and System Applications want to deal with all of these objects at a functional level, not a board layout and design level.
* You want to know if the SFP in a given port is a 40G-AOC but you don't care in even the smallest way how many I2C muxes have to be programmed before you can read it.
* You want to make sure all fans are running front-to-back but you don't care whether that information is read dynamically by specialized hardware, read from a FRU eeprom, or determined statically based on knowledge of the model number of the PSU its in.
* You want to know if any FRU has failed, regardless of the way that is determined based on the type of fan controller used.

The goal of the ONLP Application API is to hide this mess behind a standard abstraction layer based on the information that is of functional and operational interest, independent from the implementation on any given platform.
Applications using the ONLP APIs can run on all supported platforms.

### Whats wrong with IPMI?

Nothing really. But most open networking platforms do not have a BMC (and none at all when we started the ONLP project) but still require all of this functionality. Systems which do have a BMC can simply implement their ONLP interfaces using IPMI.
IPMI deals with the Fans, PSUs, and Thermals, and running the thermal plan for the system to make sure it doesn't light on fire. But it has no models for SFF pluggables and their control structures.
Likewise there are other proposals for communicating to system software through interfaces like APCI but these are not standardized either.

ONLP is the standard for all platforms supported by Open Network Linux and Open Network Linux support is required for all platforms accepted by the Open Compute Project.

[Next: ONLP API Concepts](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/concepts)

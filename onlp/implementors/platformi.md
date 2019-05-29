# platformi

The purpose of this module is to implement some or all of the following:

* Platform Identification and Assignment as per the [Getting Started](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/getting_started) section.
* Provide Management Hooks which manage the platform fans and status LEDs based on the platform thermal plan and current state of any FRUs
  * Fans are adjusted according to airflow type and values of various thermal sensors
  * LEDs states may be changed over time to indicate the operational state (or lackthereof) of certain FRUs, including but not limited to:
    * PSU failed, unplugged, or missing
    * Fan failed or missing
* Provide Debug Hooks into the platform internals (optional)
  * Many platforms have CPLDs and FPGAs with useful register state that can be exposed during development or tech support.


## Fan Management

The ```onlp_platformi_manage_fans()``` function must be implemented to read the current state of the thermals and set all fan speeds appropriately.
On most platforms this should also set all fans to max if any fan is in a failure state, regardless of the values of the thermals.

Implementation of this algorithm is highly specific to your platform. This function will be called periodically by the platform management daemon.
While this function should be comprehensive in its inspection and reaction it should not block for a unreasonable amount of time.

## LED Managmeent

The ```onlp_platformi_manage_leds()``` function must be implemented to set the state of all LEDs in the system consistent with the current state of the FRUs,
as per the platform policy.

Many platforms have a "health" LED which may be green when all fans and PSUs are operational, or Yellow if a single PSU has failed, etc.

The LED policy is left to the platform implementor. This function will be called periodically by the platform management daemon.
It should not block for an unreasonable amount of time.


## Baseboard Management Controllers

If your platform contains a BMC then the Fan and LED algorithms will be implemented there and not by ONLP.
In this case there is no need to implement the fan and led management routines.

## Platformi Debug Hook

The ```onlp_platformi_debug()``` function an ```argc``` and and ```argv``` and should return its output through the passed AIM PVS pointer.

While an application can call this it is usually issue through the command line tools.

For example, from the onlpd tool you might run:

```#> onlpd debug cpld-dump 0 1 2```

In this case the parameters "cpld-dump", "0, "1", "2" will be passed to your ```onlp_platformi_debug()``` routine, the output collected,
and shown to the user.

This interface is optional.

## Platformi Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__platformi.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/platformi.h)
* [Example Implementation](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/platforms/accton/x86-64/as7712-32x/onlp/builds/x86_64_accton_as7712_32x/module/src/platformi.c)


---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)
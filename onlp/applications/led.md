# LEDs

LEDs are typically children of the Chassis, PSU, Fan, or SFP OIDs. 

## LED Information Structure

The current status of an LED is queried using ```onlp_led_info_get``` to populate the ```onlp_led_info_t``` structure.

LED features include things like available colors, blinking modes, and character display features.

## LED Specific APIs

You can change the current mode of the LED if supported.

## LED Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__oid-led.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/led.h)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/apis)

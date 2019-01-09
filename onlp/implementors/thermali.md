# thermali

The purpose of this module is to implement platform properties of the Thermal OID.

There are only two functions which you must implement:

## ```onlp_thermali_hdr_get()```

This returns the oid header for the given thermal oid.

* These are not typically FRUs but the PRESENT status must still be set. Otherwise no other operations will be performed.
* If the sensor is failing then set the FAILED status. No operations will be performed on a failed sensors.

## ```onlp_thermali_info_get()```

This returns the full information structure for the thermal sensor. This includes the capability flags and current state of the variables corresponding to those capabilities (rpm, percentage, etc).
In most cases this is just the current temperature (in millicelcius) but may include thermal warning and shutdown thresholds.

## Thermali Documentation
* [Doxygen]()
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/thermali.h)
* [Example Implementation](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/platforms/accton/x86-64/as7712-32x/onlp/builds/x86_64_accton_as7712_32x/module/src/thermali.c)


---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

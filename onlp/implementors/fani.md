# fani

The purpose of this module is to implement platform properties of the Fan OID.

There are only two functions which you must implement:

## ```onlp_fani_hdr_get()```

This returns the oid header for the given fan oid.

* If the fan is present then you must set the PRESENT flag. Otherwise it is assumed to be absent and no further operations will be performed.
* If the fan is in a failure state then you must set the FAILED flag. If a fan has failed then no other operations will be performed.

In the event that a fan or fan tray has a software visible child oid (like an LED) then those children must be populated in the coid table.

## ```onlp_fani_info_get()```

This returns the full information structure for the fan. This includes the capability flags and current state of the variables corresponding to those capabilities (rpm, percentage, etc).

## Fani Set Functions

If the SET capability flags are set by ```onlp_fani_info_get()``` then the ```onlp_fani_*_set``` functions should be available and functional.
They will not be called unless the capabilities are reported.

## Fani Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__fani.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/fani.h)
* [Example Implementation](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/platforms/accton/x86-64/as7712-32x/onlp/builds/x86_64_accton_as7712_32x/module/src/fani.c)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

# ledi

The purpose of this module is to implement platform properties of the Led OID.

There are only two functions which you must implement:

## ```onlp_ledi_hdr_get()```

This returns the oid header for the given led oid.

* If the led is present then you must set the PRESENT flag. Otherwise it is assumed to be absent and no further operations will be performed.

## ```onlp_ledi_info_get()```

This returns the full information structure for the led. This includes the capability flags and current mode of the LED if applicable.

## Ledi Set Functions

If the SET capability flags are set by ```onlp_ledi_info_get()``` then the ```onlp_ledi_*_set``` functions should be available and functional.
They will not be called unless the capabilities are reported.

## Ledi Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__ledi.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/ledi.h)
* [Example Implementation](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/platforms/accton/x86-64/as7712-32x/onlp/builds/x86_64_accton_as7712_32x/module/src/ledi.c)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

# chassisi

The purpose of this module is to implement platform properties of the Chassis OID.

There are only two functions which you must implement:

### ```onlp_chassis_hdr_get()```

This returns the chassis root oid hdr for the system.

You must mark the chassis oid status as both PRESENT and OPERATIONAL.

You must return all top-level chassis OID children in the coids table.

### ```onlp_chassisi_info_get()```

There are no fields currently defined in the ```onlp_chassis_info_t``` structure so it is currently equivalent to populating the OID header.
The current default implementation of ```onlp_chassisi_info_get()``` does this for you.

# Chassisi Documentation
* [Doxygen](https://htmlpreview.github.io/?https://raw.githubusercontent.com/opencomputeproject/OpenNetworkLinux/ONLPv2/packages/base/any/onlp/src/onlp/doc/html/group__chassisi.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/chassisi.h)
* [Example Implementation](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/platforms/accton/x86-64/as7712-32x/onlp/builds/x86_64_accton_as7712_32x/module/src/chassisi.c)


---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

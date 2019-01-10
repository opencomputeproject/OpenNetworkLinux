# modulei

The purpose of this module is to implement platform properties of the Module OID.

There is only one function you must implement:

## ```onlp_module_hdr_get()```

This returns the oid header for the given module oid.

* You must mark the OID as PRESENT.
* If the module has failed or is inoperable you may set the FAILED state.

## ```onlp_modulei_info_get()```

There are no fields currently defined in the ```onlp_module_info_t``` structure so it is currently equivalent to populating the OID header.
The current default implementation of ```onlp_module_info_get()``` does this for you and there is no need to provide this function at this time.

## Notes

The Module OID Type is defined to be a proper, dynamic container for other OIDs. It differs from the Generic OID in that it represents a physical module with a OID heirarchy.
Think linecard or module port chassis.

Modules heirarchies are visible to the user.

# Modulei Documentation
* [Doxygen](http://ocp.opennetlinux.org/onlp/group__modulei.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/moduleii.h)


---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

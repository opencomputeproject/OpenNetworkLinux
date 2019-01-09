# generici

The purpose of this module is to implement platform properties of the Generic OID.

There is only one function you must implement:

## ```onlp_generic_hdr_get()```

This returns the oid header for the given generic oid.

* You must mark the OID as PRESENT.

## ```onlp_generici_info_get()```

There are no fields currently defined in the ```onlp_generic_info_t``` structure so it is currently equivalent to populating the OID header.
The current default implementation of ```onlp_generic_info_get()``` does this for you and there is no need to provide this function at this time.

## Notes

While the Generic OID can be a parent container for other OIDs it is generally designed to be used as a handle in conjunction with the Attribute API to expose custom features.
There are no defined behaviors for the Generic OID.

# Generici Documentation
* [Doxygen](https://htmlpreview.github.io/?https://raw.githubusercontent.com/opencomputeproject/OpenNetworkLinux/ONLPv2/packages/base/any/onlp/src/onlp/doc/html/group__generici.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/genericii.h)


---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

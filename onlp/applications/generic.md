# Generics

## Generic OIDs

Generic OIDs are defined by the platform implementor. Their properties and usage are up the the platform implementor. They are custom cookie OIDs used to access platform-specific functionality which is not defined in the ONLP API itself.

## Generic Information Structure

The current status of a generic oid is queried using ```onlp_generic_info_get``` to populate the ```onlp_generic_info_t``` structure.

There are no fields defined in the information structure. Custom interaction with a Generic OID must be done through the OID Attribute APIs.

## Generic Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__oid-generic.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/generic.h)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/apis)

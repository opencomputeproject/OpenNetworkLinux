# Modules

## Module OIDs

Module OIDs represent containers for other OIDs, for example a hot-pluggable chassis module or line-card.

Module OIDs are similar to the Chassis OID as a container for dynamic children but are not restricted to a single instance, have dynamic status, and are defined by the platform.


## Module Information Structure

The current status of a module is queried using ```onlp_module_info_get``` to populate the ```onlp_module_info_t``` structure.

There are no module-specific fields in this structure as it is just a dynamic container for child OIDs. Those children can then be inspected and enumerated recursively.

## Module Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__oid-module.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/module.h)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/apis)

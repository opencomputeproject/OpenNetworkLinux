# Chassis

## Chassis OID

The Chassis OID represents the root OID for the entire system. There is only one Chassis OID and it is the only one guaranteed to exist at all times.
The Chassis OID is represented by the special value ``ONLP_OID_CHASSIS``.

## Chassis Information Structure

Enumerating the current system topology starts with a call to ```onlp_chassis_info_get``` using the special value ONLP_OID_CHASSIS.
This populates the ```onlp_chassis_info_t``` structure.

There are no chassis-specific fields in this structure but it contains all of the top-level OIDs as its children. Those children can then be inspected and enumerated recursively.

## Chassis Specific APIs

### Chassis Environment

The Chassis API provides a standard method to generate a JSON or YAML representation of the current environmental state of the system. This includes the health and status for all Fans, Thermals, and PSUs in the system. This provides the equivalent of ```show environment```.

### Chassis Debug

It is possible to query the Chassis for any generic debug, internal state or status for the purposes of technical support. The contents of this data are not interpreted or defined by the API itself but up to the system implementation.

## Chassis Documentation

* [Doxygen](http://ocp.opennetlinux.org/onlp/group__oid-chassis.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/chassis.h)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/apis)

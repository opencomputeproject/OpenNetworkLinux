# Attributes

The OID Attribute API provides a generic mechanism to get or set key/value pairs on any object.

## The Attribute API

You can perform the following operations using this API:
* Ask an OID if it supports the given named attribute.
* Ask an OID to return the value of a named attribute.
* Ask an OID to set the value of a named attribute.
* Ask an OID to free a previously returned attribute.

## Standard Attributes

The attribute keys and values are system and implementation specific.
The API does however define two standard attributes that should be supported if possible:

### The ONIE Attribute
The ONIE attribute should return the ```onlp_onie_info_t``` structure.
At a minimum this is supported by the Chassis OID to return the system identification information.
Any OID that has an ONIE identifier (for example a pluggable module) should also support this attribute.

### The Asset Attribute
The ONIE information structure is defined by the available TLV values defined in the ONIE [specification](https://opencomputeproject.github.io/onie/design-spec/hw_requirements.html)

The fields of the ```onlp_asset_info_t``` structure are defined by the ONLP API and evolve over time.
This attribute should be implemented at a minimum by the Chassis OID to return important system information (like firmware revisions).

## Attribute Documentation
* [Doxygen](https://htmlpreview.github.io/?https://raw.githubusercontent.com/opencomputeproject/OpenNetworkLinux/ONLPv2/packages/base/any/onlp/src/onlp/doc/html/group__attributes.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/attribute.h)

# attributei

The purpose of this module is to provide any standard and/or custom attributes for any OID.

## Supporting Attributes

In order to operate on an attribute your ```onlp_attributei_supported()``` function must return True for the named attribute on the given OID.
If it does not then ```onlp_attributei_get()``` and ```onlp_attributei_set()``` will not be called with that attribute name.

## Standard Attributes

While custom attributes are completely optional, it is required that at a very minimum the Chassis OID supports retreiving both the ONIE attribute and the Asset attribute using ```onlp_attributei_onie_info_get()``` and ```onlp_attributei_asset_info_get()```.

Supporting these on anything other than the Chassis OID is optional.

Should a FRU or Module support either of these attributes then you are encouraged to support them on those OIDs as well.

## Attributei Documentation
* [Doxygen](http://ocp.opennetlinux.org/onlp/group__attributei.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/attributei.h)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

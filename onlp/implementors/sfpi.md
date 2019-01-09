# sfpi

The purpose of this module is to implement platform properties of the SFP OID.

## ```onlp_sfpi_hdr_get()```

You must implement this function. If an SFP is present then you must set the OID PRESENT status flag or no other operations will be performed.
It will be assumed that the SFP module is absent.

## SFP Operations

The sfpi interface is different from the other interfaces in that it synthesizes the contents of the ```onlp_sfp_info_t``` structure from the information provided by the SFPI interface.

The MSA SFF decode for both identification and DOM information is parsed in the higher layers. Your SFPI interface need only provide the following:

* SFP Presence Detection
* Reading SFP Signals
  * RX_LOS
* Writing SFP Signals
  * TX_DISABLE, LP_MODE, RESET, etc
* Generic Read/Write Access to the SFP via I2C

## SFP Implementations

Supporting all relevant SFP controls, status bitmaps, and read/write operations is required to provide rich access to all required SFP features.

## SFPi Documentation
* [Doxygen](https://htmlpreview.github.io/?https://raw.githubusercontent.com/opencomputeproject/OpenNetworkLinux/ONLPv2/packages/base/any/onlp/src/onlp/doc/html/group__sfpi.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/sfpi.h)
* [Example Implementation](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/platforms/accton/x86-64/as7712-32x/onlp/builds/x86_64_accton_as7712_32x/module/src/sfpi.c)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

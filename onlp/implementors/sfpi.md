# sfpi

The purpose of this module is to implement platform properties of the SFP OID.

## SFP OID Header

```onlp_sfpi_hdr_get()```

The ```onlpi_sfpi_hdr_get()``` function must be implemented. If an SFP is present then you must set the OID PRESENT status flag or no other operations will be performed. It will be assumed that the SFP module is absent.

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

### Bitmaps
Supporting the SFP Bitmap APIs allows the application to aggregate important status. 

### Read/Write Operations
All sfpi_dev_{read,readb,readw,write,writeb,writew} functions should be implemented for any device/address. You should consider an SFP like a *bus, not a single device*. Module idproms @ 0x50, DOM information for SFP+ is @ 0x51, Embedded Copper PHYs @ 0x56, etc. Access to a static set of device addresses is insufficient so your ONLP implementation must be flexible enough to communicate with any given device address. 

### SFP Controls
A proper implementation of the Control API is necessary. Do not skip support for all available control fields, either get or set where applicable. 

## SFPi Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__sfpi.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi/sfpi.h)
* [Example Implementation](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/platforms/accton/x86-64/as7712-32x/onlp/builds/x86_64_accton_as7712_32x/module/src/sfpi.c)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

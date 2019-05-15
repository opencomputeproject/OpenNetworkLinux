# SFP Management

## SFP OIDs

Each SFP OID represents a small form factor pluggable module. When an SFP OID is present then a module is plugged in. When an SFP OID is absent then the port is empty.

## SFP Information Structure
Information about the current module is queried using ```onlp_sfp_info_get``` to populate the ```onlp_sfp_info_t``` structure.

There are several useful fields in this structure:
* The physical form factor of the port - SFP+,QSFP+,SFP28,QSFP28,QSFP-DD
* The status of the external signal (depending upon module type) - TX_DISABLE, LP_MODE, RX_LOS, and others.
* The identification of the given module based on the SFF MSA standards
* The Digital Optical Monitoring data based on the SFF MSA standards.

## SFP APIs

While calling ```onlp_sfp_info_get``` is sufficient to identify the inserted module there are several operational APIs provided to manage it.

* Change the control signal status
  * Enable TX_DISABLE
  * Disable LP_MODE
* Generic Read/Write Operations
  * Perform I2C byte and word reads on the given module (any device address).

### SFP Bitmaps
SFPs can be numerous and processing their state changes on an individual level is unlikely to be performant for your NOS. As far as the hardware is concerned the state of each module's presence and IO signals are usually communicated as aggregated bitfields in a few registers. This makes collecting information about multiple SFPs at the same time better than collecting them individually. 

The ```onlp_sfp_bitmap_t``` is a bitmap indexed by SFP OID (typically the actual port number) whose value represents the value of a a given SFP state for that port. APIs are provided to return a bitmap for states which are typically asyncronous: 

* ```onlp_sfp_bitmap_get(&bitmap)``` 
  * Returns the bitmap of all valid SFP OIDs. 
* ```onlp_sfp_presence_bitmap_get(&bitmap)``` 
  * Returns the module presence status for all SFP ports. 
  * XORing this bitmap with a previous one will tell you all SFP ports which have changed state (inserted or removed). 
* ```onlp_sfp_rx_los_bitmap_get(&bitmap)```
  * Returns the current value of the RX_LOS signal for each SFP port. 
  * XORing this bitmap with the previous one will tell you all SFP ports which have a changed state (```RX_LOS``` high or low)
  * Applicable to SFP+SFP28 only.
  
It would be possible to aggregate other SFP signals (for example, ```RESET_STATE```, ```LP_MODE```) but these are inputs, not outputs, and thus are controlled explicitly by your software. 

Aggregating ```TX_FAULT``` might still be a useful but no bitmap API is currently specified for it. 


## SFP Documentation

* [Doxygen](http://ocp.opennetlinux.org/onlp/group__oid-sfp.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/sfp.h)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/apis)

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

## SFP Documentation

* [Doxygen](http://ocp.opennetlinux.org/onlp/group__oid-sfp.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/sfp.h)

---
[Return to APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/apis)

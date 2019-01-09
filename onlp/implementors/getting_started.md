# ONLP Platform Implementation APIs

## Getting Started

Adding ONLP support for your platform involves implementing all required interfaces and any optional interfaces your hardware might support.
For those that like to browse the code early all available interfaces are defined [here](https://github.com/opencomputeproject/OpenNetworkLinux/tree/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi).

## ONLP Implementation Subsystems

* platformi - platform identification, selection, and management
* chassisi - Your chassis OID implementation.
* fani - Your fan OID implementation.
* psui - Your PSU OID implementation.
* thermali - Your thermal OID implementation.
* sfpi - Your SFP OID implementation.
* ledi - Your LED OID implementation.
* attributei - You implementation of any OID attributes.

## Initialization

### Platform Discovery and Assignment

When the ONLP layer initializes the first thing it will do is ask your implementation the name of the platform for which it is written. You must implement ```onlp_platformi_get()``` to return the name of your platform implementation.
For example, if your ONL platform is ```x86-64-fantastinet-fnt9000-r0``` then this is what your ```onlp_platformi_get()``` should return.

The ONLP layer compares this to the current platform and if it matches then initialization proceeds.

#### A note about multiple platform support in your implementation
While most implementations will only support a single platform, you may have a scenario in which there are multiple platforms variants supported by your implementation -- usually because there are little or no software visible differences.

In this case you might return something like ```x86-64-fantastinet-fnt9000-rX``` because you want to support both the r0 and r1 versions of your platform. If the name returned by ```onlp_platformi_get()``` does not match the current platform then we will ask you to configure yourself for the current platform
by calling ```onlp_platformi_set("x86-64-fantastinet-fnt9000-r0")```. If this functions returns successfully then we assume you have configured yourself correctly and will continue with initialization.

### Module Software Initialization

After platform assignment is complete all of the following SW init functions will be called prior to any other entry points:

* ```onlp_chassisi_sw_init()```
* ```onlp_modulei_sw_init()```
* ```onlp_thermali_sw_init()```
* ```onlp_fani_sw_init()```
* ```onlp_psui_sw_init()```
* ```onlp_ledi_sw_init()```
* ```onlp_sfpi_sw_init()```
* ```onlp_generici_sw_init()```

All of these functions are optional. You only need to implement the vectors which do something useful. You do not need empty stubs - the default implementations of these vectors just return successfully.

If any of your SW init functions fail then initialization fails. This is not an expected condition and will preclude all other operation.

### API Operations

What happens after initialization is up to the application. Once your modules are initialized then their entry points will be called by the ONLP layer in response to application requests.

---
[Next: ONLP Platform Implementation APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/implementors/apis)

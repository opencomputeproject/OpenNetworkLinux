# ONLP Platform Implementations

Please Note: All Platform Implementations should read the [ONLP Applications API documentation](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/) first to familiarize yourself with the API concepts you will be implementing.

The ONL software architecture is designed to support multiple platforms with the same image and distribution. The ONLP Application API provides unified access to all of these different platforms at runtime.

## ONLP Architecture

The ONLP Common Application API (collectively defined [here](https://github.com/opencomputeproject/OpenNetworkLinux/tree/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp)) is used by system applications and provides common middleware for each system and is the gateway to your implementation.
You implementation libraries contain similar, but reduced functionality necessary to support the features needed by the platform and the upper layer software.

### Platform Shared Libraries

The ONLP Platform Implementation Code is arranged as a shared library written and built specifically for a platform (or family of platforms) by the platform integrator.
The correct shared library for the current platform is selected at early boot time and the correct library links are created.

All of this is handled automatically by the ONL core boot code.

### API Organization

Your APIs are a similar subset of the functionality defined in the application layer.

All APIs for the platform implementor are defined [here](https://github.com/opencomputeproject/OpenNetworkLinux/tree/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platformi).
The following sections will go into more detail on the requires for each subsystem.

### A Note on API Exclusion

The ONLP common layer provides complete exclusion between all calls to the platform implementation across the entire system. This removes the need to manage any concurrency or reentrance issues in your into your code.
You can be guaranteed that no other thread or process in the entire system is accessing your ONLP driver calls at the same time.

This has high value for platform drivers that wish to implement some or all of their functionality in user space. The API exclusion feature means you can reprogram mux trees and access resources for the entirely of the time your entry point is executing.

**You do no need and in general should not add additional locking mechanisms to your code.**

### A Note on OIDs and API Parameters

While the signatures of your entry points are roughly equivalent to the upper layer which proxies it for you it is important to note that the upper layer cleans the incoming OID (of type ```onlp_oid_t```) and sends you only the ID (of type ```onlp_oid_id_t```) as a convenience. No need to extract the ID from the OID.
The upper layer never calls your code until after it has validated that the OID passed to the API is of the correct type.

You should however verify that the ID passed to you refers to a valid object where necessary. Objects may become invalid before the caller has learned it.

### A Note on Required and Optional Functions

The ONLP API represents a union of all defined functionality. Not all features will be available on all platforms.

You do not need to provide all entry points -- only the entry points you support. It is not necessary to provide a stub for every function.
The shared library infrastructure provides additional weak definitions for every possible entry point that return ```ONLP_STATUS_E_UNSUPPORTED```.

This architecture allows new functions to be added to the API without requiring updates to existing platforms. If the platform supports the concept of the new feature then it would be preferable that it get updated but if the platform does not or will not support the new feature then there is no need to modify that platform's implementation in any way.

**Please do not put empty stubs in your implementation and rely instead on the stubs you automatically inherit. It makes the code cleaner, smaller, and more obvious.***

### A Note about Hardware and Software Initialization

Every subsystem supports the concept of Hardware and Software Initialization. The distinction is important and should be note by the platform implementor:

#### The Software Init Routine

Each subsystem has an (optional) routine called ```onlp_<name>_sw_init()```. This function will be called prior to any other call into the same module.
This function must initialize the local software and datastructures for operation but it ***must not change the hardware in any way***. This is simple preparing the software to begin operating on a running system.

Each subsystem has an (optional) routine called ```onlp_<name>_sw_denit()``` for the purposes of completeness and testing.

#### The Hardware Init Routine

Each subsystem has an (optional) routine called ```onlp_<name>_hw_init()```. This routine will be called at early boot time to perform any one-time initialization needed to prepare the system for operation.
ONLP Applications do not and should not typically call this routine but leave it to the boot time infrastructure.

As ONLP Applications are allowed to spawn as multiple processes at any time and in any number the separation of initialization of the local software state and of the platform hardware itself must be cleanly maintained.

---
[Next: Getting Started](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/getting_started)

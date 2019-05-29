# Platform Management

## Platform Fan and Thermal Management

The underlying platform implementation provides the ONLP core with the Fan and Thermal management algorithm.
Calls to this code are handled automatically by the platform daemon and are not generally used by ONLP applications.

## Platform Debug Hooks

The application may call the debug hook ```onlp_platform_debug()``` to issue platform-specific debug commands to the lower layer software.
These are completely optional and depend entirely on the system integrator.

This is generally used for interactive debugging -- for example you might want to dump the contents of all of the system CPLD registers in order to
examine them directly in the case of system debugging or technical support.

While you can call this API it is usually invoked via the command line tools.

## Platform Documentation
* [Doxygen](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/doxygen/html/group__platform.html)
* [Header](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/platform.h)

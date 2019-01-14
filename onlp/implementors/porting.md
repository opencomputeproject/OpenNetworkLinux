# Porting and Testing You Implementation

The ```onlps``` tool is build as a static binary for your platform (like the old ```onlpdump``` program). It builds all of the ONLP core and your platform code together so you can easily build, test, debug, and iterate on your platform.
No shared libraries or setup is required.

## Testing Attributes

While ONLPv1 used the ```sysi``` interface to expose ONIE and Platform information ONLPv2 uses the new attribute interface.
At a minimum you must support the ONIE and Asset attributes on the Chassis OID to port from V1 to V2.

## Testing the ONIE attribute

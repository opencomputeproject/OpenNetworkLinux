============================================================
optoe - EEPROMs on SFP/QSFP/CMIS optoelectronic modules
============================================================

Author: Don Bollinger (don@thebollingers.org)

Description:
============

Optoe is an i2c based driver that supports read/write access to all
the pages (tables) of MSA standard SFP and similar devices (conforming
to the SFF-8472 spec), MSA standard QSFP and similar devices (conforming
to the SFF-8436 or SFF-8636 spec) and CMIS devices (conforming to the
Common Management Interface Specification).

i2c based optoelectronic transceivers (SPF, QSFP, etc) provide identification,
operational status, and control registers via an EEPROM model.  Unlike the
EEPROMs that at24 supports, these devices access data beyond byte 256 via
a page select register, which must be managed by the driver.  optoe 
represents the EEPROM as a linear address space, managing the page register
as needed.  On QSFP and CMIS devices, the first 256 bytes are page 0, followed
by 128 byte pages 1-128.  On SFP devices, the first 256 bytes are from 
i2c address 0x50, followed by 256 bytes from i2c address 0x51, followed
by 128 byte pages 1-128.  See the driver code for a more detailed
explanation.

The EEPROM data is accessible via an nvmem file, e.g.
	/sys/bus/nvmem/devices/port1/nvmem

The EEPROM data is also accessible within the kernel via nvmem calls e.g.
	nvmem = nvmem_device_get(dev, "port1");
	err = nvmem_device_read(nvmem, offset, length, buffer);

The EEPROM data is also accessible via a bin_attribute file called 'eeprom',
e.g. 	/sys/bus/i2c/devices/24-0050/eeprom

Device class:
=============

Note that SFP, QSFP and CMIS type devices are not interchangeable.  The
driver expects the correct ID (optoe<n>) for each port (each i2c device).
It does not check because the port will often be empty, and the only way
to check is to interrogate the device.  Incorrect choice of ID will lead
to CORRECT data being reported for the first 256 bytes (for any ID, for
any actual class device).  Data beyond 256 bytes will be INCORRECT if 
the device doesn't match the optoe<n> type specified.

The device class (1 = QSFP, 2 = SFP, 3 = CMIS) can be read from
/sys/bus/i2c/devices/<i2c device>/dev_class.  It can also be modified
by writing to the same file.  This can be useful when upgrading from QSFP
type devices to CMIS devices (they may have the same form factor) or
when developing with plug-in adapters to convert  QSFP ports for SFP
devices.  It is also useful on development hardware that has both types
of connectors attached to the same i2c bus.

Port name:
==========

optoe maintains a 'port_name' for each device being managed.  The port name
is the device name in the nvmem directory, and the dev_name parameter in
the nvmem_device_get() APIs.

port_name can be set via a device tree property: port_name = "port1";

port_name can also be read/changed by reading/writing to the sysfs file
/sys/bus/i2c/devices/<i2c device>/port_name.  port_name can be any string,
up to 19 characters.  If not initialized, port_name will match the i2c
device name e.g. 1-0050.  If port_name is changed, the nvmem device will
be changed to match.

Device Tree:
============

optoe can be instantiated via Device Tree as a child of the i2c bus
that the device is attached to.

Required properties:
- compatible: shall be one of : 
	'optoe,optoe1' - for QSFP class devices, adhering to SFF-8636
	'optoe,optoe2' - for SFP class devices, adhering to SFF-8472
	'optoe,optoe3' - for CMIS devices (newer QSFP class devices)
- reg: 0x50  The only valid value is 0x50, as all three standards specify that
  the device is at i2c address 0x50.  (optoe allocates an i2c dummy to access
  the data at i2c address 0x51.)

Optional property:
- port_name: can be set to any string up to 19 characters.  Note that the
  actual mapping between i2c busses and network ports is platform dependent
  and varies widely.  The 'port_name' property provides a way to associate
  specific network ports with their associated hardware ports.

Example:
	#address-cells = <1>;
	#size-cells = <0>;
	optoe@50 {
		compatible = "optoe,optoe2";
		reg = <0x50>;
		port_name = "port1";
	};


Dynamic installation:
=====================

Alternatively, optoe can be instantiated with 'new_device', per the convention
described in Documentation/i2c/instantiating-devices.  It wants one of
three possible device identifiers, as described above under 'compatible'.
Use 'optoe1' to indicate this is a QSFP type device, use 'optoe2' to 
indicate this is an SFP type device, use 'optoe3' to indicate this is a
CMIS type device.

Example:
# echo optoe1 0x50 > /sys/bus/i2c/devices/i2c-64/new_device
# echo port54 > /sys/bus/i2c/devices/i2c-64/port_name

This will add a QSFP type device to i2c bus i2c-64, and name it 'port54'

Example:
# echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-11/new_device
# echo port1 > /sys/bus/i2c/devices/i2c-11/port_name

This will add an SFP type device to i2c bus i2c-11, and name it 'port1'

The second parameter to new_device is an i2c address, and MUST be 0x50 for
this driver to work properly.  This is part of the spec for these devices.
(It is not necessary to create a device at 0x51 for SFP type devices, the
driver does that automatically.)

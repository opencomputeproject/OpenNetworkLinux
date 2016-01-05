#!/usr/bin/python
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################

import pprint
import yaml
import json
import os
import re

class OnlInfoObject(object):
    DEFAULT_INDENT="    "

    def __init__(self, d, klass=None):
        self._data = d
        if klass:
            for (m,n) in klass.__dict__.iteritems():
                if m == m.upper():
                    setattr(self, m, None)

                for (k,v) in d.iteritems():
                    for (m,n) in klass.__dict__.iteritems():
                        if n == k:
                            setattr(self, m, v);
                            break

    def __getattr__(self, name):
        if name in self._data:
            return self._data[name]
        else:
            return None

    def __str__(self, indent=DEFAULT_INDENT):
        """String representation of the information container."""
        return OnlInfoObject.string(self._data, indent)

    @staticmethod
    def string(d, indent=DEFAULT_INDENT):
        return "\n".join( sorted("%s%s: %s" % (indent,k,v) for k,v in d.iteritems() if not k.startswith('_') and d[k] is not None) )


############################################################
#
# System-specific information keys.
# These represent information about a particular box.
#
############################################################

class OnieInfo(object):
    PRODUCT_NAME='Product Name'
    PART_NUMBER='Part Number'
    SERIAL_NUMBER='Serial Number'
    MAC_ADDRESS='MAC'
    MAC_RANGE='MAC Range'
    MANUFACTURER='Manufacturer'
    MANUFACTURE_DATE='Manufacture Date'
    VENDOR='Vendor'
    PLATFORM_NAME='Platform Name'
    DEVICE_VERSION='Device Version'
    LABEL_REVISION='Label Revision'
    COUNTRY_CODE='Country Code'
    DIAG_VERSION='Diag Version'
    SERVICE_TAG='Service Tag'
    ONIE_VERSION='ONIE Version'
    oids = {
        PRODUCT_NAME         : '.1',
        PART_NUMBER          : '.2',
        SERIAL_NUMBER        : '.3',
        MAC_ADDRESS          : '.4',
        MAC_RANGE            : '.5',
        MANUFACTURER         : '.6',
        MANUFACTURE_DATE     : '.7',
        VENDOR               : '.8',
        PLATFORM_NAME        : '.9',
        DEVICE_VERSION       : '.10',
        LABEL_REVISION       : '.11',
        COUNTRY_CODE         : '.12',
        DIAG_VERSION         : '.13',
        SERVICE_TAG          : '.14',
        ONIE_VERSION         : '.15',
        }


############################################################
#
# ONL Platform Base
# Baseclass for all OnlPlatform objects.
#
############################################################
class OnlPlatformBase(object):

    CONFIG_DIR='/lib/platform-config'
    CURRENT_DIR=os.path.join(CONFIG_DIR, 'current')

    def __init__(self):
        self.add_info_json("onie_info", "%s/onie-info.json" % self.basedir_onl(), OnieInfo,
                           required=False)
        self.add_info_json("platform_info", "%s/platform-info.json" % self.basedir_onl(),
                           required=False)

    def add_info_dict(self, name, d, klass=None):
        setattr(self, name, OnlInfoObject(d, klass))

    def add_info_json(self, name, f, klass=None, required=True):
        if os.path.exists(f):
            d = json.load(file(f))
            self.add_info_dict(name, d, klass)
        elif required:
            raise RuntimeError("A required system file (%s) is missing." % f)



    def load_configs(self, reload=False):
        if reload or hasattr(self, 'configs') is False:
            self.configs = {}
            for subsys in os.listdir(self.basedir()):
                cpath = os.path.join(self.basedir(), subsys, "configs")
                if os.path.isdir(cpath):
                    for config in os.listdir(cpath):
                        with file(os.path.join(cpath, config)) as f:
                            if not subsys in self.configs:
                                self.configs[subsys] = {}
                            self.configs[subsys][config] = json.load(f)

    def basedir(self, *args):
        return os.path.join(self.CONFIG_DIR, self.platform(), *args)

    def basedir_onl(self, *args):
        return self.basedir('onl', *args)

    def baseconfig(self):
        return True

    def manufacturer(self):
        raise Exception("Manufacturer is not set.")

    def model(self):
        raise Exception("Model is not set.")

    def platform(self):
        raise Exception("Platform is not set.")

    def baseplatform(self):
        p = self.platform()
        p = re.sub(r'-r\d$', '', p)
        return p

    def description(self):
        return "%s %s (%s)" % (self.manufacturer(), self.model(),
                               self.platform())

    def serialnumber(self):
        return self.onie_info.SERIAL_NUMBER

    def hw_description(self):
        return "%s (%s)" % (self.onie_info.PRODUCT_NAME,
                            self.onie_info.PART_NUMBER)


    # ONL Platform Information Tree
    def opit_oid(self):
        return "1.3.6.1.4.1.37538.2.1000"

    # ONL Platform Information General Tree
    def opitg_oid(self):
        return self.opit_oid() + ".1"

    # ONL Platform Information General Sys Tree
    def opitg_sys_oid(self):
        return self.opitg_oid() + ".1"

    # ONL Platform Information Vendor Tree
    def opitv_oid(self):
        return self.opit_oid() + ".2"

    def sys_oid_vendor(self):
        return ".37538"

    def sys_oid_platform(self):
        raise Exception("sys_oid_platform() is not set.")

    def sys_object_id(self):
        return ( self.opitv_oid() +
                 self.sys_oid_vendor() +
                 self.sys_oid_platform());


    def new_device(self, driver, addr, bus, devdir):
        if not os.path.exists(os.path.join(bus, devdir)):
            try:
                with open("%s/new_device" % bus, "w") as f:
                    f.write("%s 0x%x\n" % (driver, addr))
            except Exception, e:
                print "Unexpected error initialize device %s:0x%x:%s: %s" % (driver, addr, bus, e)
        else:
            print("Device %s:%x:%s already exists." % (driver, addr, bus))

    def new_devices(self, new_device_list):
        for (driver, addr, bus, devdir) in new_device_list:
            self.new_device(driver, addr, bus, devdir)

    def new_i2c_device(self, driver, addr, bus_number):
        bus = '/sys/bus/i2c/devices/i2c-%d' % bus_number
        devdir = "%d-%4.4x" % (bus_number, addr)
        return self.new_device(driver, addr, bus, devdir)

    def new_i2c_devices(self, new_device_list):
        for (driver, addr, bus_number) in new_device_list:
            self.new_i2c_device(driver, addr, bus_number)

    def ifnumber(self):
        # The default assumption for any platform
        # is ma1 and lo
        return 2

    def __str__(self):
        s = """Manufacturer: %s
Model: %s
Platform: %s
Description: %s
System Object Id: %s
System Information:
%s
%s
""" % (
            self.manufacturer(),
            self.model(),
            self.platform(),
            self.description(),
            self.sys_object_id(),
            str(self.onie_info),
            str(self.platform_info),
            )


        if hasattr(self, 'warning'):
            s += """

Warning: %s

""" % (self.warning())
        return s







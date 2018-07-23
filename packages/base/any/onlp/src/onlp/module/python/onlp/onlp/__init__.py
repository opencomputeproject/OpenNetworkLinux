"""__init__.py

Module init for onlp.onlp
"""

import ctypes, ctypes.util

libonlp = ctypes.cdll.LoadLibrary(ctypes.util.find_library("onlp"))
libonlp.onlp_sw_init(None)
libonlp.onlp_hw_init(0)

libc = ctypes.cdll.LoadLibrary(ctypes.util.find_library("c"))

libjson_c = ctypes.cdll.LoadLibrary(ctypes.util.find_library("json-c"))

import onlp.onlplib
import sff.sff

from AIM import aim, aim_weakref
aim.libonlp = libonlp
aim.libc = libc

from BigList import biglist
biglist.libonlp = libonlp

from cjson_util import cjson_util
cjson_util.libonlp = libonlp
cjson_util.libjson_c = libjson_c

from onlp.onlp.enums import *

# onlp.yml

##ONLP_CONFIG_INFO_STR_MAX = int(libonlp.onlp_config_lookup("ONLP_CONFIG_INFO_STR_MAX"))
ONLP_CONFIG_INFO_STR_MAX = 64
# prototype for onlp_config_lookup is not defined yet, see below

# onlp/oids.h

onlp_oid = ctypes.c_uint32

ONLP_OID_CHASSIS = (ONLP_OID_TYPE.CHASSIS<<24) | 1
# XXX not a config option

ONLP_OID_DESC_SIZE = 128
ONLP_OID_TABLE_SIZE = 256
# XXX not a config option

class OidTableIterator(object):

    def __init__(self, hdr):
        self.hdr = hdr
        self.idx = 0

    def __iter__(self):
        return self

    def next(self):
        if self.idx >= ONLP_OID_TABLE_SIZE:
            raise StopIteration
        oid = self.hdr.coids[self.idx]
        self.idx += 1
        if oid == 0:
            raise StopIteration
        oidHdr = onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(oidHdr))
        if oid != oidHdr._id:
            raise AssertionError("invalid coid: %d vs %s"
                                 % (oid, oidHdr._id,))
        return oidHdr

onlp_oid_format = ctypes.c_int
onlp_oid_desc = ctypes.c_char * ONLP_OID_DESC_SIZE
onlp_oid_table = onlp_oid * ONLP_OID_TABLE_SIZE

onlp_oid_type_flags = ctypes.c_uint32
onlp_oid_status_flags = ctypes.c_uint32

class onlp_oid_hdr(ctypes.Structure):

    _fields_ = [("_id", onlp_oid,),
                ("description", onlp_oid_desc,),
                ("poid", onlp_oid,),
                ("coids", onlp_oid_table,),
                ("status", onlp_oid_status_flags,),]

    def getType(self):
        return self._id >> 24

    def isChassis(self):
        return self.getType() == ONLP_OID_TYPE.CHASSIS
    def isModule(self):
        return self.getType() == ONLP_OID_TYPE.MODULE
    def isThermal(self):
        return self.getType() == ONLP_OID_TYPE.THERMAL
    def isFan(self):
        return self.getType() == ONLP_OID_TYPE.FAN
    def isPsu(self):
        return self.getType() == ONLP_OID_TYPE.PSU
    def isLed(self):
        return self.getType() == ONLP_OID_TYPE.LED
    def isSfp(self):
        return self.getType() == ONLP_OID_TYPE.SFP
    def isGeneric(self):
        return self.getType() == ONLP_OID_TYPE.GENERIC

    def children(self):
        return OidTableIterator(self)

    # XXX roth -- semantics may not be correct here

    def isPresent(self):
        if self.status & ONLP_OID_STATUS_FLAG.UNPLUGGED:
            return False
        if self.status & ONLP_OID_STATUS_FLAG.PRESENT:
            return True
        return False

    def isMissing(self):
        return not self.isPresent()

    def isFailed(self):
        return self.status & ONLP_OID_STATUS_FLAG.FAILED

    def isNormal(self):
        if not self.isPresent():
            return False
        if self.isFailed():
            return False
        if self.status & ONLP_OID_STATUS_FLAG.OPERATIONAL:
            return True
        return False

class onlp_oid_info(ctypes.Structure):
    """Opaque stub for onlp_XXX_info."""
    _fields_ = [("hdr", onlp_oid_hdr,),]

class onlp_oid_info_ptr(aim_weakref.AimPointer):
    """Opaque malloc-tracked pointer to populated onlp_oid_info."""

    @classmethod
    def deletePointer(cls, ptr):
        if ptr:
            libonlp.aim_free(ptr)

    @property
    def ptr(self):
        return ctypes.cast(self.value, ctypes.POINTER(onlp_oid_info))

    @property
    def contents(self):
        return self.ptr.contents

    @property
    def hnd(self):
        return ctypes.cast(ctypes.byref(self),
                           ctypes.POINTER(ctypes.POINTER(onlp_oid_info)))

onlp_oid_handle = ctypes.POINTER(ctypes.POINTER(onlp_oid_info))
# pointer-to-pointer typedef for receiving a new onlp_oid_info

onlp_oid_iterate_f = ctypes.CFUNCTYPE(ctypes.c_int, onlp_oid, ctypes.c_void_p)

class onlp_oid_info_all(aim_weakref.AimPointer):
    """Opaque malloc-tracked pointer to a BigList of oid info."""

    def __init__(self, ptr=0):
        super(onlp_oid_info_all, self).__init__(ptr)

    @classmethod
    def deletePointer(cls, ptr):
        if ptr:
            libonlp.onlp_oid_get_all_free(ptr)

    @property
    def ptr(self):
        return ctypes.cast(self.value, ctypes.POINTER(biglist.biglist))

    @property
    def hnd(self):
        return ctypes.cast(ctypes.byref(self),
                           ctypes.POINTER(ctypes.POINTER(biglist.biglist)))

def onlp_oid_init_prototypes():

    libonlp.onlp_oid_hdr_get.restype = ctypes.c_int
    libonlp.onlp_oid_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_oid_info_get.restype = ctypes.c_int
    libonlp.onlp_oid_info_get.argtypes = (onlp_oid, onlp_oid_handle,)

    libonlp.onlp_oid_to_str.restype = ctypes.c_int
    libonlp.onlp_oid_to_str.argtypes = (onlp_oid, ctypes.POINTER(ctypes.c_char),)

    libonlp.onlp_oid_to_user_str.restype = ctypes.c_int
    libonlp.onlp_oid_to_user_str.argtypes = (onlp_oid, ctypes.POINTER(ctypes.c_char),)

    libonlp.onlp_oid_table_to_json.restype = ctypes.c_int
    libonlp.onlp_oid_table_to_json.argtypes = (onlp_oid_table, ctypes.POINTER(ctypes.POINTER(cjson_util.cJSON)),)

    libonlp.onlp_oid_hdr_to_json.restype = ctypes.c_int
    libonlp.onlp_oid_hdr_to_json.argtypes = (ctypes.POINTER(onlp_oid_hdr), ctypes.POINTER(ctypes.POINTER(cjson_util.cJSON)),
                                             ctypes.c_uint32,)

    libonlp.onlp_oid_info_to_json.restype = ctypes.c_int
    libonlp.onlp_oid_info_to_json.argtypes = (ctypes.POINTER(onlp_oid_info), ctypes.POINTER(ctypes.POINTER(cjson_util.cJSON)),
                                             ctypes.c_uint32,)

    libonlp.onlp_oid_to_json.restype = ctypes.c_int
    libonlp.onlp_oid_to_json.argtypes = (onlp_oid, ctypes.POINTER(ctypes.POINTER(cjson_util.cJSON)),
                                         ctypes.c_uint32,)

    libonlp.onlp_oid_to_user_json.restype = ctypes.c_int
    libonlp.onlp_oid_to_user_json.argtypes = (onlp_oid, ctypes.POINTER(ctypes.POINTER(cjson_util.cJSON)),
                                              ctypes.c_uint32,)

    libonlp.onlp_oid_iterate.restype = ctypes.c_int
    libonlp.onlp_oid_iterate.argtypes = (onlp_oid, onlp_oid_type_flags,
                                         onlp_oid_iterate_f, ctypes.c_void_p,)

    libonlp.onlp_oid_info_get_all.restype = ctypes.c_int
    libonlp.onlp_oid_info_get_all.argtypes = (onlp_oid, onlp_oid_type_flags, ctypes.c_uint32, biglist.biglist_handle,)

    libonlp.onlp_oid_hdr_get_all.restype = ctypes.c_int
    libonlp.onlp_oid_hdr_get_all.argtypes = (onlp_oid, onlp_oid_type_flags, ctypes.c_uint32, biglist.biglist_handle,)

    libonlp.onlp_oid_get_all_free.restype = ctypes.c_int
    libonlp.onlp_oid_get_all_free.argtypes = (ctypes.POINTER(biglist.biglist),)

# onlp/attribute.h

class AttributeHandle(aim_weakref.AimPointer):
    """Maintain a handle to an attribute object allocated from memory."""

    def __init__(self, attr, klass=None, attrPtr=None):
        super(AttributeHandle, self).__init__(attrPtr)
        self.attr = attr
        self.klass = klass

    @classmethod
    def deletePointer(cls, attrPtr):
        """Override this with the proper delete semantics."""
        if attrPtr is not None and attrPtr != 0:
            libonlp.onlp_attribute_free(onlp.onlp.ONLP_OID_CHASSIS, self.attr, self.attrPtr)

    @property
    def hnd(self):
        return ctypes.byref(self)

    @property
    def contents(self):
        if self.value is None or self.value == 0:
            raise ValueError("NULL pointer")
        if self.klass is not None:
            ptr = ctypes.cast(self.value, ctypes.POINTER(self.klass))
            return ptr.contents
        else:
            raise ValueError("no class type for cast")

def onlp_attribute_init_prototypes():

    libonlp.onlp_attribute_sw_init.restype = ctypes.c_int

    libonlp.onlp_attribute_hw_init.restype = ctypes.c_int
    libonlp.onlp_attribute_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_attribute_supported.restype = ctypes.c_int
    libonlp.onlp_attribute_supported.argtypes = (onlp_oid, ctypes.c_char_p,)

    libonlp.onlp_attribute_set.restype = ctypes.c_int
    libonlp.onlp_attribute_set.argtypes = (onlp_oid, ctypes.c_char_p, ctypes.c_void_p,)

    libonlp.onlp_attribute_get.restype = ctypes.c_int
    libonlp.onlp_attribute_get.argtypes = (onlp_oid, ctypes.c_char_p, ctypes.POINTER(ctypes.c_void_p),)

    libonlp.onlp_attribute_free.restype = ctypes.c_int
    libonlp.onlp_attribute_free.argtypes = (onlp_oid, ctypes.c_char_p, ctypes.c_void_p,)

# onlp/stdattrs.h

ONLP_ATTRIBUTE_ASSET_INFO = "onlp.asset_info"
ONLP_ATTRIBUTE_ASSET_INFO_JSON = "onlp.asset_info_json"
ONLP_ATTRIBUTE_ONIE_INFO = "onlp.attr.onie_info"
ONLP_ATTRIBUTE_ONIE_INFO_JSON = "onlp.attr.onie_info_json"

class onlp_asset_info(ctypes.Structure):
    _fields_ = [("oid", onlp_oid,),
                ("manufacturer", ctypes.c_char_p,),
                ("date", ctypes.c_char_p,),
                ("part_number", ctypes.c_char_p,),
                ("serial_number", ctypes.c_char_p,),
                ("hardware_revision", ctypes.c_char_p,),
                ("firmware_revision", ctypes.c_char_p,),
                ("manufacture_date", ctypes.c_char_p,),
                ("description", ctypes.c_char_p,),
                ("additional", ctypes.c_char_p,),]

def onlp_stdattrs_init_prototypes():

    libonlp.onlp_asset_info_show.restype = ctypes.c_int
    libonlp.onlp_asset_info_show.argtyeps = (ctypes.POINTER(onlp_asset_info), ctypes.POINTER(aim.aim_pvs),)

    libonlp.onlp_asset_info_free.restype = ctypes.c_int
    libonlp.onlp_asset_info_free.argtypes = (ctypes.POINTER(onlp_asset_info),)

# onlp/platform.h

def onlp_platform_init_prototypes():

    libonlp.onlp_platform_name_get.restype = ctypes.c_char_p

    libonlp.onlp_platform_sw_init.restype = ctypes.c_int

    libonlp.onlp_platform_hw_init.restype = ctypes.c_int
    libonlp.onlp_platform_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_platform_manager_start.restype = ctypes.c_int
    libonlp.onlp_platform_manager_start.argtypes = (ctypes.c_int,)

    libonlp.onlp_platform_manager_stop.restype = ctypes.c_int
    libonlp.onlp_platform_manager_stop.argtypes = (ctypes.c_int,)

    libonlp.onlp_platform_manager_join.restype = ctypes.c_int

    ##libonlp.onlp_platform_manager_manage.restype = None
    ### XXX roth -- missing

    libonlp.onlp_platform_manager_daemon.restype = None
    libonlp.onlp_platform_manager_daemon.argtypes = (ctypes.c_char_p, ctypes.c_char_p,
                                                     ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p),)

    ##libonlp.onlp_platform_dump.restype = None
    ##libonlp.onlp_platform_dump.argtypes = (ctypes.POINTER(aim_pvs), ctypes.uint32,)
    ### XXX roth -- missing

    ##libonlp.onlp_platform_show.restype = None
    ##libonlp.onlp_platform_show.argtypes = (ctypes.POINTER(aim_pvs), ctypes.uint32,)
    ### XXX roth -- missing

    ##libonlp.onlp_platform_debug.restype = ctypes.c_int
    ##libonlp.onlp_platform_debug.argtypes = (ctypes.POINTER(aim.aim_pvs),
    ##                                        ctypes.c_int, ctypes.POINTER(ctypes.c_char_p),)
    ### XXX roth -- missing

# onlp/module.h

class onlp_module_info(onlp_oid_info):
    pass

def onlp_module_init_prototypes():

    ##libonlp.onlp_module_sw_init.restype = ctypes.c_int
    ### XXX roth -- missing

    ##libonlp.onlp_module_hw_init.restype = ctypes.c_int
    ##libonlp.onlp_module_hw_init.argtypes = (ctypes.c_uint32,)
    ### XXX roth -- missing

    libonlp.onlp_module_hdr_get.restype = ctypes.c_int
    libonlp.onlp_module_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_module_info_get.restype = ctypes.c_int
    libonlp.onlp_module_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_module_info),)

    libonlp.onlp_module_format.restype = ctypes.c_int
    libonlp.onlp_module_format.argtypes = (onlp_oid, onlp_oid_format,
                                           ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_module_info_format.restype = ctypes.c_int
    libonlp.onlp_module_info_format.argtypes = (ctypes.POINTER(onlp_module_info), onlp_oid_format,
                                                ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

# onlp/chassis.h

class onlp_chassis_info(onlp_oid_info):
    pass

def onlp_chassis_init_prototypes():

    libonlp.onlp_chassis_sw_init.restype = ctypes.c_int

    libonlp.onlp_chassis_hw_init.restype = ctypes.c_int
    libonlp.onlp_chassis_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_chassis_hdr_get.restype = ctypes.c_int
    libonlp.onlp_chassis_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_chassis_info_get.restype = ctypes.c_int
    libonlp.onlp_chassis_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_chassis_info),)

    libonlp.onlp_chassis_format.restype = ctypes.c_int
    libonlp.onlp_chassis_format.argtypes = (onlp_oid, onlp_oid_format,
                                           ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_chassis_info_format.restype = ctypes.c_int
    libonlp.onlp_chassis_info_format.argtypes = (ctypes.POINTER(onlp_chassis_info), onlp_oid_format,
                                                 ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)
# onlp/fan.h

onlp_fan_dir = ctypes.c_int
onlp_fan_mode = ctypes.c_int

class onlp_fan_info(onlp_oid_info):
    _fields_ = [("dir", onlp_fan_dir,),
                ("caps", ctypes.c_uint32,),
                ("rpm", ctypes.c_int,),
                ("percentage", ctypes.c_int,),
                ("model", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),
                ("serial", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),]

def onlp_fan_init_prototypes():

    libonlp.onlp_fan_sw_init.restype = ctypes.c_int

    libonlp.onlp_fan_hw_init.restype = ctypes.c_int
    libonlp.onlp_fan_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_fan_hdr_get.restype = ctypes.c_int
    libonlp.onlp_fan_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_fan_info_get.restype = ctypes.c_int
    libonlp.onlp_fan_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_fan_info),)

    libonlp.onlp_fan_format.restype = ctypes.c_int
    libonlp.onlp_fan_format.argtypes = (onlp_oid, onlp_oid_format,
                                        ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_fan_info_format.restype = ctypes.c_int
    libonlp.onlp_fan_info_format.argtypes = (ctypes.POINTER(onlp_fan_info), onlp_oid_format,
                                             ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_fan_rpm_set.restype = ctypes.c_int
    libonlp.onlp_fan_rpm_set.argtypes = (onlp_oid, ctypes.c_int,)

    libonlp.onlp_fan_percentage_set.restype = ctypes.c_int
    libonlp.onlp_fan_percentage_set.argtypes = (onlp_oid, ctypes.c_int,)

    libonlp.onlp_fan_dir_set.restype = ctypes.c_int
    libonlp.onlp_fan_dir_set.argtypes = (onlp_oid, onlp_fan_dir,)

# onlp/led.h

onlp_led_mode = ctypes.c_uint32

class onlp_led_info(onlp_oid_info):
    _fields_ = [("caps", ctypes.c_uint32,),
                ("mode", onlp_led_mode,),
                ("character", ctypes.c_char,),]

def onlp_led_init_prototypes():

    libonlp.onlp_led_sw_init.restype = ctypes.c_int

    libonlp.onlp_led_hw_init.restype = ctypes.c_int
    libonlp.onlp_led_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_led_hdr_get.restype = ctypes.c_int
    libonlp.onlp_led_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_led_info_get.restype = ctypes.c_int
    libonlp.onlp_led_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_led_info),)

    libonlp.onlp_led_format.restype = ctypes.c_int
    libonlp.onlp_led_format.argtypes = (onlp_oid, onlp_oid_format,
                                        ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_led_info_format.restype = ctypes.c_int
    libonlp.onlp_led_info_format.argtypes = (ctypes.POINTER(onlp_led_info), onlp_oid_format,
                                             ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_led_mode_set.restype = ctypes.c_int
    libonlp.onlp_led_mode_set.argtypes = (onlp_oid, onlp_led_mode,)

    libonlp.onlp_led_char_set.restype = ctypes.c_int
    libonlp.onlp_led_char_set.argtypes = (onlp_oid, ctypes.c_char,)

# onlp/onlp_config.h

# don't need the actual config structure, since we'll be using lookups

def onlp_config_init_prototypes():

    libonlp.onlp_config_lookup.restype = ctypes.c_char_p
    libonlp.onlp_config_lookup.argtypes = (ctypes.c_char_p,)

    libonlp.onlp_config_show.restype = ctypes.c_int
    libonlp.onlp_config_show.argtypes = (ctypes.POINTER(aim.aim_pvs),)

# onlp/thermal.h

ONLP_THERMAL_CAPS_ALL = 0xF

class onlp_thermal_info_thresholds(ctypes.Structure):
    _fields_ = [('warning', ctypes.c_int,),
                ('error', ctypes.c_int,),
                ('shutdown', ctypes.c_int,),]

class onlp_thermal_info(onlp_oid_info):
    _fields_ = [('caps', ctypes.c_uint32,),
                ('mcelcius', ctypes.c_int,),
                ('thresholds', onlp_thermal_info_thresholds,),]

def onlp_thermal_init_prototypes():

    libonlp.onlp_thermal_sw_init.restype = ctypes.c_int

    libonlp.onlp_thermal_hw_init.restype = ctypes.c_int
    libonlp.onlp_thermal_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_thermal_hdr_get.restype = ctypes.c_int
    libonlp.onlp_thermal_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_thermal_info_get.restype = ctypes.c_int
    libonlp.onlp_thermal_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_thermal_info),)

    libonlp.onlp_thermal_format.restype = ctypes.c_int
    libonlp.onlp_thermal_format.argtypes = (onlp_oid, onlp_oid_format,
                                            ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_thermal_info_format.restype = ctypes.c_int
    libonlp.onlp_thermal_info_format.argtypes = (ctypes.POINTER(onlp_thermal_info), onlp_oid_format,
                                                 ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

# onlp/psu.h

onlp_psu_type = ctypes.c_int

class onlp_psu_info(onlp_oid_info):
    _fields_ = [("model", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),
                ("serial", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),
                ("caps", ctypes.c_uint32,),
                ("type", onlp_psu_type,),
                ("mvin", ctypes.c_int,),
                ("mvout", ctypes.c_int,),
                ("miin", ctypes.c_int,),
                ("miout", ctypes.c_int,),
                ("mpin", ctypes.c_int,),
                ("mpout", ctypes.c_int,),]

def onlp_psu_init_prototypes():

    libonlp.onlp_psu_sw_init.restype = ctypes.c_int

    libonlp.onlp_psu_hw_init.restype = ctypes.c_int
    libonlp.onlp_psu_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_psu_hdr_get.restype = ctypes.c_int
    libonlp.onlp_psu_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_psu_info_get.restype = ctypes.c_int
    libonlp.onlp_psu_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_psu_info),)

    libonlp.onlp_psu_format.restype = ctypes.c_int
    libonlp.onlp_psu_format.argtypes = (onlp_oid, onlp_oid_format,
                                        ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_psu_info_format.restype = ctypes.c_int
    libonlp.onlp_psu_info_format.argtypes = (ctypes.POINTER(onlp_psu_info), onlp_oid_format,
                                             ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

# onlp/sfp.h

ONLP_SFP_BLOCK_DATA_SIZE = 256

onlp_sfp_control = ctypes.c_int
onlp_sfp_type = ctypes.c_int
onlp_sfp_bitmap = aim.aim_bitmap256

class onlp_sfp_info_bytes(ctypes.Structure):
    _fields_ = [("a0", ctypes.c_ubyte * ONLP_SFP_BLOCK_DATA_SIZE,),
                ("a2", ctypes.c_ubyte * ONLP_SFP_BLOCK_DATA_SIZE,),]

class onlp_sfp_info(onlp_oid_info):
    _fields_ = [("type", onlp_sfp_type,),
                ("controls", ctypes.c_uint32,),
                ("sff", sff.sff.sff_info,),
                ("dom", sff.sff.sff_dom_info,),
                ("bytes", onlp_sfp_info_bytes,),]

def onlp_sfp_init_prototypes():

    libonlp.onlp_sfp_sw_init.restype = ctypes.c_int

    libonlp.onlp_sfp_hw_init.restype = ctypes.c_int
    libonlp.onlp_sfp_hw_init.argtypes = (ctypes.c_uint32,)

    libonlp.onlp_sfp_bitmap_t_init.restype = None
    libonlp.onlp_sfp_bitmap_t_init.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_bitmap_get.restype = ctypes.c_int
    libonlp.onlp_sfp_bitmap_get.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_info_get.restype = ctypes.c_int
    libonlp.onlp_sfp_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_sfp_info),)

    ##libonlp.onlp_sfp_info_dom_get.restype = ctypes.c_int
    ##libonlp.onlp_sfp_info_dom_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_sfp_info),)
    ### XXX roth -- missing

    libonlp.onlp_sfp_hdr_get.restype = ctypes.c_int
    libonlp.onlp_sfp_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_sfp_port_valid.restype = ctypes.c_int
    libonlp.onlp_sfp_port_valid.argtypes = (ctypes.c_int,)

    libonlp.onlp_sfp_is_present.restype = ctypes.c_int
    libonlp.onlp_sfp_is_present.argtypes = (ctypes.c_int,)

    libonlp.onlp_sfp_presence_bitmap_get.restype = ctypes.c_int
    libonlp.onlp_sfp_presence_bitmap_get.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_rx_los_bitmap_get.restype = ctypes.c_int
    libonlp.onlp_sfp_rx_los_bitmap_get.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_dev_read.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_read.argtypes = (ctypes.c_int, ctypes.c_ubyte,
                                          ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int)

    libonlp.onlp_sfp_dev_write.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_write.argtypes = (ctypes.c_int, ctypes.c_ubyte,
                                           ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int)

    libonlp.onlp_sfp_dev_readb.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_readb.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte,)

    libonlp.onlp_sfp_dev_writeb.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_writeb.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte, ctypes.c_ubyte)

    libonlp.onlp_sfp_dev_readw.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_readw.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte,)

    libonlp.onlp_sfp_dev_writew.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_writew.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte, ctypes.c_ushort)

    libonlp.onlp_sfp_post_insert.restype = ctypes.c_int
    libonlp.onlp_sfp_post_insert.argtypes = (ctypes.c_int, ctypes.POINTER(sff.sff.sff_info),)

    libonlp.onlp_sfp_control_set.restype = ctypes.c_int
    libonlp.onlp_sfp_control_set.argtypes = (ctypes.c_int, onlp_sfp_control, ctypes.c_int,)

    libonlp.onlp_sfp_control_get.restype = ctypes.c_int
    libonlp.onlp_sfp_control_get.argtypes = (ctypes.c_int, onlp_sfp_control, ctypes.POINTER(ctypes.c_int))

    libonlp.onlp_sfp_control_flags_get.restype = ctypes.c_int
    libonlp.onlp_sfp_control_flags_get.argtyeps = (ctypes.c_int, ctypes.POINTER(ctypes.c_uint32),)


    ##libonlp.onlp_sfp_dom_info_get.restype = ctypes.c_int
    ##libonlp.onlp_sfp_dom_info_get.argtypes = (onlp_oid, ctypes.POINTER(sff_dom_info),)
    ### XXX roth -- not implemented

    libonlp.onlp_sfp_format.restype = ctypes.c_int
    libonlp.onlp_sfp_format.argtypes = (onlp_oid, onlp_oid_format,
                                        ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_sfp_info_format.restype = ctypes.c_int
    libonlp.onlp_sfp_info_format.argtypes = (ctypes.POINTER(onlp_sfp_info), onlp_oid_format,
                                             ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    ##libonlp.onlp_sfp_sw_denit.restype = ctypes.c_int
    ##libonlp.onlp_sfp_hw_denit.restype = ctypes.c_int
    ### XXX roth -- not implemented

# onlp/generic.h

class onlp_generic_info(onlp_oid_info):
    pass

def onlp_generic_init_prototypes():

    ##libonlp.onlp_generic_sw_init.restype = ctypes.c_int
    ### XXX roth -- missing

    ##libonlp.onlp_generic_hw_init.restype = ctypes.c_int
    ##libonlp.onlp_generic_hw_init.argtypes = (ctypes.c_uint32,)
    ### XXX roth -- missing

    libonlp.onlp_generic_hdr_get.restype = ctypes.c_int
    libonlp.onlp_generic_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_generic_info_get.restype = ctypes.c_int
    libonlp.onlp_generic_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_generic_info),)

    libonlp.onlp_generic_format.restype = ctypes.c_int
    libonlp.onlp_generic_format.argtypes = (onlp_oid, onlp_oid_format,
                                            ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_generic_info_format.restype = ctypes.c_int
    libonlp.onlp_generic_info_format.argtypes = (ctypes.POINTER(onlp_generic_info), onlp_oid_format,
                                                 ctypes.POINTER(aim.aim_pvs), ctypes.c_uint32,)

# onlp/onlp.h

def init_prototypes():

    aim.aim_memory_init_prototypes(libonlp)
    aim.aim_pvs_init_prototypes(libonlp)
    aim.aim_bitmap_init_prototypes(libonlp)

    biglist.biglist_init_prototypes(libonlp)

    cjson_util.cjson_util_init_prototypes(libonlp, libjson_c)

    onlp.onlplib.onlplib_onie_init_prototypes(libonlp)

    onlp_oid_init_prototypes()
    onlp_attribute_init_prototypes()
    onlp_stdattrs_init_prototypes()

    onlp_platform_init_prototypes()
    onlp_module_init_prototypes()
    onlp_chassis_init_prototypes()

    onlp_fan_init_prototypes()
    onlp_led_init_prototypes()

    onlp_config_init_prototypes()

    strMax = int(libonlp.onlp_config_lookup("ONLP_CONFIG_INFO_STR_MAX"))
    if ONLP_CONFIG_INFO_STR_MAX != strMax:
        raise AssertionError("ONLP_CONFIG_INFO_STR_MAX changed from %d to %d"
                             % (ONLP_CONFIG_INFO_STR_MAX, strMax,))

    onlp_thermal_init_prototypes()
    onlp_psu_init_prototypes()

    sff.sff.sff_sff_init_prototypes(libonlp)

    ##sff.sff.sff_dom_init_prototypes(libonlp)
    ##sff.sff.sff_dom_8436_init_prototypes(libonlp)
    ##sff.sff.sff_dom_8472_init_prototypes(libonlp)
    ##sff.sff.sff_dom_8636_init_prototypes(libonlp)
    ### XXX roth -- missing

    onlp_sfp_init_prototypes()
    onlp_generic_init_prototypes()

init_prototypes()

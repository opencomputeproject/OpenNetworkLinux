"""__init__.py

Module init for onlp.onlp
"""

import ctypes

libonlp = ctypes.cdll.LoadLibrary("libonlp.so")
libonlp.onlp_init()

import ctypes.util
libc = ctypes.cdll.LoadLibrary(ctypes.util.find_library("c"))

import onlp.onlplib
import onlp.sff

from onlp.onlp.enums import *

# AIM/aim_memory.h

class aim_void_p(ctypes.c_void_p):
    """Generic data allocated by AIM."""
    def __del__(self):
        libonlp.aim_free(self)

class aim_char_p(aim_void_p):
    """AIM data that is a printable string."""

    def __init__(self, stringOrAddress):

        if stringOrAddress is None:
            aim_void_p.__init__(self, stringOrAddress)
            return

        if isinstance(stringOrAddress, aim_void_p):
            aim_void_p.__init__(self, stringOrAddress)
            return

        if isinstance(stringOrAddress, basestring):
            cs = ctypes.c_char_p(stringOrAddress)
            ptr = libonlp.aim_malloc(len(stringOrAddress)+1)
            libc.strcpy(ptr, ctypes.addressof(cs))
            aim_void_p.__init__(self, ptr)
            return

        if type(stringOrAddress) == int:
            aim_void_p.__init__(self, stringOrAddress)
            return

        raise ValueError("invalid initializer for aim_char_p: %s"
                         % repr(stringOrAddress))

    def string_at(self):
        if self.value:
            return ctypes.string_at(self.value)
        else:
            return None

    def __eq__(self, other):
        return (isinstance(other, aim_char_p)
                and (self.string_at()==other.string_at()))

    def __neq__(self, other):
        return not self == other

    def __hash__(self):
        return hash(self.string_at())

def aim_memory_init_prototypes():

    libonlp.aim_malloc.restype = aim_void_p
    libonlp.aim_malloc.argtypes = (ctypes.c_size_t,)

    libonlp.aim_free.restype = None
    libonlp.aim_free.argtypes = (aim_void_p,)

# AIM/aim_object.h

aim_object_dtor = ctypes.CFUNCTYPE(None, ctypes.c_void_p)

class aim_object(ctypes.Structure):
    _fields_ = [("_id", ctypes.c_char_p,),
                ("subtype", ctypes.c_int,),
                ("cookie", ctypes.c_void_p,),
                ("destructor", aim_object_dtor,),]

# AIM/aim_pvs.h
# AIM/aim_pvs_*.h

aim_vprintf_f = ctypes.CFUNCTYPE(ctypes.c_int)

class aim_pvs(ctypes.Structure):
    _fields_ = [("object", aim_object,),
                ("description", ctypes.c_char_p,),
                ("vprintf", aim_vprintf_f,),
                ("enabled", ctypes.c_int,),
                ("counter", ctypes.c_uint32,),
                ("isatty", aim_vprintf_f,),]

def aim_pvs_init_prototypes():

    libonlp.aim_pvs_buffer_create.restype = ctypes.POINTER(aim_pvs)

    libonlp.aim_pvs_destroy.restype = None
    libonlp.aim_pvs_destroy.argtypes = (ctypes.POINTER(aim_pvs),)

    libonlp.aim_pvs_buffer_size.restype = ctypes.c_int
    libonlp.aim_pvs_buffer_size.argtypes = (ctypes.POINTER(aim_pvs),)

    libonlp.aim_pvs_buffer_get.restype = aim_char_p
    libonlp.aim_pvs_buffer_get.argtypes = (ctypes.POINTER(aim_pvs),)

    libonlp.aim_pvs_buffer_reset.restype = None
    libonlp.aim_pvs_buffer_reset.argtypes = (ctypes.POINTER(aim_pvs),)

# AIM/aim_bitmap.h

aim_bitmap_word = ctypes.c_uint32
AIM_BITMAP_BITS_PER_WORD = 32

def AIM_BITMAP_WORD_COUNT(bitcount):
    return ((bitcount // AIM_BITMAP_BITS_PER_WORD)
            + (1 if (bitcount % AIM_BITMAP_BITS_PER_WORD) else 0))

# ugh, most of aim_bitmap.h is inline C

def AIM_BITMAP_HDR_WORD_GET(hdr, word):
    """Return a specific ctypes word."""
    return hdr.words[word]

def AIM_BITMAP_HDR_BIT_WORD_GET(hdr, bit):
    """Return the ctypes word holding this bit."""
    return hdr.words[bit/AIM_BITMAP_BITS_PER_WORD]

def AIM_BITMAP_HDR_BIT_WORD_SET(hdr, bit, word):
    """Return the ctypes word holding this bit."""
    hdr.words[bit/AIM_BITMAP_BITS_PER_WORD] = word

def AIM_BITMAP_BIT_POS(bit):
    return (1<<(bit % AIM_BITMAP_BITS_PER_WORD))

def AIM_BITMAP_INIT(bitmap, count):
    """Initialize a static bitmap."""
    libc.memset(ctypes.byref(bitmap), 0, ctypes.sizeof(bitmap))
    bitmap.hdr.maxbit = count
    bitmap.hdr.words = ctypes.cast(ctypes.byref(bitmap.words), ctypes.POINTER(ctypes.c_uint))
    bitmap.hdr.wordcount = AIM_BITMAP_WORD_COUNT(count)

class aim_bitmap_hdr(ctypes.Structure):
    _fields_ = [("wordcount", ctypes.c_int,),
                ("words", ctypes.POINTER(aim_bitmap_word),),
                ("maxbit", ctypes.c_int,),
                ("allocated", ctypes.c_int,),]

class aim_bitmap(ctypes.Structure):
    _fields_ = [("hdr", aim_bitmap_hdr,),]

    @classmethod
    def fromAim(cls, bitcount):
        """Return a pointer to a bitmap from aim_alloc().

        Pre-initialized; needs to be freed with aim_free().
        """
        return libonlp.aim_bitmap_alloc(None, bitcount)

class aim_bitmap256(aim_bitmap):
    """Statically-allocated AIM bitmap."""
    _fields_ = [("words", aim_bitmap_word * AIM_BITMAP_WORD_COUNT(256),),]

    def __init__(self):
        super(aim_bitmap256, self).__init__()
        AIM_BITMAP_INIT(self, 255)

def aim_bitmap_set(hdr, bit):
    word = AIM_BITMAP_HDR_BIT_WORD_GET(hdr, bit)
    word |= AIM_BITMAP_BIT_POS(bit)
    AIM_BITMAP_HDR_BIT_WORD_SET(hdr, bit, word)

def aim_bitmap_clr(hdr, bit):
    word = AIM_BITMAP_HDR_BIT_WORD_GET(hdr, bit)
    word &= ~(AIM_BITMAP_BIT_POS(bit))
    AIM_BITMAP_HDR_BIT_WORD_SET(hdr, bit, word)

def aim_bitmap_mod(hdr, bit, value):
    if value:
        aim_bitmap_set(hdr, bit)
    else:
        aim_bitmap_clr(hdr, bit)

def aim_bitmap_get(hdr, bit):
    val = AIM_BITMAP_HDR_BIT_WORD_GET(hdr,bit) & AIM_BITMAP_BIT_POS(bit)
    return 1 if val else 0

# Huh, these is inline too, but calls into glibc memset

def aim_bitmap_set_all(hdr):
    libc.memset(ctypes.byref(hdr.words), 0xFF, hdr.wordcount*ctypes.sizeof(aim_bitmap_word))

def aim_bitmap_clr_all(hdr):
    libc.memset(ctypes.byref(hdr.words), 0x00, hdr.wordcount*ctypes.sizeof(aim_bitmap_word))

# XXX aim_bitmap_count is left out

def aim_bitmap_init_prototypes():

    libonlp.aim_bitmap_alloc.restype = ctypes.POINTER(aim_bitmap)
    libonlp.aim_bitmap_alloc.argtypes = (ctypes.POINTER(aim_bitmap), ctypes.c_int,)

    libonlp.aim_bitmap_free.restype = None
    libonlp.aim_bitmap_free.argtypes = (ctypes.POINTER(aim_bitmap),)

# onlp.yml

##ONLP_CONFIG_INFO_STR_MAX = int(libonlp.onlp_config_lookup("ONLP_CONFIG_INFO_STR_MAX"))
ONLP_CONFIG_INFO_STR_MAX = 64
# prototype for onlp_config_lookup is not defined yet, see below

# onlp/oids.h

onlp_oid = ctypes.c_uint32

ONLP_OID_SYS = (ONLP_OID_TYPE.SYS<<24) | 1
# XXX not a config option

ONLP_OID_DESC_SIZE = 128
ONLP_OID_TABLE_SIZE = 32
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
        return oidHdr

class onlp_oid_hdr(ctypes.Structure):

    _fields_ = [("_id", onlp_oid,),
                ("description", ctypes.c_char * ONLP_OID_DESC_SIZE,),
                ("poid", onlp_oid,),
                ("coids", onlp_oid * ONLP_OID_TABLE_SIZE,),]

    def getType(self):
        return self._id >> 24

    def isSystem(self):
        return self.getType() == ONLP_OID_TYPE.SYS
    def isThermal(self):
        return self.getType() == ONLP_OID_TYPE.THERMAL
    def isFan(self):
        return self.getType() == ONLP_OID_TYPE.FAN
    def isPsu(self):
        return self.getType() == ONLP_OID_TYPE.PSU
    def isLed(self):
        return self.getType() == ONLP_OID_TYPE.LED
    def isModule(self):
        return self.getType() == ONLP_OID_TYPE.MODULE
    def isRtc(self):
        return self.getType() == ONLP_OID_TYPE.RTC

    def children(self):
        return OidTableIterator(self)

onlp_oid_iterate_f = ctypes.CFUNCTYPE(ctypes.c_int, onlp_oid, ctypes.c_void_p)

def onlp_oid_init_prototypes():

    libonlp.onlp_oid_dump.restype = None
    libonlp.onlp_oid_dump.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_oid_table_dump.restype = None
    libonlp.onlp_oid_table_dump.argtypes = (ctypes.POINTER(onlp_oid), ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_oid_show.restype = None
    libonlp.onlp_oid_show.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_oid_table_show.restype = None
    libonlp.onlp_oid_table_show.argtypes = (ctypes.POINTER(onlp_oid), ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_oid_iterate.restype = ctypes.c_int
    libonlp.onlp_oid_iterate.argtypes = (onlp_oid, ctypes.c_int,
                                         onlp_oid_iterate_f, ctypes.c_void_p,)

    libonlp.onlp_oid_hdr_get.restype = ctypes.c_int
    libonlp.onlp_oid_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr,))

# onlp/sys.h

class onlp_sys_info(ctypes.Structure):

    initialized = False

    _fields_ = [("hdr", onlp_oid_hdr,),
                ("onie_info", onlp.onlplib.onlp_onie_info,),
                ("platform_info", onlp.onlplib.onlp_platform_info,),]

    def __del__(self):
        if self.initialized:
            libonlp.onlp_sys_info_free(ctypes.byref(self))

def onlp_sys_init_prototypes():

    libonlp.onlp_sys_init.restype = ctypes.c_int

    libonlp.onlp_sys_info_get.restype = ctypes.c_int
    libonlp.onlp_sys_info_get.argtypes = (ctypes.POINTER(onlp_sys_info),)

    libonlp.onlp_sys_info_free.restype = None
    libonlp.onlp_sys_info_get.argtypes = (ctypes.POINTER(onlp_sys_info),)

    libonlp.onlp_sys_hdr_get.restype = ctypes.c_int
    libonlp.onlp_sys_hdr_get.argtypes = (ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_sys_dump.restype = None
    libonlp.onlp_sys_dump.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_sys_show.restype = None
    libonlp.onlp_sys_show.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_sys_ioctl.restype = ctypes.c_int
    # leave the parameters empty (varargs)

    ##libonlp.onlp_sys_vioctl.restype = ctypes.c_int
    # NOTE that ctypes cannot automatically handle va_list

    libonlp.onlp_sys_platform_manage_start.restype = ctypes.c_int
    libonlp.onlp_sys_platform_manage_start.argtypes = (ctypes.c_int,)

    libonlp.onlp_sys_platform_manage_stop.restype = ctypes.c_int
    libonlp.onlp_sys_platform_manage_stop.argtypes = (ctypes.c_int,)

    libonlp.onlp_sys_platform_manage_join.restype = ctypes.c_int

    libonlp.onlp_sys_platform_manage_now.restype = None

    libonlp.onlp_sys_debug.restype = ctypes.c_int
    libonlp.onlp_sys_debug.argtypes = (ctypes.POINTER(aim_pvs), ctypes.c_int,
                                       ctypes.POINTER(ctypes.POINTER(ctypes.c_char)),)

# onlp/fan.h

class onlp_fan_info(ctypes.Structure):
    _fields_ = [("hdr", onlp_oid_hdr,),
                ("status", ctypes.c_uint32,),
                ("caps", ctypes.c_uint32,),
                ("rpm", ctypes.c_int,),
                ("percentage", ctypes.c_int,),
                ("mode", ctypes.c_uint32,),
                ("model", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),
                ("serial", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),]

    def isPresent(self):
        return self.status & ONLP_FAN_STATUS.PRESENT

    def isMissing(self):
        return not self.PRESENT()

    def isFailed(self):
        return self.status & ONLP_FAN_STATUS.FAILED

    def isNormal(self):
        return self.isPresent() and not self.isFailed()

def onlp_fan_init_prototypes():

    libonlp.onlp_fan_init.restype = None

    libonlp.onlp_fan_info_get.restype = ctypes.c_int
    libonlp.onlp_fan_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_fan_info),)

    libonlp.onlp_fan_status_get.restype = ctypes.c_int
    libonlp.onlp_fan_status_get.argtypes = (onlp_oid, ctypes.POINTER(ctypes.c_uint32),)

    libonlp.onlp_fan_hdr_get.restype = ctypes.c_int
    libonlp.onlp_fan_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_fan_rpm_set.restype = ctypes.c_int
    libonlp.onlp_fan_rpm_set.argtypes = (onlp_oid, ctypes.c_int,)

    libonlp.onlp_fan_percentage_set.restype = ctypes.c_int
    libonlp.onlp_fan_percentage_set.argtypes = (onlp_oid, ctypes.c_int,)

    libonlp.onlp_fan_mode_set.restype = ctypes.c_int
    libonlp.onlp_fan_mode_set.argtypes = (onlp_oid, ctypes.c_uint32,)

    libonlp.onlp_fan_dir_set.restype = ctypes.c_int
    libonlp.onlp_fan_dir_set.argtypes = (onlp_oid, ctypes.c_uint32,)

    libonlp.onlp_fan_dump.restype = None
    libonlp.onlp_fan_dump.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_fan_show.restype = None
    libonlp.onlp_fan_show.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

# onlp/led.h

class onlp_led_info(ctypes.Structure):
    _fields_ = [("hdr", onlp_oid_hdr,),
                ("status", ctypes.c_uint32,),
                ("caps", ctypes.c_uint32,),
                ("mode", ctypes.c_uint32,),
                ("character", ctypes.c_char,),]

def onlp_led_init_prototypes():

    libonlp.onlp_led_init.restype = None

    libonlp.onlp_led_info_get.restype = ctypes.c_int
    libonlp.onlp_led_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_led_info),)

    libonlp.onlp_led_status_get.restype = ctypes.c_int
    libonlp.onlp_led_status_get.argtypes = (onlp_oid, ctypes.POINTER(ctypes.c_uint32),)

    libonlp.onlp_led_hdr_get.restype = ctypes.c_int
    libonlp.onlp_led_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_led_set.restype = ctypes.c_int
    libonlp.onlp_led_set.argtypes = (onlp_oid, ctypes.c_int,)

    libonlp.onlp_led_mode_set.restype = ctypes.c_int
    libonlp.onlp_led_mode_set.argtypes = (onlp_oid, ctypes.c_uint32,)

    libonlp.onlp_led_char_set.restype = ctypes.c_int
    libonlp.onlp_led_char_set.argtypes = (onlp_oid, ctypes.c_char,)

    libonlp.onlp_led_dump.restype = None
    libonlp.onlp_led_dump.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

    libonlp.onlp_led_show.restype = None
    libonlp.onlp_led_show.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint32,)

# onlp/onlp_config.h

# don't need the actual config structure, since we'll be using lookups

def onlp_config_init_prototypes():

    libonlp.onlp_config_lookup.restype = ctypes.c_char_p
    libonlp.onlp_config_lookup.argtypes = (ctypes.c_char_p,)

    libonlp.onlp_config_show.restype = ctypes.c_int
    libonlp.onlp_config_show.argtypes = (ctypes.POINTER(aim_pvs),)

# onlp/thermal.h

class onlp_thermal_info_thresholds(ctypes.Structure):
    _fields_ = [('warning', ctypes.c_int,),
                ('error', ctypes.c_int,),
                ('shutdown', ctypes.c_int,),]

class onlp_thermal_info(ctypes.Structure):
    _fields_ = [('hdr', onlp_oid_hdr,),
                ('status', ctypes.c_uint32,),
                ('caps', ctypes.c_uint32,),
                ('mcelcius', ctypes.c_int,),
                ('thresholds', onlp_thermal_info_thresholds,),]

def onlp_thermal_init_prototypes():

    libonlp.onlp_thermal_init.restype = ctypes.c_int

    libonlp.onlp_thermal_info_get.restype = ctypes.c_int
    libonlp.onlp_thermal_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_thermal_info),)

    libonlp.onlp_thermal_status_get.restype = ctypes.c_int
    libonlp.onlp_thermal_status_get.argtypes = (onlp_oid, ctypes.POINTER(ctypes.c_uint32),)

    libonlp.onlp_thermal_hdr_get.restype = ctypes.c_int
    libonlp.onlp_thermal_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_thermal_ioctl.restype = ctypes.c_int

    libonlp.onlp_thermal_dump.restype = None
    libonlp.onlp_thermal_dump.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs),)

    libonlp.onlp_thermal_show.restype = None
    libonlp.onlp_thermal_show.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs),)

# onlp/psu.h

class onlp_psu_info(ctypes.Structure):
    _fields_ = [("hdr", onlp_oid_hdr,),
                ("model", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),
                ("serial", ctypes.c_char * ONLP_CONFIG_INFO_STR_MAX,),
                ("status", ctypes.c_uint32,),
                ("caps", ctypes.c_uint32,),
                ("mvin", ctypes.c_int,),
                ("mvout", ctypes.c_int,),
                ("miin", ctypes.c_int,),
                ("miout", ctypes.c_int,),
                ("mpin", ctypes.c_int,),
                ("mpout", ctypes.c_int,),]

def onlp_psu_init_prototypes():

    libonlp.onlp_psu_init.restype = ctypes.c_int

    libonlp.onlp_psu_info_get.restype = ctypes.c_int
    libonlp.onlp_psu_info_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_psu_info),)

    libonlp.onlp_psu_status_get.restype = ctypes.c_int
    libonlp.onlp_psu_status_get.argtypes = (onlp_oid, ctypes.POINTER(ctypes.c_uint32),)

    libonlp.onlp_psu_hdr_get.restype = ctypes.c_int
    libonlp.onlp_psu_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr),)

    libonlp.onlp_psu_ioctl.restype = ctypes.c_int

    libonlp.onlp_psu_dump.restype = None
    libonlp.onlp_psu_dump.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs),)

    libonlp.onlp_psu_show.restype = None
    libonlp.onlp_psu_show.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs),)

# sff/sff.h

def sff_init_prototypes():

    libonlp.sff_sfp_type_get.restype = onlp.sff.sff_sfp_type
    libonlp.sff_sfp_type_get.argtypes = (ctypes.POINTER(ctypes.c_ubyte),)

    libonlp.sff_module_type_get.restype = onlp.sff.sff_module_type
    libonlp.sff_module_type_get.argtypes = (ctypes.POINTER(ctypes.c_ubyte),)

    libonlp.sff_media_type_get.restype = onlp.sff.sff_media_type
    libonlp.sff_media_type_get.argtypes = (onlp.sff.sff_module_type,)

    libonlp.sff_module_caps_get.restype = ctypes.c_int
    libonlp.sff_module_caps_get.argtypes = (onlp.sff.sff_module_type, ctypes.POINTER(ctypes.c_uint32),)

    libonlp.sff_eeprom_parse.restype = ctypes.c_int
    libonlp.sff_eeprom_parse.argtypes = (ctypes.POINTER(onlp.sff.sff_eeprom), ctypes.POINTER(ctypes.c_ubyte),)

    libonlp.sff_eeprom_parse_file.restype = ctypes.c_int
    libonlp.sff_eeprom_parse_file.argtypes = (ctypes.POINTER(onlp.sff.sff_eeprom), ctypes.c_char_p,)

    libonlp.sff_eeprom_invalidate.restype = None
    libonlp.sff_eeprom_invalidate.argtypes = (ctypes.POINTER(onlp.sff.sff_eeprom),)

    libonlp.sff_eeprom_validate.restype = ctypes.c_int
    libonlp.sff_eeprom_validate.argtypes = (ctypes.POINTER(onlp.sff.sff_eeprom), ctypes.c_int,)

    libonlp.sff_info_show.restype = None
    libonlp.sff_info_show.argtypes = (ctypes.POINTER(onlp.sff.sff_info), ctypes.POINTER(aim_pvs),)

    libonlp.sff_info_init.restype = ctypes.c_int
    libonlp.sff_info_init.argtypes = (ctypes.POINTER(onlp.sff.sff_info), onlp.sff.sff_module_type,
                                      ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p,
                                      ctypes.c_int,)

# onlp/sff.h

onlp_sfp_bitmap = aim_bitmap256

onlp_sfp_control = ctypes.c_int

def onlp_sfp_init_prototypes():

    libonlp.onlp_sfp_init.restype = ctypes.c_int

    libonlp.onlp_sfp_bitmap_t_init.restype = None
    libonlp.onlp_sfp_bitmap_t_init.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_bitmap_get.restype = ctypes.c_int
    libonlp.onlp_sfp_bitmap_get.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_port_valid.restype = ctypes.c_int
    libonlp.onlp_sfp_port_valid.argtypes = (ctypes.c_int,)

    libonlp.onlp_sfp_is_present.restype = ctypes.c_int
    libonlp.onlp_sfp_is_present.argtypes = (ctypes.c_int,)

    libonlp.onlp_sfp_presence_bitmap_get.restype = ctypes.c_int
    libonlp.onlp_sfp_presence_bitmap_get.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_eeprom_read.restype = ctypes.c_int
    libonlp.onlp_sfp_eeprom_read.argtypes = (ctypes.c_int, ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte,)),)

    libonlp.onlp_sfp_dom_read.restype = ctypes.c_int
    libonlp.onlp_sfp_dom_read.argtypes = (ctypes.c_int, ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte)),)

    libonlp.onlp_sfp_denit.restype = ctypes.c_int

    libonlp.onlp_sfp_rx_los_bitmap_get.restype = ctypes.c_int
    libonlp.onlp_sfp_rx_los_bitmap_get.argtypes = (ctypes.POINTER(onlp_sfp_bitmap),)

    libonlp.onlp_sfp_dev_readb.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_readb.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte,)

    libonlp.onlp_sfp_dev_writeb.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_writeb.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte, ctypes.c_ubyte)

    libonlp.onlp_sfp_dev_readw.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_readw.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte,)

    libonlp.onlp_sfp_dev_writew.restype = ctypes.c_int
    libonlp.onlp_sfp_dev_writew.argtypes = (ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte, ctypes.c_ushort)

    libonlp.onlp_sfp_dump.restype = None
    libonlp.onlp_sfp_dump.argtypes = (ctypes.POINTER(aim_pvs),)

    libonlp.onlp_sfp_ioctl.restype = ctypes.c_int

    libonlp.onlp_sfp_post_insert.restype = ctypes.c_int
    libonlp.onlp_sfp_post_insert.argtypes = (ctypes.c_int, ctypes.POINTER(onlp.sff.sff_info),)

    libonlp.onlp_sfp_control_set.restype = ctypes.c_int
    libonlp.onlp_sfp_control_set.argtypes = (ctypes.c_int, onlp_sfp_control, ctypes.c_int,)

    libonlp.onlp_sfp_control_get.restype = ctypes.c_int
    libonlp.onlp_sfp_control_get.argtypes = (ctypes.c_int, onlp_sfp_control, ctypes.POINTER(ctypes.c_int))

    libonlp.onlp_sfp_control_flags_get.restype = ctypes.c_int
    libonlp.onlp_sfp_control_flags_get.argtyeps = (ctypes.c_int, ctypes.POINTER(ctypes.c_uint32),)

# onlp/onlp.h

def init_prototypes():
    aim_memory_init_prototypes()
    aim_pvs_init_prototypes()
    aim_bitmap_init_prototypes()
    onlp_oid_init_prototypes()
    onlp_sys_init_prototypes()
    onlp_fan_init_prototypes()
    onlp_led_init_prototypes()

    onlp_config_init_prototypes()

    strMax = int(libonlp.onlp_config_lookup("ONLP_CONFIG_INFO_STR_MAX"))
    if ONLP_CONFIG_INFO_STR_MAX != strMax:
        raise AssertionError("ONLP_CONFIG_INFO_STR_MAX changed from %d to %d"
                             % (ONLP_CONFIG_INFO_STR_MAX, strMax,))

    onlp_thermal_init_prototypes()
    onlp_psu_init_prototypes()
    sff_init_prototypes()
    onlp_sfp_init_prototypes()

init_prototypes()

"""__init__.py

Module init for onlp.onlp
"""

import ctypes

libonlp = ctypes.cdll.LoadLibrary("libonlp.so")

import ctypes.util
libc = ctypes.cdll.LoadLibrary(ctypes.util.find_library("c"))

import onlp.onlplib

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
                ("counter", ctypes.c_uint,),
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

# onlp/oids.h

onlp_oid = ctypes.c_uint

ONLP_OID_DESC_SIZE = 128
ONLP_OID_TABLE_SIZE = 32

ONLP_OID_DUMP_F_RECURSE = 0x1
ONLP_OID_DUMP_F_EVEN_IF_ABSENT = 0x2

ONLP_OID_SHOW_F_RECURSE = 0x1
ONLP_OID_SHOW_F_EXTENDED = 0x2
ONLP_OID_SHOW_F_YAML = 0x4

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

ONLP_OID_TYPE_SYS = 1
ONLP_OID_TYPE_THERMAL = 2
ONLP_OID_TYPE_FAN = 3
ONLP_OID_TYPE_PSU = 4
ONLP_OID_TYPE_LED = 5
ONLP_OID_TYPE_MODULE = 6
ONLP_OID_TYPE_RTC = 7
# XXX roth waiting for enum generator

class onlp_oid_hdr(ctypes.Structure):

    _fields_ = [("_id", onlp_oid,),
                ("description", ctypes.c_char * ONLP_OID_DESC_SIZE,),
                ("poid", onlp_oid,),
                ("coids", onlp_oid * ONLP_OID_TABLE_SIZE,),]

    def getType(self):
        return self._id >> 24

    def isSystem(self):
        return self.getType() == ONLP_OID_TYPE_SYS
    def isThermal(self):
        return self.getType() == ONLP_OID_TYPE_THERMAL
    def isFan(self):
        return self.getType() == ONLP_OID_TYPE_FAN
    def isPsu(self):
        return self.getType() == ONLP_OID_TYPE_PSU
    def isLed(self):
        return self.getType() == ONLP_OID_TYPE_LED
    def isModule(self):
        return self.getType() == ONLP_OID_TYPE_MODULE
    def isRtc(self):
        return self.getType() == ONLP_OID_TYPE_RTC

    def children(self):
        return OidTableIterator(self)

onlp_oid_iterate_f = ctypes.CFUNCTYPE(ctypes.c_int, onlp_oid, ctypes.c_void_p)

def onlp_oid_init_prototypes():

    #onlp_oid_dump
    #onlp_oid_table_dump
    #onlp_oid_show
    #onlp_oid_table_show

    libonlp.onlp_oid_iterate.restype = ctypes.c_int
    libonlp.onlp_oid_iterate.argtypes = (onlp_oid, ctypes.c_int,
                                         onlp_oid_iterate_f, ctypes.c_void_p,)
    # XXX enum

    libonlp.onlp_oid_hdr_get.restype = ctypes.c_int
    libonlp.onlp_oid_hdr_get.argtypes = (onlp_oid, ctypes.POINTER(onlp_oid_hdr,))
    # XXX enum

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
    libonlp.onlp_sys_dump.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint,)

    libonlp.onlp_sys_show.restype = None
    libonlp.onlp_sys_show.argtypes = (onlp_oid, ctypes.POINTER(aim_pvs), ctypes.c_uint,)

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

# onlp/onlp.h

def onlp_init():
    libonlp.onlp_init()
    aim_memory_init_prototypes()
    aim_pvs_init_prototypes()
    onlp_oid_init_prototypes()
    onlp_sys_init_prototypes()

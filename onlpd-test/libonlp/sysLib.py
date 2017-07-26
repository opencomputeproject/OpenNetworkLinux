import ctypes
from time import sleep

syslib = ctypes.CDLL('/lib/x86_64-linux-gnu/libonlp.so')

class onlp_onie_info_t(ctypes.Structure):
    _fields_ = [("product_name",ctypes.c_char_p),
                ("part_number",ctypes.c_char_p),
                ("serial_number",ctypes.c_char_p),
                ("MAC",ctypes.c_uint),
                ("manufacture_date",ctypes.c_char_p),
                ("device_version",ctypes.c_uint),
                ("label_revision",ctypes.c_char_p),
                ("platform_name",ctypes.c_char_p),
                ("onie_version",ctypes.c_char_p),
                ("mac_range",ctypes.c_uint),
                ("manufacturer",ctypes.c_char_p),
                ("country_code",ctypes.c_char_p),
                ("vendor",ctypes.c_char_p),
                ("diag_version",ctypes.c_char_p),
                ("service_tag",ctypes.c_char_p),
                ("crc",ctypes.c_uint * 32)]

class onlp_oid_hdr_t(ctypes.Structure):
    _fields_ = [("id", ctypes.c_uint),
               ("description", ctypes.c_char * 128),
               ("poid", ctypes.c_uint),
               ("coids", ctypes.c_uint * 32)]

class onlp_platform_info_t(ctypes.Structure):
    _fields_ = [("cpld_versions",ctypes.c_char_p),
                ("other versions",ctypes.c_char_p)]


class onlp_sys_info_t(ctypes.Structure):
    _fields_ = [("hdr",onlp_oid_hdr_t),
                ("onie_info",onlp_onie_info_t),
                ("platform",onlp_platform_info_t)]

class sys:
    def __init__(self,id):
        syslib.onlp_sys_init()
        onlp_sys = onlp_sys_info_t()
        syslib.onlp_sys_info_get(ctypes.byref(onlp_sys))
        self.sys_oid = id
        self.hdr = onlp_sys.hdr
        self.onie_info = onlp_sys.onie_info
        self.platform = onlp_sys.platform

syslist = [] #List to store the system object

"""
Initialize the system API
"""
syslib.onlp_sys_init.restype = ctypes.c_int

"""
Get the system information Structure
Parameter: Recieves the system Structure
"""
syslib.onlp_sys_info_get.argtypes = [ctypes.POINTER(onlp_sys_info_t)]

def get_sys():
    global sys_oid
    sys_oid = 0x1000001
    sys1 = sys(sys_oid)
    print "\n"
    print "Product name: ",sys1.onie_info.product_name
    print "Serial Number: ",sys1.onie_info.serial_number
    print "MAC Range: ",sys1.onie_info.mac_range
    print "Manufacturer: ",sys1.onie_info.manufacturer
    print "Manufacture Date: ",sys1.onie_info.manufacture_date
    print "Country code: ",sys1.onie_info.country_code
    print "Diag Version: ",sys1.onie_info.diag_version
    print "ONIE Version: ",sys1.onie_info.onie_version
    syslist.append(sys1)
    return syslist

import ctypes
from time import sleep

psulib = ctypes.CDLL('/lib/x86_64-linux-gnu/libonlp.so')

class onlp_oid_hdr_t(ctypes.Structure):
    _fields_ = [("id", ctypes.c_uint),
               ("description", ctypes.c_char * 128),
               ("poid", ctypes.c_uint),
               ("coids", ctypes.c_uint * 32)]

class onlp_psu_caps_t(ctypes.Structure):
    ONLP_PSU_CAPS_AC = (1 << 0)
    ONLP_PSU_CAPS_DC12 = (1 << 1)
    ONLP_PSU_CAPS_DC48 = (1 << 2)
    ONLP_PSU_CAPS_VIN = (1 << 3)
    ONLP_PSU_CAPS_VOUT = (1 << 4)
    ONLP_PSU_CAPS_IIN = (1 << 5)
    ONLP_PSU_CAPS_IOUT = (1 << 6)
    ONLP_PSU_CAPS_PIN = (1 << 7)
    ONLP_PSU_CAPS_POUT = (1 << 8)

class onlp_psu_status_t(ctypes.Structure):
    ONLP_PSU_STATUS_PRESENT = (1 << 0)
    ONLP_PSU_STATUS_FAILED = (1 << 1)
    ONLP_PSU_STATUS_UNPLUGGED = (1 << 2)

class onlp_psu_info_t(ctypes.Structure):
    _fields_ = [("hdr",onlp_oid_hdr_t),
                ("model",ctypes.c_char * 64),
                ("serial",ctypes.c_char * 64),
                ("status",ctypes.c_uint),
                ("caps",ctypes.c_uint),
                ("mvin",ctypes.c_int), #millivolts
                ("mvout",ctypes.c_int),
                ("miin",ctypes.c_int), #milliamps
                ("miout",ctypes.c_int),
                ("mpin",ctypes.c_int), #milliwatts
                ("mpout",ctypes.c_int)]

"""
Initialize the PSU subsystem
"""
psulib.onlp_psu_init.restype = ctypes.c_int

"""
Get the PSU information
Parameter 1: The PSU OID
Parameter 2: Recieves the information Structure
"""
psulib.onlp_psu_info_get.argtypes = [ctypes.c_uint,ctypes.POINTER(onlp_psu_info_t)]
psulib.onlp_psu_info_get.restype = ctypes.c_int

psulist = []

class psu:
    def __init__(self,id):
        psulib.onlp_psu_init()
        onlp_psu = onlp_psu_info_t()
        psulib.onlp_psu_info_get(psu_oid, ctypes.byref(onlp_psu))
        self.psu_oid = id
        self.hdr = onlp_psu.hdr
        self.model = onlp_psu.model
        self.serial = onlp_psu.serial
        self.status = onlp_psu.status
        self.caps = onlp_psu.caps
        self.mvin = onlp_psu.mvin
        self.mvout = onlp_psu.mvout
        self.miin = onlp_psu.miin
        self.miout = onlp_psu.miout
        self.mpin = onlp_psu.mpin
        self.mpout = onlp_psu.mpout

def get_psus():
    global psu_oid
    psu_oid = 0x4000001

    while(True):
        sleep(1)
        psu1 = psu(psu_oid)
        if(psu1.status):
            print "PSU Description: ",psu1.hdr.description
            print "model:",psu1.model
            print "serial:",psu1.serial
            print "status:",psu1.status
            print "caps:",psu1.caps
            print "mvin:",psu1.mvin
            print "mvout:",psu1.mvout
            print "miin:",psu1.miin
            print "miout:",psu1.miout
            print "mpin:",psu1.mpin
            print "mpout:",psu1.mpout
            print "\n"
            psulist.append(psu1)
        else:
            break
        psu_oid = psu_oid + 1
    return psulist

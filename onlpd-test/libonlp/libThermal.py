import ctypes
from time import sleep

ThermalLib = ctypes.CDLL('/lib/x86_64-linux-gnu/libonlp.so')

class onlp_thermal_caps_t(ctypes.Structure):
    ONLP_THERMAL_CAPS_GET_TEMPERATURE = (1 << 0)
    ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD = (1 << 1)
    ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD = (1 << 2)
    ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD = (1 << 3)

class onlp_thermal_status_t(ctypes.Structure):
    ONLP_THERMAL_STATUS_PRESENT = (1<<0)
    ONLP_THERMAL_STATUS_FAILED = (1<<1)

class onlp_thermal_threshold_t(ctypes.Structure):
    ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT = 45000
    ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT = 55000
    ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT = 60000

class onlp_oid_hdr_t(ctypes.Structure):
    _fields_ = [("id", ctypes.c_uint),
               ("description", ctypes.c_char * 128),
               ("poid", ctypes.c_uint),
               ("coids", ctypes.c_uint * 32)]

class onlp_thermal_info_t(ctypes.Structure):
    _fields_ = [("hdr",onlp_oid_hdr_t),
                ("status",ctypes.c_uint),
                ("caps",ctypes.c_uint),
                ("mcelcius",ctypes.c_int),
                ("warning",ctypes.c_int),
                ("error",ctypes.c_int),
                ("shutdown",ctypes.c_int)]

"""
Initialize the thermal subsystem
Parameter: void
"""
ThermalLib.restype = ctypes.c_int

"""
Retrieve the thermal's operational status
Parameter 1: The thermal OID
Parameter 2: Recieves the thermal information
"""
ThermalLib.onlp_thermal_info_get.argtypes = [ctypes.c_uint, ctypes.POINTER(onlp_thermal_info_t)]
ThermalLib.onlp_thermal_info_get.restype = ctypes.c_int

Thermallist = []

class thermal:
    ThermalLib.onlp_thermal_init()
    onlp_thermal = onlp_thermal_info_t()
    def __init__(self,id):
        ThermalLib.onlp_thermal_info_get(thermal_oid, ctypes.byref(self.onlp_thermal))
        self.thermal_oid = id
        self.hdr = self.onlp_thermal.hdr
        self.status = self.onlp_thermal.status
        self.caps = self.onlp_thermal.caps
        self.mcelcius = self.onlp_thermal.mcelcius
        self.warning = self.onlp_thermal.warning
        self.error = self.onlp_thermal.error
        self.shutdown = self.onlp_thermal.shutdown

def get_thermals():
    global thermal_oid
    thermal_oid = 0x2000001

    while(True):
        sleep(1)
        thermal1 = thermal(thermal_oid)
        if(thermal1.status == 1):
            print "Thermal:",thermal1.hdr.description
            print "Status:",thermal1.status
            print "Caps",thermal1.caps
            print "Temperature(in millicelcius):",thermal1.mcelcius
            print "Thresholds Warning:",thermal1.warning
            print "Thresholds error:",thermal1.error
            print "Thresholds shutdown:",thermal1.shutdown
            print "\n"
            Thermallist.append(thermal1)
        else:
            break
        thermal_oid = thermal_oid + 1
    return Thermallist

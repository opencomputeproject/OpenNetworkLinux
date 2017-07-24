import ctypes
from time import sleep

testlib = ctypes.CDLL('/lib/x86_64-linux-gnu/libonlp.so')

class onlp_fan_caps_t(ctypes.Structure):
    ONLP_FAN_CAPS_B2F = (1 << 0)
    ONLP_FAN_CAPS_F2B = (1 << 1)
    ONLP_FAN_CAPS_SET_RPM = (1 << 2)
    ONLP_FAN_CAPS_SET_PERCENTAGE = (1 << 3)
    ONLP_FAN_CAPS_GET_RPM = (1 << 4)
    ONLP_FAN_CAPS_GET_PERCENTAGE = (1 << 5)


class onlp_fan_status_t(ctypes.Structure):
    ONLP_FAN_STATUS_PRESENT = (1 << 0)
    ONLP_FAN_STATUS_FAILED = (1 << 1)
    ONLP_FAN_STATUS_B2F = (1 << 2)
    ONLP_FAN_STATUS_F2B = (1 << 3)

onlp_fan_status = onlp_fan_status_t()

class onlp_fan_dir_t(ctypes.Structure):
    ONLP_FAN_DIR_B2F = 1
    ONLP_FAN_DIR_F2B = 2
    ONLP_FAN_DIR_LAST = ONLP_FAN_DIR_F2B
    ONLP_FAN_DIR_COUNT = 3
    ONLP_FAN_DIR_INVALID = -1

class onlp_fan_mode_t:
        ONLP_FAN_MODE_OFF = 1,
        ONLP_FAN_MODE_SLOW = 2,
        ONLP_FAN_MODE_NORMAL = 3,
        ONLP_FAN_MODE_FAST = 4,
        ONLP_FAN_MODE_MAX = 5,
        ONLP_FAN_MODE_LAST = 5,
        ONLP_FAN_MODE_COUNT= 6,
        ONLP_FAN_MODE_INVALID = -1

class onlp_oid_hdr_t(ctypes.Structure):
    _fields_ = [("id", ctypes.c_uint),
               ("description", ctypes.c_char * 128),
               ("poid", ctypes.c_uint),
               ("coids", ctypes.c_uint * 32)]

class onlp_fan_info_t(ctypes.Structure):
    _fields_ = [("hdr", onlp_oid_hdr_t),
               ("status", ctypes.c_uint),
               ("caps", ctypes.c_uint),
               ("rpm", ctypes.c_int),
               ("percentage", ctypes.c_int),
               ("mode", ctypes.c_int),
               ("model", ctypes.c_char * 64),
               ("serial",ctypes.c_char * 64)]


"""
Retrieve fan information
Parameter 1(id): The fan OID
Parameter 2(rv): Recieves fan information
"""

testlib.onlp_fan_info_get.argtypes = [ctypes.c_uint, ctypes.POINTER(onlp_fan_info_t)]
testlib.onlp_fan_info_get.restype = ctypes.c_int

#Initialize the fan subsystem
testlib.onlp_fan_init()
testlib.onlp_fan_init.restype = ctypes.c_int


"""
Set the fan speed in rpm
Parameter 1(id):The fan OID
Parameter 2(rpm): The new rpm
Note: Only valid if the fan has the SET_RPM capability
"""
testlib.onlp_fan_rpm_set.argtypes = [ctypes.c_uint,ctypes.c_int]
testlib.onlp_fan_rpm_set.restype = ctypes.c_int

"""
Set the fan speed in percentage.
Parameter 1: The fan OID.
Parameter 2: The percentage.
Note: Only valid if the fan has the SET_PERCENTAGE capability.
"""
testlib.onlp_fan_percentage_set.argtypes = [ctypes.c_uint,ctypes.c_int]
testlib.onlp_fan_percentage_set.restype = ctypes.c_int

"""
Set the fan by mode
Parameter 1: The fan id
Parameter 2: The fan mode value
"""
testlib.onlp_fan_mode_set.argtypes = [ctypes.c_uint,ctypes.c_uint]
testlib.onlp_fan_mode_set.restype = ctypes.c_int

"""
Set the fan direction
Parameter 1: The fan OID
Parameter 2: The fan direction (B2F or F2B)
Note: Only called if both capabilities are set
"""

testlib.onlp_fan_dir_set.argtypes = [ctypes.c_uint,ctypes.c_int]
testlib.onlp_fan_dir_set.restype = ctypes.c_int

"""
Fan OID debug dump
Parameter 1: The fan OID
Parameter 2: The output pvs
Parameter 3: The output flags
"""

testlib.onlp_fan_dump.argtypes = [ctypes.c_uint,ctypes.POINTER(ctypes.c_uint),ctypes.c_uint]
testlib.onlp_fan_dump.restype = ctypes.c_void_p

"""
Show the given Fan OID
Parameter 1: The fan OID
Parameter 2: The output pvs
Parameter 3: The output flags
"""
testlib.onlp_fan_show.argtypes = [ctypes.c_uint,ctypes.POINTER(ctypes.c_uint),ctypes.c_uint]
testlib.onlp_fan_show.restype = ctypes.c_void_p

fanlist = []
testlib.onlp_fan_init()

class fan:
    def __init__(self,id):
        onlp_fan = onlp_fan_info_t()
        testlib.onlp_fan_info_get(fan_oid, ctypes.byref(onlp_fan))
        self.fan_oid = id
        self.hdr = onlp_fan.hdr
        self.status = onlp_fan.status
        self.caps = onlp_fan.caps
        self.rpm = onlp_fan.rpm
        self.percentage = onlp_fan.percentage
        self.mode = onlp_fan.mode
        self.model = onlp_fan.model

    def get_caps(self):
        index = []
        x = int(bin(self.caps)[2:])
        y = len(str(x))
        capability = ['ONLP_FAN_CAPS_B2F','ONLP_FAN_CAPS_F2B','ONLP_FAN_CAPS_SET_RPM','ONLP_FAN_CAPS_SET_PERCENTAGE','ONLP_FAN_CAPS_GET_RPM','ONLP_FAN_CAPS_GET_PERCENTAGE']
        for i in range(y):
            if((x & (1 << i))!=0):
                index.append(capability[i])
        print "CAPS for",self.hdr.description," :",index
        print "\n"
        del index

    def set_normal_speed(self):
        testlib.onlp_fan_rpm_set(self.fan_oid,8000)

    def set_normal_percent(self):
        testlib.onlp_fan_percentage_set(self.fan_oid,47)

    def set_rpm(self,user_rpm):
        if(self.caps & (1<<2)):
            testlib.onlp_fan_rpm_set(self.fan_oid,user_rpm)
            print "Setting rpm of",self.hdr.description,self.fan_oid," to ",user_rpm
            sleep(10)
            onlp_fan = onlp_fan_info_t()
            testlib.onlp_fan_info_get(self.fan_oid, ctypes.byref(onlp_fan))
            print "RPM of",self.hdr.description,"(after setting):",onlp_fan.rpm,"\n"
            return onlp_fan.rpm
        else:
            print "SET_RPM is not enabled for ",self.hdr.description
            print "\n"

    def get_rpm(self):
        if(self.caps & (1<<4)):
            onlp_fan = onlp_fan_info_t()
            testlib.onlp_fan_info_get(self.fan_oid, ctypes.byref(onlp_fan))
            print "RPM:",onlp_fan.rpm
            return onlp_fan.rpm

    def set_percent(self,user_percent):
        if(self.caps & (1 << 3)):
            print "Setting Percentage of ",self.hdr.description,"to ",user_percent
            testlib.onlp_fan_percentage_set(self.fan_oid,user_percent)
            sleep(10)
            onlp_fan = onlp_fan_info_t()
            testlib.onlp_fan_info_get(self.fan_oid, ctypes.byref(onlp_fan))
            print "Current Percentage(after setting):",onlp_fan.percentage
            print "\n"
            return onlp_fan.percentage
        else:
            print "SET_PERCENTAGE is not enabled for",self.hdr.description
            print "\n"


    def get_percent(self):
        if(self.caps & (1<<3)):
            onlp_fan = onlp_fan_info_t()
            testlib.onlp_fan_info_get(self.fan_oid, ctypes.byref(onlp_fan))

    def set_mode(self,user_mode):
        return testlib.onlp_fan_mode_set(self.fan_oid,user_mode)

    def set_direction(self,user_direction):
        if((self.caps & (1<<0)) | (self.caps & (1<<1))):
            return testlib.onlp_fan_dir_set(self.fan_oid,user_direction)
        else:
            print "SET_DIRECTION is not enabled"

    def print_all(self):
        onlp_fan = onlp_fan_info_t()
        testlib.onlp_fan_info_get(self.fan_oid, ctypes.byref(onlp_fan))
        print "Fan OID: ",self.fan_oid
        print "Description: ",onlp_fan.hdr.description
        print "RPM: ",onlp_fan.rpm
        print "Status: ",onlp_fan.status
        print "CAPS: ",onlp_fan.percentage
        print "Percentage: ",onlp_fan.percentage
        print "Mode: ",onlp_fan.mode
        print "Model: ",onlp_fan.model

def get_fans():
    global fan_oid
    fan_oid = 0x3000001

    while(True):
        fanl = fan(fan_oid)
        if((fanl.status == 1) | (fanl.status == 5) | (fanl.status == 13)) :
            print "fan:",fanl.hdr.description
            print "status:",fanl.status
            print "caps:",fanl.caps
            print "\n"
            fanlist.append(fanl)
        else:
            break
        fan_oid = fan_oid + 1
    return fanlist

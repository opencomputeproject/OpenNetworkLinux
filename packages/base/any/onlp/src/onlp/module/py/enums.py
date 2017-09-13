#!/usr/bin/python
############################################################
#
# These are all ONLP Enumeration values
# for use with the Python API.
#
############################################################
class Enumeration(object):
    @classmethod
    def name(klass, value):
        for (k, v) in klass.__dict__.iteritems():
            if v == value:
                return k
        return None

# <auto.start.pyenum(ALL).define>
class ONLP_FAN_CAPS(Enumeration):
    B2F = (1 << 0)
    F2B = (1 << 1)
    SET_RPM = (1 << 2)
    SET_PERCENTAGE = (1 << 3)
    GET_RPM = (1 << 4)
    GET_PERCENTAGE = (1 << 5)


class ONLP_FAN_DIR(Enumeration):
    B2F = 0
    F2B = 1


class ONLP_FAN_MODE(Enumeration):
    OFF = 0
    SLOW = 1
    NORMAL = 2
    FAST = 3
    MAX = 4


class ONLP_FAN_STATUS(Enumeration):
    PRESENT = (1 << 0)
    FAILED = (1 << 1)
    B2F = (1 << 2)
    F2B = (1 << 3)


class ONLP_LED_CAPS(Enumeration):
    ON_OFF = (1 << 0)
    CHAR = (1 << 1)
    RED = (1 << 10)
    RED_BLINKING = (1 << 11)
    ORANGE = (1 << 12)
    ORANGE_BLINKING = (1 << 13)
    YELLOW = (1 << 14)
    YELLOW_BLINKING = (1 << 15)
    GREEN = (1 << 16)
    GREEN_BLINKING = (1 << 17)
    BLUE = (1 << 18)
    BLUE_BLINKING = (1 << 19)
    PURPLE = (1 << 20)
    PURPLE_BLINKING = (1 << 21)
    AUTO = (1 << 22)
    AUTO_BLINKING = (1 << 23)


class ONLP_LED_MODE(Enumeration):
    OFF = 0
    ON = 1
    BLINKING = 2
    RED = 10
    RED_BLINKING = 11
    ORANGE = 12
    ORANGE_BLINKING = 13
    YELLOW = 14
    YELLOW_BLINKING = 15
    GREEN = 16
    GREEN_BLINKING = 17
    BLUE = 18
    BLUE_BLINKING = 19
    PURPLE = 20
    PURPLE_BLINKING = 21
    AUTO = 22
    AUTO_BLINKING = 23


class ONLP_LED_STATUS(Enumeration):
    PRESENT = (1 << 0)
    FAILED = (1 << 1)
    ON = (1 << 2)


class ONLP_OID_TYPE(Enumeration):
    SYS = 1
    THERMAL = 2
    FAN = 3
    PSU = 4
    LED = 5
    MODULE = 6
    RTC = 7


class ONLP_PSU_CAPS(Enumeration):
    AC = (1 << 0)
    DC12 = (1 << 1)
    DC48 = (1 << 2)
    VIN = (1 << 3)
    VOUT = (1 << 4)
    IIN = (1 << 5)
    IOUT = (1 << 6)
    PIN = (1 << 7)
    POUT = (1 << 8)


class ONLP_PSU_STATUS(Enumeration):
    PRESENT = (1 << 0)
    FAILED = (1 << 1)
    UNPLUGGED = (1 << 2)


class ONLP_SFP_CONTROL(Enumeration):
    RESET = 0
    RESET_STATE = 1
    RX_LOS = 2
    TX_FAULT = 3
    TX_DISABLE = 4
    TX_DISABLE_CHANNEL = 5
    LP_MODE = 6
    POWER_OVERRIDE = 7


class ONLP_SFP_CONTROL_FLAG(Enumeration):
    RESET = (1 << 0)
    RESET_STATE = (1 << 1)
    RX_LOS = (1 << 2)
    TX_FAULT = (1 << 3)
    TX_DISABLE = (1 << 4)
    TX_DISABLE_CHANNEL = (1 << 5)
    LP_MODE = (1 << 6)
    POWER_OVERRIDE = (1 << 7)


class ONLP_STATUS(Enumeration):
    OK = 0
    E_GENERIC = -1
    E_UNSUPPORTED = -10
    E_MISSING = -11
    E_INVALID = -12
    E_INTERNAL = -13
    E_PARAM = -14
    E_I2C = -15


class ONLP_THERMAL_CAPS(Enumeration):
    GET_TEMPERATURE = (1 << 0)
    GET_WARNING_THRESHOLD = (1 << 1)
    GET_ERROR_THRESHOLD = (1 << 2)
    GET_SHUTDOWN_THRESHOLD = (1 << 3)


class ONLP_THERMAL_STATUS(Enumeration):
    PRESENT = (1 << 0)
    FAILED = (1 << 1)


class ONLP_THERMAL_THRESHOLD(Enumeration):
    WARNING_DEFAULT = 45000
    ERROR_DEFAULT = 55000
    SHUTDOWN_DEFAULT = 60000

# <auto.end.pyenum(ALL).define>

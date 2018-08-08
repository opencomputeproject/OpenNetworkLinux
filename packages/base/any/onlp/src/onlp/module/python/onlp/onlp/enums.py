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
    SET_DIR = (1 << 0)
    GET_DIR = (1 << 1)
    SET_RPM = (1 << 2)
    SET_PERCENTAGE = (1 << 3)
    GET_RPM = (1 << 4)
    GET_PERCENTAGE = (1 << 5)


class ONLP_FAN_DIR(Enumeration):
    UNKNOWN = 0
    B2F = 1
    F2B = 2


class ONLP_LED_CAPS(Enumeration):
    OFF = (1 << 0)
    AUTO = (1 << 1)
    AUTO_BLINKING = (1 << 2)
    CHAR = (1 << 3)
    RED = (1 << 4)
    RED_BLINKING = (1 << 5)
    ORANGE = (1 << 6)
    ORANGE_BLINKING = (1 << 7)
    YELLOW = (1 << 8)
    YELLOW_BLINKING = (1 << 9)
    GREEN = (1 << 10)
    GREEN_BLINKING = (1 << 11)
    BLUE = (1 << 12)
    BLUE_BLINKING = (1 << 13)
    PURPLE = (1 << 14)
    PURPLE_BLINKING = (1 << 15)


class ONLP_LED_MODE(Enumeration):
    OFF = 0
    AUTO = 1
    AUTO_BLINKING = 2
    CHAR = 3
    RED = 4
    RED_BLINKING = 5
    ORANGE = 6
    ORANGE_BLINKING = 7
    YELLOW = 8
    YELLOW_BLINKING = 9
    GREEN = 10
    GREEN_BLINKING = 11
    BLUE = 12
    BLUE_BLINKING = 13
    PURPLE = 14
    PURPLE_BLINKING = 15


class ONLP_LOG_FLAG(Enumeration):
    JSON = 0


class ONLP_OID_JSON_FLAG(Enumeration):
    RECURSIVE = (1 << 0)
    UNSUPPORTED_FIELDS = (1 << 1)
    TO_USER_JSON = (1 << 2)


class ONLP_OID_STATUS_FLAG(Enumeration):
    PRESENT = (1 << 0)
    FAILED = (1 << 1)
    OPERATIONAL = (1 << 2)
    UNPLUGGED = (1 << 3)


class ONLP_OID_TYPE(Enumeration):
    CHASSIS = 1
    MODULE = 2
    THERMAL = 3
    FAN = 4
    PSU = 5
    LED = 6
    SFP = 7
    GENERIC = 8


class ONLP_OID_TYPE_FLAG(Enumeration):
    CHASSIS = (1 << 1)
    MODULE = (1 << 2)
    THERMAL = (1 << 3)
    FAN = (1 << 4)
    PSU = (1 << 5)
    LED = (1 << 6)
    SFP = (1 << 7)
    GENERIC = (1 << 8)


class ONLP_PSU_CAPS(Enumeration):
    GET_TYPE = (1 << 0)
    GET_VIN = (1 << 1)
    GET_VOUT = (1 << 2)
    GET_IIN = (1 << 3)
    GET_IOUT = (1 << 4)
    GET_PIN = (1 << 5)
    GET_POUT = (1 << 6)


class ONLP_PSU_TYPE(Enumeration):
    AC = 0
    DC12 = 1
    DC48 = 2


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


class ONLP_SFP_TYPE(Enumeration):
    SFP = 0
    QSFP = 1
    SFP28 = 2
    QSFP28 = 3


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


class ONLP_THERMAL_THRESHOLD(Enumeration):
    WARNING_DEFAULT = 45000
    ERROR_DEFAULT = 55000
    SHUTDOWN_DEFAULT = 60000

# <auto.end.pyenum(ALL).define>

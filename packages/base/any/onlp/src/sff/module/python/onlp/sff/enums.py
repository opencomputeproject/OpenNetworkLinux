"""enums.py

Enumerations from the SFF auto.yml.
"""

class Enumeration(object):
    @classmethod
    def name(klass, value):
        for (k, v) in klass.__dict__.iteritems():
            if v == value:
                return k
        return None

# <auto.start.pyenum(ALL).define>
class SFF_MEDIA_TYPE(Enumeration):
    COPPER = 0
    FIBER = 1


class SFF_MODULE_CAPS(Enumeration):
    F_100 = 1
    F_1G = 2
    F_10G = 4
    F_25G = 8
    F_40G = 16
    F_100G = 32


class SFF_MODULE_TYPE(Enumeration):
    _100G_AOC = 0
    _100G_BASE_CR4 = 1
    _100G_BASE_SR4 = 2
    _100G_BASE_LR4 = 3
    _100G_CWDM4 = 4
    _100G_PSM4 = 5
    _100G_SWDM4 = 6
    _40G_BASE_CR4 = 7
    _40G_BASE_SR4 = 8
    _40G_BASE_LR4 = 9
    _40G_BASE_LM4 = 10
    _40G_BASE_ACTIVE = 11
    _40G_BASE_CR = 12
    _40G_BASE_SR2 = 13
    _40G_BASE_SM4 = 14
    _40G_BASE_ER4 = 15
    _25G_BASE_CR = 16
    _25G_BASE_SR = 17
    _25G_BASE_LR = 18
    _25G_BASE_AOC = 19
    _10G_BASE_SR = 20
    _10G_BASE_LR = 21
    _10G_BASE_LRM = 22
    _10G_BASE_ER = 23
    _10G_BASE_CR = 24
    _10G_BASE_SX = 25
    _10G_BASE_LX = 26
    _10G_BASE_ZR = 27
    _10G_BASE_SRL = 28
    _1G_BASE_SX = 29
    _1G_BASE_LX = 30
    _1G_BASE_CX = 31
    _1G_BASE_T = 32
    _100_BASE_LX = 33
    _100_BASE_FX = 34
    _4X_MUX = 35


class SFF_SFP_TYPE(Enumeration):
    SFP = 0
    QSFP = 1
    QSFP_PLUS = 2
    QSFP28 = 3
    SFP28 = 4

# <auto.end.pyenum(ALL).define>

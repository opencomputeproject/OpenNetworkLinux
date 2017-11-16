"""__init__.py

Module init for onlp/sff
"""

from onlp.sff.enums import *

import ctypes

sff_media_type = ctypes.c_int
sff_module_caps = ctypes.c_int
sff_module_type = ctypes.c_int
sff_sfp_type = ctypes.c_int

class sff_info(ctypes.Structure):
    _fields_ = [("vendor", ctypes.c_char * 17),
                ("model", ctypes.c_char * 17),
                ("serial", ctypes.c_char * 17),
                ("sfp_type", sff_sfp_type),
                ("sfp_type_name", ctypes.c_char_p),
                ("module_type", sff_module_type),
                ("module_type_name", ctypes.c_char_p),
                ("media_type", sff_media_type),
                ("media_type_name", ctypes.c_char_p),
                ("caps", sff_module_caps),
                ("length", ctypes.c_int),
                ("desc", ctypes.c_char * 16),]

class sff_eeprom(ctypes.Structure):
    _fields_ = [("eeprom", ctypes.c_ubyte * 256),
                ("cc_base", ctypes.c_ubyte),
                ("cc_ext", ctypes.c_ubyte),
                ("identified", ctypes.c_int),
                ("info", sff_info),]

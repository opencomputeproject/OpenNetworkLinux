# /////////////////////////////////////////////////////////////////////
#
#  oomtypes.py : Common type definitions used by multiple OOM modules
#
#  Copyright 2015  Finisar Inc.
#
#  Author: Don Bollinger don@thebollingers.org
#
# ////////////////////////////////////////////////////////////////////

import struct
from ctypes import Structure, c_void_p, c_int, c_ubyte


#
# This class recreates the port structure in the southbound API
#
class c_port_t(Structure):
    _fields_ = [("handle", c_void_p),
                ("oom_class", c_int),
                ("name", c_ubyte * 32)]

# Southbound API will report the 'class' of a module, basically
# whether it uses i2c addresses, pages, and bytes (SFF) or
# it uses mdio, a flat 16 bit address space, and words (CFP)
port_class_e = {
    'UNKNOWN': 0x00,
    'SFF': 0x01,
    'CFP': 0x02
    }

#!/usr/bin/python
################################################################
#
#        Copyright 2016, Big Switch Networks, Inc.
#
# Licensed under the Eclipse Public License, Version 1.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#        http://www.eclipse.org/legal/epl-v10.html
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the
# License.
#
################################################################
#
# Wrapper object class for use with the onlp library.
#
################################################################
import ctypes
import os
import struct
from ctypes import byref
from ctypes.util import find_library
import pdb

def enum(**enums):
    """ enumeration for python versions less the 3
    """
    return type('Enum', (), enums)

#enum for SFP module caps
class sff_module_caps_t(object):
    SFF_MODULE_CAPS_F_100 = 1
    SFF_MODULE_CAPS_F_1G = 2
    SFF_MODULE_CAPS_F_10G = 4
    SFF_MODULE_CAPS_F_40G = 8
    SFF_MODULE_CAPS_F_100G = 16

#enum SFP media type
sff_media_type_t = enum(
    SFF_MEDIA_TYPE_COPPER = 'COPPER',
    SFF_MEDIA_TYPE_FIBER = 'FIBER',
    SFF_MEDIA_TYPE_LAST = 'FIBER',
    SFF_MEDIA_TYPE_COUNT = 0,
    SFF_MEDIA_TYPE_INVALID = -1)

#enum SFP module types
sff_module_type_t = enum(
    SFF_MODULE_TYPE_100G_AOC = '100G_AOC',
    SFF_MODULE_TYPE_100G_BASE_CR4 = '100G_BASE_CR4',
    SFF_MODULE_TYPE_100G_BASE_SR4 = '100G_BASE_SR4',
    SFF_MODULE_TYPE_100G_BASE_LR4 = '100G_BASE_LR4',
    SFF_MODULE_TYPE_40G_BASE_CR4 = '40G_BASE_CR4',
    SFF_MODULE_TYPE_40G_BASE_SR4 = '40G_BASE_SR4',
    SFF_MODULE_TYPE_40G_BASE_LR4 = '40G_BASE_LR4',
    SFF_MODULE_TYPE_40G_BASE_ACTIVE = '40G_BASE_ACTIVE',
    SFF_MODULE_TYPE_40G_BASE_CR = '40G_BASE_CR',
    SFF_MODULE_TYPE_40G_BASE_SR2 = '40G_BASE_SR2',
    SFF_MODULE_TYPE_10G_BASE_SR = '10G_BASE_SR',
    SFF_MODULE_TYPE_10G_BASE_LR = '10G_BASE_LR',
    SFF_MODULE_TYPE_10G_BASE_LRM = '10G_BASE_LRM',
    SFF_MODULE_TYPE_10G_BASE_ER = '10G_BASE_ER',
    SFF_MODULE_TYPE_10G_BASE_CR = '10G_BASE_CR',
    SFF_MODULE_TYPE_10G_BASE_SX = '10G_BASE_SX',
    SFF_MODULE_TYPE_10G_BASE_LX = '10G_BASE_LX',
    SFF_MODULE_TYPE_10G_BASE_ZR = '10G_BASE_ZR',
    SFF_MODULE_TYPE_10G_BASE_SRL = '10G_BASE_SRL',
    SFF_MODULE_TYPE_1G_BASE_SX = '1G_BASE_SX',
    SFF_MODULE_TYPE_1G_BASE_LX = '1G_BASE_LX',
    SFF_MODULE_TYPE_1G_BASE_CX = '1G_BASE_CX',
    SFF_MODULE_TYPE_1G_BASE_T = '1G_BASE_T',
    SFF_MODULE_TYPE_100_BASE_LX = '100_BASE_LX',
    SFF_MODULE_TYPE_100_BASE_FX = '100_BASE_FX',
    SFF_MODULE_TYPE_LAST = '100_BASE_FX',
    SFF_MODULE_TYPE_COUNT = 0,
    SFF_MODULE_TYPE_INVALID = -1)

#enum SFP types
sff_sfp_type_t = enum(
    SFF_SFP_TYPE_SFP = 'SFP',
    SFF_SFP_TYPE_QSFP = 'QSFP',
    SFF_SFP_TYPE_QSFP_PLUS = 'QSFP_PLUS',
    SFF_SFP_TYPE_QSFP28 = 'QSFP28',
    SFF_SFP_TYPE_LAST = 'QSFP28',
    SFF_SFP_TYPE_COUNT = 0,
    SFF_SFP_TYPE_INVALID = -1)

# onlp_sfp_control_flag
onlp_sfp_control_flag = enum(
    ONLP_SFP_CONTROL_FLAG_RESET = (1 << 0),
    ONLP_SFP_CONTROL_FLAG_RESET_STATE = ( 1 << 1),
    ONLP_SFP_CONTROL_FLAG_RX_LOS = ( 1 << 2 ),
    ONLP_SFP_CONTROL_FLAG_TX_FAULT = ( 1 << 3 ),
    ONLP_SFP_CONTROL_FLAG_TX_DISABLE = ( 1 << 4 ),
    ONLP_SFP_CONTROL_FLAG_LP_MODE = ( 1 << 5 ),
    ONLP_SFP_CONTROL_FLAG_POWER_OVERRIDE = ( 1 << 6 ))



class sff_info(ctypes.Structure):
    _fields_ = [('eeprom',ctypes.c_ubyte*256),
                ('vendor',ctypes.c_char*17),
                ('model',ctypes.c_char*17),
                ('serial',ctypes.c_char*17),
                ('sfp_type',ctypes.c_int),
                ('sfp_type_name',ctypes.c_char_p),
                ('module_type',ctypes.c_int),
                ('module_type_name',ctypes.c_char_p),
                ('media_type',ctypes.c_int),
                ('media_type_name',ctypes.c_char_p),
                ('sff_module_caps_t',ctypes.c_int),
                ('length',ctypes.c_int),
                ('length_desc',ctypes.c_char*16),
                ('cc_base',ctypes.c_ubyte),
                ('cc_ext',ctypes.c_ubyte),
                ('supported',ctypes.c_int)]


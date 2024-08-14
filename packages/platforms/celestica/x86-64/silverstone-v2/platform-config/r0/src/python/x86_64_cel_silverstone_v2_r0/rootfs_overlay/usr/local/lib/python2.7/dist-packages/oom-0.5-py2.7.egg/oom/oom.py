# /////////////////////////////////////////////////////////////////////
#
#  oom.py : Provides the Northbound API, hides the implementation in
#  oomlib.py decode.py and the module type definitions
#
#  Copyright 2015  Finisar Inc.
#
#  Author: Don Bollinger don@thebollingers.org
#
# ////////////////////////////////////////////////////////////////////

from . import oomlib
from .oomlib import print_block_hex
from .decode import get_string

"""
The members of the port class are:
    c_port: the C port structure returned by the Southbound API
        (see oom_south.h)
    port_name: The name of the port provided by the Southbound API
    port_type: The type of the port, per the SFF specs.  For example:
        SFP is type 3, QSFP+ is type 13
    mmap: The dictionary of keys, decoders and locations for everything
        OOM knows how to access in this port.  See qsfp_plus.py for
        the list of QSFP+ keys, for example.
    fmap: The list of keys that form function groups (for oom_get_memory())
    wmap: The list of writable keys, and the encoder to pack the
        data to write for each key

    port class manages a page cache, saving the data for each page read
    (see oom_get_cached_sff(), oom_get_keyvalue_cached()
    As part of cache management, port class exposes the function:
        invalidate_page(address, page)
    As usual, address is the i2c address (eg 0xA0).
    Page is the page to invalidate.  IMPORTANT: Low memory (offset < 128)
    is not actually in a 'page'.  That memory is cached as page -1.
    Thus 'port.invalidate_page(0xA0, -1)' clears the cache for
    bytes 0-127 on 0xA0 (which is in fact where the dynamic data
    for a QSFP+ module resides)
"""


#
# helper routine, provides a port without requiring the user to
# define the complicated port_t struct
#
def oom_get_port(n):
    port = oomlib.oom_get_port(n)
    return (port)


#
# get the full list of ports, allocates the
# memory, defines the data types
#
def oom_get_portlist():
    port_list = oomlib.oom_get_portlist()
    return(port_list)


#
# magic decoder - gets any attribute based on its key
# if there is no decoder for the port type, or the key is not
# listed in the memory map for that port type, then returns None
# NOTE: the type of the value returned depends on the key.
# Use 'str(oom_get_keyvalue(port, key))' to get a readable string
# for all return types
#
def oom_get_keyvalue(port, key):
    return oomlib.oom_get_keyvalue(port, key)


def oom_get_keyvalue_cached(port, key):
    return oomlib.oom_get_keyvalue_cached(port, key)


#
#
# Set a key to chosen value (write value to EEPROM)
# Be careful with this, this is likely to change the function
# of your module
#
def oom_set_keyvalue(port, key, value):
    return oomlib.oom_set_keyvalue(port, key, value)


#
# given a 'function', return a dictionary with the values of all the
# keys in that function, on the specified port
#
def oom_get_memory(port, function):
    return oomlib.oom_get_memory(port, function)


#
# fetch raw data from sff type memory.
#   port: an OOM port from oom_get_portlist()
#   address: i2c address (eg 0xA0, 0xA2)
#   page: page of the data.  NOT USED if offset < 128
#   offset: byte address within the page
#   length: number of bytes to read
#
def oom_get_memory_sff(port, address, page, offset, length):
    return oomlib.oom_get_memory_sff(port, address, page, offset, length)


#
# same as oom_get_memory_sff except uses a page cache.
# each page, of each address, of each port is potentially cached
# OOM manages filling the pages if needed, and reads from
# the cache if available.  Note that each key is tagged with
# whether it is dynamic (first field is 1) or static (first field is 0)
# to invalidate a cached page, use 'port.invalidate_page(address, page)'
# (note that offset 0-127 is cached as page -1)
#
def oom_get_cached_sff(port, address, page, offset, length):
    return oomlib.oom_get_cached_sff(port, address, page, offset, length)


#
# write raw memory to EEPROM
# parameters are the same as oom_get_memory_sff
# with the addition of 'data', which is a byte array of the data to be written
# oom_set_memory_sff invalidates the page cache for the page written to
def oom_set_memory_sff(port, address, page, offset, length, data):
    return oomlib.oom_set_memory_sff(port, address, page, offset, length, data)


#
# fetch raw data from CFP type memory.
#   port: an OOM port from oom_get_portlist()
#   address: WORD address in the EEPROM
#   length: number of WORDS to read
#
def oom_get_memory_cfp(port, address, length):
    return oomlib.oom_get_memory_cfp(port, address, length)


#
# same as oom_get_memory_cfp except uses a page cache.
#   (OOM does not cache CFP at this time)
#
def oom_get_cached_cfp(port, address, length):
    return oomlib.oom_get_cached_cfp(port, address, length)


#
# write raw memory to CFP EEPROM
# parameters are the same as oom_get_memory_cfp
# with the addition of 'data', which is a byte array of the data to be written
def oom_set_memory_cfp(port, address, length, data):
    return oomlib.oom_set_memory_cfp(port, address, length, data)

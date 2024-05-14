# /////////////////////////////////////////////////////////////////////
#
#  keytest.py : Automatically detects all available keys (on port 0),
#  decodes the value for each key.  For raw byte keys, reports their
#  hex values.  Finally, detects avaliable 'functions' (bundles of keys)
#  and reports their values.
#
#  Copyright 2015  Finisar Inc.
#
#  Author: Don Bollinger don@thebollingers.or/
#
# ////////////////////////////////////////////////////////////////////

from oom import *                   # the published OOM Northbound API
from oom.decode import get_hexstr   # helper function from the decode pack
from oom.oomlib import print_block_hex
import sys

# open port
try:
    port = oom_get_port(int(sys.argv[1]))
except Exception:
    print("Usage: python3 keytest.py <port-index>")
    print("       <port-index> starts from 0")
    sys.exit(0)

print('0xA0, page 0, offset 0, 128 bytes')
print_block_hex(oom_get_memory_sff(port, 0xA0, 0, 0, 128), 0)
print("")
print('0xA0, page 0, offset 128, 128 bytes')
print_block_hex(oom_get_memory_sff(port, 0xA0, 0, 128, 128), 128)

# get the internal list of keys and decoders for this type of module
# report their values for this port
keymap = port.mmap
print(str(len(keymap.keys())) + ' keys implemented')
for keyx in sorted(keymap.keys()):
    if len(keymap[keyx]) >= 6:
        if keymap[keyx][1] == 'get_bytes':
            val = oom_get_keyvalue(port, keyx)
            print(keyx + ': ' + get_hexstr(val))
        else:
            print(keyx + ': ' + str(oom_get_keyvalue(port, keyx)))

# similarly, get the function keys for this module type,
# report their values for this port
print(' ')
print('functions, with their keys and values:')
print(' ')
fnkeys = port.fmap
for keyx in fnkeys:
    val = oom_get_memory(port, keyx)
    print(keyx + ': ')
    print(str(val))
    print("")

# get the writable keys, for each, read the current value, write it back
print(' ')
print('writable keys, with before and after values (should match)')
wmapkeys = port.wmap
for keyx in sorted(wmapkeys):
    val = oom_get_keyvalue(port, keyx)
    retval = oom_set_keyvalue(port, keyx, val)
    newval = oom_get_keyvalue(port, keyx)
    print(keyx + ': ' + str(val) + ',' + str(newval))

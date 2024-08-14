#! /usr/bin/python

from oom import *
from oom.oomlib import type_to_str
import sys


# This version of keys works with caching
def add_QSFP_keys(port):

    new_mm_keys = {'Q_WRITE_0':    (1, 'get_int', 0xA0, 2, 128, 1),
                   'Q_WRITE_127':    (1, 'get_int', 0xA0, 2, 255, 1),
                   }

    new_wm_keys = {'Q_WRITE_0':   'set_int',
                   'Q_WRITE_127': 'set_int',
                   }

    port.mmap.update(new_mm_keys)
    port.wmap.update(new_wm_keys)


# get the requested port number
print("")
parms = sys.argv
if (len(parms) != 2):
    print('Usage: python portinfo.py <port>')
    print('   <port> Can be a name or a number (position in portlist array)')
    exit()
portid = parms[1]
portlist = oom_get_portlist()
port = 0
for p in portlist:
    if p.port_name == portid:
        port = p
if port == 0:
    try:
        portnum = int(portid)
        if ((portnum >= 0) and (portnum < len(portlist))):
            port = portlist[portnum]
    except:
        port = 0
if (port == 0):
    print('Bad port identifier: ' + portid)
    exit()

# pick where to write based on SFP or QSFP
if port.port_type == 3:
    print('SFP')
    print("")

    # initial state
    print('initial memory content (hex), A2, byte 110:')
    print('bit 7 is TX_DISABLE_STATE, bit 6 is SOFT_TX_DISABLE_SELECT')
    print('We will write bit 6, expect bit 7 may flip as well')
    print_block_hex(oom_get_memory_sff(port, 0xA2, 0, 110, 1), 110)
    init_val = oom_get_keyvalue(port, 'SOFT_TX_DISABLE_SELECT')
    print('initial value of SOFT_TX_DISABLE_SELECT: %d' % init_val)
    print("")

    # set to '1'
    retval0 = oom_set_keyvalue(port, 'SOFT_TX_DISABLE_SELECT', 1)
    print('attempted to change SOFT_TX_DISABLE_SELECT to 1 (bit 6)')
    print('changed memory content (hex), A2, byte 110:')
    print_block_hex(oom_get_memory_sff(port, 0xA2, 0, 110, 1), 110)
    new_val = oom_get_keyvalue(port, 'SOFT_TX_DISABLE_SELECT')
    print('new value of SOFT_TX_DISABLE_SELECT: %d' % (new_val))
    print("")

    # set to 0
    retval1 = oom_set_keyvalue(port, 'SOFT_TX_DISABLE_SELECT', 0)
    print('attempted to change SOFT_TX_DISABLE_SELECT to 0')
    print('changed memory content (hex), A2, byte 110:')
    print_block_hex(oom_get_memory_sff(port, 0xA2, 0, 110, 1), 110)
    new_val = oom_get_keyvalue(port, 'SOFT_TX_DISABLE_SELECT')
    print('updated value of SOFT_TX_DISABLE_SELECT: %d' % (new_val))
    print("")

    # restore initial value
    retval2 = oom_set_keyvalue(port, 'SOFT_TX_DISABLE_SELECT', init_val)
    print('attempted to restore SOFT_TX_DISABLE_SELECT')
    print('restored memory content (hex), A2, byte 110:')
    print_block_hex(oom_get_memory_sff(port, 0xA2, 0, 110, 1), 110)
    new_val = oom_get_keyvalue(port, 'SOFT_TX_DISABLE_SELECT')
    print('restored value of SOFT_TX_DISABLE_SELECT: %d' % (init_val))

if port.port_type == 13:
    print('QSFP')
    add_QSFP_keys(port)

    print('initial memory content (hex), A0, page 2, bytes 0-127:')
    print_block_hex(oom_get_memory_sff(port, 0xA0, 2, 128, 128), 128)
    init_val0 = oom_get_keyvalue(port, 'Q_WRITE_0')
    init_val127 = oom_get_keyvalue(port, 'Q_WRITE_127')
    print('initial value of byte 0: %x, byte 127: %x' %
          (init_val0, init_val127))

    retval0 = oom_set_keyvalue(port, 'Q_WRITE_0', 0xAA)
    retval127 = oom_set_keyvalue(port, 'Q_WRITE_127', 0xEF)

    print('attempted to change byte 0 to 0xAA, byte 127 to 0xEF')
    print('changed memory content (hex), A0, page 2, bytes 0-127:')
    print_block_hex(oom_get_memory_sff(port, 0xA0, 2, 128, 128), 128)
    new_val0 = oom_get_keyvalue(port, 'Q_WRITE_0')
    new_val127 = oom_get_keyvalue(port, 'Q_WRITE_127')
    print('new value of byte 0: %x, byte 127: %x' % (new_val0, new_val127))

    retval0 = oom_set_keyvalue(port, 'Q_WRITE_0', init_val0)
    retval127 = oom_set_keyvalue(port, 'Q_WRITE_127', init_val127)

    print('attempted to restore byte 0 and byte 127')
    print('restored memory content (hex), A0, page 2, bytes 0-127:')
    print_block_hex(oom_get_memory_sff(port, 0xA0, 2, 128, 128), 128)
    res_val0 = oom_get_keyvalue(port, 'Q_WRITE_0')
    res_val127 = oom_get_keyvalue(port, 'Q_WRITE_127')
    print('restored value of byte 0: %x, byte 127: %x' %
          (res_val0, res_val127))

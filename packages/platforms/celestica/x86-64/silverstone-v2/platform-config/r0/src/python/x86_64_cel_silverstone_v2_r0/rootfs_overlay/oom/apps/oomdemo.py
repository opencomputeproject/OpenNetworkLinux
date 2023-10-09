# /////////////////////////////////////////////////////////////////////
#
#  oomdemo.py : Exercises OOM Northbound API
#
#  Copyright 2015  Finisar Inc.
#
#  Author: Don Bollinger don@thebollingers.org
#
# ////////////////////////////////////////////////////////////////////

from oom import *
from ctypes import *

# get the list of available ports (port_list is a list of port structures)
port_list = oom_get_portlist()

# Demo the raw memory access API
print('*******************')
print('oom_get_memory_sff demo:')
print('Port 0, address A2h, page 0, offset 0, 128 bytes:')
print('Port Name: ' + port_list[0].port_name)
print("")
print_block_hex(oom_get_memory_sff(port_list[0], 0xA2, 0, 0, 128), 0)

# demo the get_keyvalue() API
print("")
print('*******************')
print('oom_get_keyvalue demo:')
print("VENDOR_SN: " + oom_get_keyvalue(port_list[0], "VENDOR_SN"))
print("XYZ: " + oom_get_keyvalue(port_list[0], "XYZ"))
print("IDENTIFIER: " + str(oom_get_keyvalue(port_list[0], "IDENTIFIER")))

# demo the oom_get_memory() API...  DOM shows the values for 5 keys
print("")
print('*******************')
print('oom_get_memory demo:')
print("DOM: " + str(oom_get_memory(port_list[0], "DOM")))

# demo the oom_getport() API, by getting port 2.  Note that the keys
# have different values on this different port
print("")
print('*******************')
print('oom_get_port demo:')
portnum = 2
print("Port " + str(portnum))
port = oom_get_port(portnum)
print("VENDOR_SN: " + str(oom_get_keyvalue(port, "VENDOR_SN")))
print("DOM: " + str(oom_get_memory(port, "DOM")))

# demo the raw write API
# SFP address A2h, page 0, offset 128-247 are user writable, scribble there
print('*******************')
print('oom_set_memory_sff demo')
print('0xA2, page 0, offset 128, 16 bytes, initial content:')
content = oom_get_memory_sff(port, 0xA2, 0, 128, 16)
print_block_hex(content, 128)
content = bytearray('16 changed bytes', 'utf-8')
length = oom_set_memory_sff(port, 0xA2, 0, 128, 16, content)
print('0xA2, page 0, offset 128, 16 bytes, new content: \'16 changed bytes\':')
content = oom_get_memory_sff(port, 0xA2, 0, 128, 16)
print(get_string(content))
print('*******************')

# demo the key write API
# SOFT_TX_DISABLE_SELECT is a user writable control bit to disable TX
print('oom_set_memory(port, key, value) demo')
print('Current value of TX_DISABLE: ' +
      str(oom_get_keyvalue(port, 'SOFT_TX_DISABLE_SELECT')))
status = oom_set_keyvalue(port, 'SOFT_TX_DISABLE_SELECT', 1)
print('New value of TX_DISABLE: ' +
      str(oom_get_keyvalue(port, 'SOFT_TX_DISABLE_SELECT')))
status = oom_set_keyvalue(port, 'SOFT_TX_DISABLE_SELECT', 0)
print('Newer value of TX_DISABLE: ' +
      str(oom_get_keyvalue(port, 'SOFT_TX_DISABLE_SELECT')))
print('*******************')

# demo QSFP+
print('QSFP+ demo')
port = oom_get_port(5)   # in the southbound shim, 5 is a QSFP port
print('port 5, page 1 (QSFP, 0xA0, page 0, offset 128, 128 bytes)')
print_block_hex(oom_get_memory_sff(port, 0xA0, 0, 128, 128), 128)
print('*******************')
print('Serial ID keys (all from page 0)')
print("SERIAL_ID: " + str(oom_get_memory(port, "SERIAL_ID")))

# demo QSFP key write
# TXn_DISABLE disables Tx on channel n
print('QSFP oom_set_memory(port, key, value) demo')
print('Current value of TX_DISABLE (one bit per ch, ch4 is high bit): ' +
      hex(oom_get_keyvalue(port, 'TX_DISABLE')))
status = oom_set_keyvalue(port, 'TX4_DISABLE', 1)
status = oom_set_keyvalue(port, 'TX2_DISABLE', 1)
print('New value of TX_DISABLE, disabled ch4, ch2: ' +
      hex(oom_get_keyvalue(port, 'TX_DISABLE')))
status = oom_set_keyvalue(port, 'TX4_DISABLE', 0)
status = oom_set_keyvalue(port, 'TX3_DISABLE', 1)
status = oom_set_keyvalue(port, 'TX2_DISABLE', 0)
status = oom_set_keyvalue(port, 'TX1_DISABLE', 1)
print('Newer value of TX_DISABLE, swapped all 4 channels: ' +
      hex(oom_get_keyvalue(port, 'TX_DISABLE')))
status = oom_set_keyvalue(port, 'TX3_DISABLE', 0)
status = oom_set_keyvalue(port, 'TX1_DISABLE', 0)
print('Re-enable all 4 ports: ' +
      hex(oom_get_keyvalue(port, 'TX_DISABLE')))
print('*******************')

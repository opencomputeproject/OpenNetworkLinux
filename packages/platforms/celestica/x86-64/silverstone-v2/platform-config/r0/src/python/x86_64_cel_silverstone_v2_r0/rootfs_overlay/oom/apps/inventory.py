#! /usr/bin/python
#
# inventory.py
#
# OOM script to inventory the modules in a switch
#
import sys
from oom import *
from oom.oomlib import type_to_str

# load the json shim if a URL has been provided
parms = sys.argv
if len(parms) > 1:
    if parms[1] == '-url':
        oomlib.setshim("oomjsonshim", parms[2])

formatstr = '%-10s %-16s %-13s %-16s %-16s'
print("")
print(formatstr % ('Port Name', 'Vendor', 'Type', 'Part #', 'Serial #'))
print("")
for port in oom_get_portlist():
    modtype = type_to_str(port.port_type)
    if modtype == 'UNKNOWN':
        modtype = 'No Module'
    print(formatstr % (port.port_name,
          oom_get_keyvalue(port, 'VENDOR_NAME'),
          modtype,
          oom_get_keyvalue(port, 'VENDOR_PN'),
          oom_get_keyvalue(port, 'VENDOR_SN')))
print("")

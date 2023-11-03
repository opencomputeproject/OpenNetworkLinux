# /////////////////////////////////////////////////////////////////////
#
#  ocpdemo.py :
#  In an endless loop, repeating every 5(?) seconds...
#      For each port,
#          Show fixed identifying keys, then scrolling other keys
#
#  Just to show that we can, and give an idea of how much data is available
#
#  Copyright 2015  Finisar Inc.
#
#  Author: Don Bollinger don@thebollingers.or/
#
# ////////////////////////////////////////////////////////////////////

from oom import *                   # the published OOM Northbound API
from oom.oomlib import type_to_str
from oom.decode import get_hexstr   # helper function from the decode pack
from time import sleep
import sys

"""
  tweak numlines and linelen to tune demo visual
  numlines should be the number of lines in the display window,
  or the number of modules in the switch,  whichever is smaller
  linelen should be the number of chars wide the window is, wider is better!
  these can be passed as parameters on the command line:
  eg: py ocpdemo.py 5 80   < 5 lines, 80 characters per line
"""
numlines = 3
linelen = 80
parms = sys.argv
if (len(parms) > 1):
    numlines = int(parms[1])
if (len(parms) > 2):
    linelen = int(parms[2])


portlist = oom_get_portlist()
numports = len(portlist)
print(numports)
pcycle = 0
pcount = 0
try:
    while 1:
        lines = 0
        while lines < numlines:
            pnum = (pcount) % numports
            pcount += 1
            port = portlist[pnum]
            outstr = str.format("{:6} {:10}",
                                port.port_name, type_to_str(port.port_type))
            keylist = port.fmap
            if keylist == {}:
                continue
            keylist = port.fmap["SERIAL_ID"]
            keyoff = 0
            while len(outstr) < linelen:
                temp = (pcycle + keyoff) % len(keylist)
                keyoff += 1
                key = keylist[temp % len(keylist)]
                if len(port.mmap[key]) >= 6:
                    if port.mmap[key][1] == 'get_bytes':
                        val = oom_get_keyvalue(port, key)
                        outstr += key + ': ' + get_hexstr(val) + '; '
                    else:
                        outstr += key + ': ' \
                                + str(oom_get_keyvalue(port, key)) + "; "
                else:
                    outstr += '                   '
            print(outstr[0:linelen])
            lines += 1
        pcycle += 1  # it will take a LONG time to roll over
        sleep(2)
except KeyboardInterrupt:
    print("Thanks for running the OOM OCP Demo!")

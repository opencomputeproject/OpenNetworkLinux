# /////////////////////////////////////////////////////////////////////
#
# opticsdump.py : Test script to dump optics module data in human readable or JSON formats.
#
# Based on https://github.com/opencomputeproject/oom/blob/master/apps/inventory.py .
#
# "calculated" dBm values derived based on https://www.rapidtables.com/convert/power/mW_to_dBm.html#how
# and on the "SFF-8636 Specification for Management Interface for Cabled Environments Rev 2.9 April 21st, 2017"
# section 6.2.5 (page 39).
#
#  Author: Bhishma Acharya bhishma@rtbrick.com/bhishma.acharya@gmail.com
#
# ////////////////////////////////////////////////////////////////////


import argparse
import json
import math
import sys

from oom import *
from oom.oomlib import type_to_str

ag_parser = argparse.ArgumentParser()
ag_parser.add_argument("-p", "--port-name", help="The port name in 'port12' format. The port number is different than both i2cdump and onlpdump -S port numbers.")
ag_parser.add_argument("-j", "--json",  help="Generate output in JSON format. Currently only honored if dumping a single port.", action="store_true")
args = ag_parser.parse_args()

if args.port_name is None:
    formatstr = '%-10s %-16s %-13s %-16s %-16s'
    print(formatstr % ('Port Name', 'Vendor', 'Type', 'Part #', 'Serial #'))
    for port in oom_get_portlist():
        modtype = type_to_str(port.port_type)
        if modtype == 'UNKNOWN':
            modtype = 'No Module'
        print(formatstr % (port.port_name,
                        oom_get_keyvalue(port, 'VENDOR_NAME'),
                        modtype,
                        oom_get_keyvalue(port, 'VENDOR_PN'),
                        oom_get_keyvalue(port, 'VENDOR_SN')))
else:
    port_found = False
    port_data = {}
    if args.json:
        for port in oom_get_portlist():
            if args.port_name == port.port_name:
                for k in sorted(port.fmap.keys()):
                    port_data[k] = {}
                    for l in sorted(port.fmap[k]):
                        v = oom_get_keyvalue(port, l)
                        if l.endswith('POWER'):
                            port_data[k][l + '_CALCULATED_DBM'] = 10 * math.log10(v if v>0 else 1)
                        if (isinstance(v, bytes)):
                            v = v.decode('ISO-8859-1')
                        port_data[k][l] = v
                for k in sorted(port.mmap.keys()):
                    v = oom_get_keyvalue(port, k)
                    if l.endswith('POWER'):
                        port_data[k + '_CALCULATED_DBM'] = 10 * math.log10(v if v>0 else 1)
                    if (isinstance(v, bytes)):
                        v = v.decode('ISO-8859-1')
                    port_data[k] = v
                port_found = True
                break
        if not port_found:
            port_data["error"] = "port %s doesn't exist" % args.port_name
            print(json.dump(port_data, sort_keys=True, indent=4, ensure_ascii=False))
            sys.exit(1)
        print(json.dumps(port_data, sort_keys=True, indent=4, ensure_ascii=False))
    else:
        for port in oom_get_portlist():
            if args.port_name == port.port_name:
                for k in sorted(port.fmap.keys()):
                    print(k)
                    for l in sorted(port.fmap[k]):
                        v = oom_get_keyvalue(port, l)
                        if (isinstance(v, bytes)):
                            v = v.decode('ISO-8859-1')
                        if l.endswith('POWER'):
                            print('\t%-20s= %-10s(calculated %s dBm)' % (l, v, 10 * math.log10(v if v>0 else 1)))
                        else:
                            print('\t%-20s= %s' % (l, v))
                for k in sorted(port.mmap.keys()):
                    v = oom_get_keyvalue(port, k)
                    if (isinstance(v, bytes)):
                        v = v.decode('ISO-8859-1')
                    if k.endswith('POWER'):
                        print('%-26s= %-10s(calculated %s dBm)' % (k, v, 10 * math.log10(v if v>0 else 1)))
                    else:
                        print('%-26s= %s' % (k, v))
                port_found = True
                break
        if not port_found:
            print("Port %s doesn't exist !" % args.port_name)
            sys.exit(1)

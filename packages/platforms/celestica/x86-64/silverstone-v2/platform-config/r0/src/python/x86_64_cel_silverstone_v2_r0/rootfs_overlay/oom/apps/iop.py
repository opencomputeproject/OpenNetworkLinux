"""

iop.py

Display Interop Testing info for the module in 'port'

python iop.py [<port number> | -all] [-f]

The first parameter can be either a port number, or 'all' for all ports
Default is port number 0
The next parameter (which could be the first) is -f if results
should go to a file.  The file's name includes the vendor's name
and other module data.
If -f is not specified, print to stdout
"""

from oom import *
from oom.decode import get_hexstr, mod_id
from datetime import datetime
import sys


def iop(port, fileflag):

    # if the port is null, just bail
    if port.port_type == 0:
        return

    # build the name of the file to store results to
    vendor_name = oom_get_keyvalue(port, 'VENDOR_NAME').replace(" ", "_")[0:8]
    outfilename = vendor_name + '_'
    vendor_sn = oom_get_keyvalue(port, 'VENDOR_SN').replace(" ", "_")
    outfilename += vendor_sn + '_EEPROMdecode_'
    dt = datetime.now()
    dateformat = "%Y%m%d%H%M%S"
    timestr = dt.strftime(dateformat)
    outfilename += timestr + '.txt'

    # see if the output should go to the screen or a file
    if fileflag == '-f':
        sys.stdout = open(outfilename, 'w+')

    # identify the module
    print("")
    print('Port: %s' % port.port_name)
    print('%s %s module' %
          (oom_get_keyvalue(port, 'VENDOR_NAME'),
           mod_id(port.port_type)))
    print('Part Number: %s  Serial Number: %s' %
          (oom_get_keyvalue(port, 'VENDOR_PN'),
           oom_get_keyvalue(port, 'VENDOR_SN')))
    print(outfilename)

    # print out the Serial ID keys
    print("")
    keys = port.fmap['SERIAL_ID']
    print('SERIAL_ID Keys:')
    for key in sorted(keys):
        val = oom_get_keyvalue(port, key)
        decoder = port.mmap[key][1]
        if ((decoder == 'get_bytes') or (decoder == 'get_cablespec')):
            valstr = get_hexstr(val)
        else:
            valstr = str(val)
        print('%s: %s' % (key, valstr))

    # print out the Vendor Specific data after the Serial ID data
    print("")
    vend_specific = ''
    if port.port_type == 0x3 or (port.port_type == 0xB):   # SFP
        vend_specific = \
            get_hexstr(oom_get_keyvalue(port, 'VENDOR_SPECIFIC_96'))
    if (port.port_type == 0xD) or (port.port_type == 0x11):  # QSFP+/QSFP28
        vend_specific = \
            get_hexstr(oom_get_keyvalue(port, 'VENDOR_SPECIFIC_224'))
    print('Vendor Specific: ' + vend_specific)

    # dump the raw data from the two most popular blocks, by type
    print("")
    if port.port_type == 0x3 or (port.port_type == 0xB):  # SFP
        print('I2C Address A0h, bytes 0-127, in hex')
        print_block_hex(oom_get_memory_sff(port, 0xA0, 0, 0, 128), 0)
        print("")
        print('I2C Address A2h, bytes 0-127, in hex')
        print_block_hex(oom_get_memory_sff(port, 0xA2, 0, 0, 128), 0)

    if (port.port_type == 0xD) or (port.port_type == 0x11):  # QSFP+/QSFP28
        print('I2C Address A0h, bytes 0-127, in hex')
        print_block_hex(oom_get_memory_sff(port, 0xA0, 0, 0, 128), 0)
        print("")
        print('I2C Address A0h, page 0, bytes 128-255, in hex')
        print_block_hex(oom_get_memory_sff(port, 0xA0, 0, 128, 128), 0)

    # print out all the keys
    keys = port.mmap
    print('All Keys:')
    print("")
    for key in sorted(keys):
        val = oom_get_keyvalue(port, key)
        decoder = port.mmap[key][1]
        if ((decoder == 'get_bytes') or (decoder == 'get_cablespec')):
            valstr = get_hexstr(val)
        else:
            valstr = str(val)
        print('%s: %s' % (key, valstr))


if __name__ == "__main__":
    parms = sys.argv
    # defaults: port 0, stdout, don't loop through ports
    portnum = 0
    fileflag = ''
    loop = 0
    if (len(parms) > 1):
        try:  # check for an integer port number
            portnum = int(parms[1])
            if (len(parms)) > 2:
                fileflag = parms[2]
        except:  # otherwise maybe '-all', maybe not
            if parms[1] == "-all":
                loop = 1
                if (len(parms) > 2):
                    fileflag = parms[2]
            else:
                loop = 0
                fileflag = parms[1]

    if portnum < 0:
        print("Bad (negative) port number")
        exit()

    portlist = oom_get_portlist()
    if len(portlist) == 0:
        print("Empty portlist, configuration issue?")
        exit()

    if loop == 0:
        iop(portlist[portnum], fileflag)
    else:
        for port in portlist:
            iop(port, fileflag)

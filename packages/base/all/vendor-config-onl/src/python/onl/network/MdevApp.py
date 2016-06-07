"""MdevApp.py

busybox mdev handler for network devices

"""

import os, sys
import onl.platform.current
import subprocess

import logging
logger = None

def do_add(device):

    platform = onl.platform.current.OnlPlatform()
    d = platform.platform_config
    d = d.get('network', {})
    d = d.get('interfaces', {})

    syspath = None
    src = "/sys/class/net/%s/device" % device
    dst = os.path.realpath(src)
    if dst.startswith("/sys/devices/"):
        syspath = dst[13:]

    tgtname = None
    for intf, idata in d.items():

        n = idata.get('name', None)
        if n is not None and n == device:
            sys.stdout.write("found interface name alias %s --> %s\n"
                             % (device, intf,))
            tgtname = intf
            break

        p = idata.get('syspath', None)
        if p is not None and p == syspath:
            sys.stdout.write("found interface sysfs alias %s --> %s\n"
                             % (syspath, intf,))
            tgtname = intf
            break

    if tgtname is not None:
        src = "/sys/class/net/%s/device" % tgtname
        if not os.path.exists(src):
            sys.stdout.write("remapping interface %s --> %s\n"
                             % (device, tgtname,))
            cmd = ('ip', 'link', 'set', device, 'name', tgtname,)
            subprocess.check_call(cmd)

    return 0

def main():

    args = list(sys.argv)
    args.pop(0)
    device = args.pop(0)
    action = args.pop(0)

    if action == 'add':
        sys.exit(do_add(device))

    sys.stderr.write("*** invalid action %s\n" % action)
    sys.exit(1)

if __name__ == "__main__":
    main()

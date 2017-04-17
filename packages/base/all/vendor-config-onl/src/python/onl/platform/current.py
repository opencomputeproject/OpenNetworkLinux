#!/usr/bin/python
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################
#
# This file provides the container for the
# platform-specific class provided by the files in the
# platform-config packages.
#
############################################################
import os
import importlib

def platform_name_get():
    # Determine the current platform name.
    platform = None
    if os.path.exists("/etc/onl/platform"):
        with open("/etc/onl/platform", 'r') as f:
            platform=f.read().strip()
    elif os.path.exists("/etc/machine.conf"):
        with open("/etc/machine.conf", 'r') as f:
            lines = f.readlines(False)
            lines = [x for x in lines if x.startswith('onie_platform=')]
            if lines:
                platform = lines[0].partition('=')[2].strip()
    if platform is None:
        raise RuntimeError("cannot find a platform declaration")

    return platform

def import_subsystem_platform_class(subsystem='onl', klass='OnlPlatform'):
    platform = platform_name_get()
    platform_module = platform.replace('-', '_').replace('.', '_')
    m = importlib.import_module('%s.platform.%s' % (subsystem, platform_module))
    return getattr(m, '%s_%s' % (klass, platform_module))


OnlPlatformName = platform_name_get()
OnlPlatform = import_subsystem_platform_class()

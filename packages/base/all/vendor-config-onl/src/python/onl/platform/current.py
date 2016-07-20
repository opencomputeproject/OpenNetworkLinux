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

def import_subsystem_platform_class(subsystem='onl', klass='OnlPlatform'):
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

    platform_module = platform.replace('-', '_')

    imp_platform_module = platform_module

    if "arm_accton_as4610" in imp_platform_module:
         imp_platform_module = imp_platform_module.replace('_30_r0', '_r0')
         imp_platform_module = imp_platform_module.replace('_54_r0', '_r0')

    # Import the platform module
    m = importlib.import_module('%s.platform.%s' % (subsystem, imp_platform_module))

    return getattr(m, '%s_%s' % (klass, platform_module))


OnlPlatform = import_subsystem_platform_class()


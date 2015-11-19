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
import importlib

def import_subsystem_platform_class(subsystem='onl', klass='OnlPlatform'):
    # Determine the current platform name.
    with open("/etc/onl/platform", 'r') as f:
        platform=f.read().strip()

    platform_module = platform.replace('-', '_')

    # Import the platform module
    m = importlib.import_module('%s.platform.%s' % (subsystem, platform_module))

    return getattr(m, '%s_%s' % (klass, platform_module))


OnlPlatform = import_subsystem_platform_class()


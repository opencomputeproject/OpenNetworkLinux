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
import os, sys
import importlib
import subprocess
import ast

def platform_name_get():
    # Determine the current platform name.
    platform = None

    # running ONL proper
    if platform is None and os.path.exists("/etc/onl/platform"):
        with open("/etc/onl/platform", 'r') as f:
            platform=f.read().strip()

    # in the middle of an ONL install
    if platform is None and os.path.exists("/etc/onl/installer.conf"):
        with open("/etc/onl/installer.conf") as f:
            lines = f.readlines(False)
            lines = [x for x in lines if x.startswith('onie_platform')]
            if lines:
                platform = lines[0].partition('=')[2].strip()

    # running ONIE
    if platform is None and os.path.exists("/bin/onie-sysinfo"):
        try:
            platform = subprocess.check_output(('/bin/onie-sysinfo', '-p',)).strip()
        except subprocess.CalledProcessError as what:
            for line in (what.output or "").splitlines():
                sys.stderr.write(">>> %s\n" % line)
            sys.stderr.write("onie-sysinfo failed with code %d\n" % what.returncode)
            platform = None

    # running ONL loader, with access to ONIE
    if platform is None and os.path.exists("/usr/bin/onie-shell"):
        try:
            platform = subprocess.check_output(('/usr/bin/onie-shell', '-c', "onie-sysinfo -p",)).strip()
        except subprocess.CalledProcessError as what:
            for line in (what.output or "").splitlines():
                sys.stderr.write(">>> %s\n" % line)
            sys.stderr.write("onie-sysinfo (onie-shell) failed with code %d\n" % what.returncode)
            platform = None

    # legacy ONIE environment (including parsable shell in machine.conf)
    if platform is None and os.path.exists("/etc/machine.conf"):
        cmd = "IFS=; . /tmp/machine.conf; set | egrep ^onie_platform="
        buf = subprocess.check_output(cmd)
        if buf:
            platform = buf.partition('=')[2].strip()
            if platform.startswith('"') or platform.startswith("'"):
                platform = ast.literal_eval(platform)

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

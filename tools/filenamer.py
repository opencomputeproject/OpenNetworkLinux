#!/usr/bin/python2
############################################################
#
# This script provides the file naming scheme for
# various build products (Switch Images, Installers, etc)
#
import argparse
import json

ap=argparse.ArgumentParser(description="ONL File Namer")
ap.add_argument("--type",      help="File Type.", choices = [ 'swi', 'installer' ], required=True)
ap.add_argument("--manifest",  help="File manifest.", required=True)
ap.add_argument("file",        help="File.")
ops = ap.parse_args()

manifest = json.load(open(ops.manifest))
versions = manifest['version']
versions['UARCH'] = manifest['arch'].upper().replace("POWERPC","PPC")


if ops.type == 'swi':
    print "%(FNAME_PRODUCT_VERSION)s_ONL-OS_%(FNAME_BUILD_ID)s_%(UARCH)s.swi" % versions
elif ops.type == 'installer':
    print "%(FNAME_PRODUCT_VERSION)s_ONL-OS_%(FNAME_BUILD_ID)s_%(UARCH)s_INSTALLER" % versions
else:
    raise ValueError("Unknown type '%s'" % ops.type)








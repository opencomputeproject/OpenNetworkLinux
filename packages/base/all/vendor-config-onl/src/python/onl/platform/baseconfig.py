############################################################
#
# Platform Base Configuration
#
############################################################
import sys
import os
from onl.platform.base import OnlPlatformBase
from onl.platform.current import OnlPlatform
import shutil

def msg(s, fatal=False):
    sys.stderr.write(s)
    sys.stderr.flush()
    if fatal:
        sys.exit(1)

def baseconfig():

    platform=OnlPlatform()

    msg("Setting up base ONL platform configuration for %s...\n" %
        platform.platform())

    if os.path.exists(OnlPlatform.CURRENT_DIR):
        os.unlink(OnlPlatform.CURRENT_DIR)

    os.symlink(platform.basedir(), OnlPlatform.CURRENT_DIR)

    DEB_GNU_HOST_TYPE = None
    HOST_TYPES = [ 'powerpc-linux-gnu',
                   'i486-linux-gnu',
                   'i386-linux-gnu',
                   'x86_64-linux-gnu',
                   'arm-linux-gnueabi',
                   'aarch64-linux-gnu',
                   ]

    for ht in HOST_TYPES:
        if os.path.exists('/lib/%s' % ht):
            DEB_GNU_HOST_TYPE=ht
            break

    if DEB_GNU_HOST_TYPE is None:
        msg("Could not determine the current host type.\n", fatal=True)

    DEFAULT_ONLP_LIB = "/lib/%s/libonlp-platform.so" % DEB_GNU_HOST_TYPE

    PLATFORM_ONLP_LIBS = [
        # Look for full platform and revision library
        "%s/lib/libonlp-%s.so" % (platform.basedir_onl(), platform.platform()),
        # Look for common platform library
        "%s/lib/libonlp-%s.so" % (platform.basedir_onl(), platform.baseplatform()),
        ]

    for l in PLATFORM_ONLP_LIBS:
        if os.path.exists(l):
            if os.path.exists(DEFAULT_ONLP_LIB):
                os.unlink(DEFAULT_ONLP_LIB)
            os.symlink(l, DEFAULT_ONLP_LIB)

    ONLPDUMP = "%s/bin/onlpdump" % (platform.basedir_onl())

    try:
        import dmidecode
        with open("%s/dmi-system-version" % platform.basedir_onl(), "w") as f:
            f.write(dmidecode.QuerySection('system')['0x0001']['data']['Version'])
    except:
        pass

    if not platform.baseconfig():
        msg("*** platform class baseconfig failed.\n", fatal=True)

    if os.path.exists(ONLPDUMP):
        os.system("%s -i > %s/oids" % (ONLPDUMP,platform.basedir_onl()))
        os.system("%s -o -j > %s/onie-info.json" % (ONLPDUMP, platform.basedir_onl()))
        os.system("%s -x -j > %s/platform-info.json" % (ONLPDUMP, platform.basedir_onl()))

    msg("Setting up base platform configuration for %s: done\n" %
        platform.platform())

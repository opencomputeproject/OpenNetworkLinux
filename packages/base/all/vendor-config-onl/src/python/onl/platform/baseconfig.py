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

    ONLPD = "/bin/onlpd"

    try:
        import dmidecode
        with open("%s/dmi-system-version" % platform.basedir_onl(), "w") as f:
            f.write(dmidecode.QuerySection('system')['0x0001']['data']['Version'])
    except:
        pass
    finally:
        if 'dmidecodemod' in sys.modules:
            mod = sys.modules['dmidecodemod']
            buf = mod.get_warnings()
            if buf:
                [msg("*** %s\n" % x) for x in buf.splitlines(False)]
            mod.clear_warnings()

    if not platform.baseconfig():
        msg("*** platform class baseconfig failed.\n", fatal=True)

    if os.path.exists(ONLPD):
        chassis_dir = os.path.join(platform.basedir_onl(), "chassis")
        if not os.path.isdir(chassis_dir):
            os.makedirs(chassis_dir)
        os.system("%s chassis onie  json > %s/onie-info.json" % (ONLPD, chassis_dir))
        os.system("%s chassis asset json > %s/asset-info.json" % (ONLPD, chassis_dir))

    msg("Setting up base platform configuration for %s: done\n" %
        platform.platform())

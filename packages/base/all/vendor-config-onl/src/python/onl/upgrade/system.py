#!/usr/bin/python
############################################################
#
# ONL System Upgrade
#
############################################################
import os
import sys
import fnmatch
from onl.upgrade import ubase
from onl.sysconfig import sysconfig
from onl.mounts import OnlMountManager, OnlMountContextReadOnly, OnlMountContextReadWrite

from onl.install.SystemInstall import App

class SystemUpgrade(ubase.BaseUpgrade):
    name="system"
    Name="System"
    title="System Compatibility Version Check"
    atype="A Compatible System"

    current_version_key="Current System Compatibility Version"
    next_version_key="Next System Compatibility Version"

    def auto_upgrade_default(self):
        return sysconfig.upgrade.system.auto

    def init_versions(self):

        #
        # Current loader version file.
        # If this file doesn't exist then in-place upgrade is not supported.
        #
        ETC_LOADER_VERSIONS_JSON = sysconfig.upgrade.loader.versions

        # Upgrade Loader Version file.
        NEXT_LOADER_VERSIONS_JSON = os.path.join(sysconfig.upgrade.loader.package.dir, "manifest.json")

        VKEY = "SYSTEM_COMPATIBILITY_VERSION"

        self.current_version = self.load_json(ETC_LOADER_VERSIONS_JSON, VKEY, None)

        self.next_version = self.load_json(NEXT_LOADER_VERSIONS_JSON,
                                           "version", {}).get(VKEY, None)

    def prepare_upgrade(self):
        pass

    def summarize(self):
        self.logger.info("Current System Compatibility Version: %s",
                         self.current_version)
        self.logger.info("   Next System Compatibility Version: %s",
                         self.next_version)
        self.logger.info("")


    def upgrade_notes(self):
        return """
    * One or more reboots will be required to complete this upgrade.
"""

    def do_upgrade(self, forced=False):
        app = App(force=True, log=self.logger)
        try:
            code = app.run()
        except:
            self.logger.exception("upgrade failed")
            code = 1
        app.shutdown()
        if code:
            self.abort("System upgrade failed.")
        else:
            self.logger.info("Upgrade succeeded, rebooting")
            self.reboot()

if __name__ == '__main__':
    klass = SystemUpgrade
    klass().main()

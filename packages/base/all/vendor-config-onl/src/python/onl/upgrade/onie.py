#!/usr/bin/python -u

import os
import sys

from onl.upgrade import ubase
from onl.sysconfig import sysconfig
from onl.mounts import OnlMountManager, OnlMountContextReadOnly, OnlMountContextReadWrite

class OnieUpgrade(ubase.BaseOnieUpgrade):

    name="onie"
    Name="ONIE"
    title="ONIE Upgrade Check"
    atype="An ONIE"

    current_version_key="Current ONIE Version"
    next_version_key="Next ONIE Version"

    def init_versions(self):

        # Get the current platform ONIE version
        self.current_version = self.platform.onie_version()
        self.next_version = None
        self.updater = None

        self.manifest = self.load_json(os.path.join(sysconfig.upgrade.onie.package.dir, "manifest.json"))

        if self.manifest is None:
            self.finish("No ONIE updater available for the current platform.")

        if 'onie-version' not in self.manifest:
            self.finish("No ONIE version in the upgrade manifest.")
        else:
            self.next_version = self.manifest['onie-version']

        if 'onie-updater' not in self.manifest:
            self.finish("No ONIE updater in the upgrade manifest.")


    def summarize(self):
        self.logger.info("Current ONIE Version: %s" % self.current_version)
        self.logger.info("   Next ONIE Version: %s" % self.manifest.get('onie-version'))
        self.logger.info("             Updater: %s" % self.manifest.get('onie-updater'))
        self.logger.info("")

    def upgrade_notes(self):
        return """
    * The system will reboot into ONIE to complete the update, and then reboot to return to Switch Light
"""

    def do_upgrade(self, forced=False):
        self.install_onie_updater(sysconfig.upgrade.onie.package.dir,
                                  self.manifest['onie-updater'])
        self.initiate_onie_update()

    def do_no_upgrade(self):
        self.clean_onie_updater()


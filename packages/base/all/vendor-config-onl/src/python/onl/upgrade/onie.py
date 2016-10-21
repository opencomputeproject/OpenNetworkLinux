#!/usr/bin/python -u

import os
import sys

from onl.upgrade import ubase
from onl.sysconfig import sysconfig

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
        self.load_manifest(os.path.join(sysconfig.upgrade.onie.package.dir, "manifest.json"))

    def do_upgrade(self, forced=False):
        self.install_onie_updater(sysconfig.upgrade.onie.package.dir,
                                  self.manifest['updater'])
        self.initiate_onie_update()

    def do_no_upgrade(self):
        self.clean_onie_updater()

    def upgrade_notes(self):
        return """
    * The system will reboot into ONIE to complete the update, and then reboot to return to Switch Light
"""



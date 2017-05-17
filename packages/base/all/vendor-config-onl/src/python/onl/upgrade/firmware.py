#!/usr/bin/python -u

import os
import sys

from onl.upgrade import ubase
from onl.sysconfig import sysconfig

class FirmwareUpgrade(ubase.BaseOnieUpgrade):

    name="firmware"
    Name="Firmware"
    title="Firmware Upgrade Check"
    atype="A firmware"

    current_version_key="Current Firmware Version"
    next_version_key="Next Firmware Version"

    def auto_upgrade_default(self):
        return sysconfig.upgrade.firmware.auto

    def init_versions(self):

        # Get the current platform firmware version
        self.current_version = self.platform.firmware_version()
        self.next_version = None
        self.updater = None
        self.load_manifest(os.path.join(sysconfig.upgrade.firmware.package.dir, "manifest.json"))

    def do_upgrade(self, forced=False):
        if self.manifest.get('fwpkg', False):
            if not self.onie_fwpkg_exists():
                # An ONIE upgrade is probably required.
                print "The firmware cannot be upgraded because the current ONIE version is not correct. Please perform an ONIE upgrade first."
                self.abort()

            self.onie_fwpkg_add(os.path.join(sysconfig.upgrade.firmware.package.dir,
                                             self.manifest['updater']))
        else:
            self.install_onie_updater(sysconfig.upgrade.firmware.package.dir,
                                      self.manifest['updater'])
        self.initiate_onie_update()


    def upgrade_notes(self):
        notes = """
    * Two reboots will be required to complete this upgrade.

    * Do not turn the power off on this device until the upgrade is complete.
      Disrupting power during the firmware upgrade may result in an unrecoverable system."""

        duration = self.manifest.get("duration", None)
        if duration:
            notes = notes + """

    * THIS UPGRADE WILL REQUIRE APPROXIMATELY %s MINUTES.
      The system will reboot when completed.""" % duration

        return notes


    def do_no_upgrade(self):
        self.clean_onie_updater()

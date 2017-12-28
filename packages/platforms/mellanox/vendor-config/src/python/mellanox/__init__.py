#!/usr/bin/python

from onl.platform.base import *

class OnlPlatformMellanox(OnlPlatformBase):
    MANUFACTURER='Mellanox'
    PRIVATE_ENTERPRISE_NUMBER=33049

    #
    # Some platforms rely on the output of the onie-syseeprom tool
    # and the machine.conf file to implement parts of ONLP.
    #
    def syseeprom_export(self):
        print "Caching ONIE System EEPROM..."
        onie = self.onie_syseeprom_get()
        mc = self.onie_machine_get()
        # XXX roth -- deprecated
        if 'onie_version' in mc:
            onie['0x29'] = mc['onie_version']
            self.onie_syseeprom_set(onie)

"""sample-preinstall.py

"""

import onl.install.Plugin

class Plugin(onl.install.Plugin.Plugin):

    def run(self):
        self.log.info("hello from preinstall plugin")
        return 0

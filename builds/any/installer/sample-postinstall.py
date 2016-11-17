"""sample-postinstall.py

"""

import onl.install.Plugin

class Plugin(onl.install.Plugin.Plugin):

    def run(self):
        self.log.info("hello from postinstall plugin")
        return 0

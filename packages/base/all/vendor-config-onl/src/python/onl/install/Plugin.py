"""Plugin.py

Base class for installer plugins.
"""

class Plugin(object):

    def __init__(self, installer):
        self.installer = installer
        self.log = self.installer.log.getChild("plugin")

    def run(self):
        self.log.warn("not implemented")
        return 0

    def shutdown(self):
        pass

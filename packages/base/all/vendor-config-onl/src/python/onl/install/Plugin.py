"""Plugin.py

Base class for installer plugins.
"""

class Plugin(object):

    PLUGIN_PREINSTALL = "preinstall"
    PLUGIN_POSTINSTALL = "postinstall"

    def __init__(self, installer):
        self.installer = installer
        self.log = self.installer.log.getChild("plugin")

    def run(self, mode):

        if mode == self.PLUGIN_PREINSTALL:
            self.log.warn("pre-install plugin not implemented")
            return 0

        if mode == self.PLUGIN_POSTINSTALL:
            self.log.warn("post-install plugin not implemented")
            return 0

        self.log.warn("invalid plugin mode %s", repr(mode))
        return 1

    def shutdown(self):
        pass

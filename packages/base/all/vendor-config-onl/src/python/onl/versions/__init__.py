import os
import json

class OnlVersionManifest(object):
    def __init__(self, manifest):
        self.version = json.load(open(manifest))

        if 'version' in self.version:
            self.version = self.version['version']

    def __getattr__(self, name):
        if name in self.version:
            return self.version[name]
        else:
            raise AttributeError("version key '%s' does not exist." % name)

class OnlVersionBase(OnlVersionManifest):
    def __init__(self):
        OnlVersionManifest.__init__(self, self.MANIFEST)

class OnlRootfsVersion(OnlVersionBase):
    MANIFEST='/etc/onl/rootfs/manifest.json'

class OnlLoaderVersion(OnlVersionBase):
    MANIFEST='/etc/onl/loader/manifest.json'

#
# Expected usage:
#
# import onl.versions
#
# print onl.versions.rootfs.BUILD_TIMESTAMP
#
rootfs = OnlRootfsVersion() if os.path.exists(OnlRootfsVersion.MANIFEST) else None
loader = OnlLoaderVersion() if os.path.exists(OnlLoaderVersion.MANIFEST) else None




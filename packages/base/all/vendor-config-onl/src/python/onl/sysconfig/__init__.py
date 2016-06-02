############################################################
#
# ONL System Configuration
#
############################################################
import os
import sys
import yaml
import types
import onl.onlyaml
import onl.util
import platform as pp
from onl.platform.current import OnlPlatform

class DotDict(dict):
    """ Access keys in a nested dictionary using dot notation """

    def __getattr__(self, attr):
        item = self.get(attr, None)

        if item is None:
            raise AttributeError("'%s' object has no attribute '%s'" % (type(self), attr))

        if type(item) == types.DictType:
            item = DotDict(item)

        return item

    __setattr__= dict.__setitem__
    __delattr__= dict.__delitem__


class OnlSystemConfig(object):
    SYSTEM_CONFIG_DIR = '/etc/onl/config'

    def __init__(self):

        platform = OnlPlatform()
        self.variables = {}
        self.variables['PLATFORM'] = platform.platform()
        self.variables['ARCH'] = pp.machine()
        self.variables['PARCH'] = dict(ppc='powerpc',
                                       x86_64='amd64',
                                       armv7l='armel')[pp.machine()]

        self.config = {}
        for f in sorted(os.listdir(self.SYSTEM_CONFIG_DIR)):
            if f.endswith('.yml'):
                d = onl.onlyaml.loadf(os.path.join(self.SYSTEM_CONFIG_DIR, f), self.variables)
                self.config = onl.util.dmerge(self.config, d)

        self.config['pc'] = platform.platform_config

    def dump(self):
        return yaml.dump(self.config, default_flow_style=False)

x = OnlSystemConfig()
sysconfig = DotDict(x.config)
sysconfig['OnlSystemConfig'] = x

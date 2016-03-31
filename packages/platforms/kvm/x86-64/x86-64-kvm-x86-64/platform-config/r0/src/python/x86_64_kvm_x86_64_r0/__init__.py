from onl.platform.base import *
from onl.platform.kvm import *

class OnlPlatform_x86_64_kvm_x86_64_r0(OnlPlatformKVM):

    def model(self):
        return "KVM X86_64"

    def platform(self):
        return "x86-64-kvm-x86-64-r0"

    def sys_oid_platform(self):
        return ".1"

    def baseconfig(self):
        return True

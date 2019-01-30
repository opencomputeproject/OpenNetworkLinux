#!/usr/bin/python
############################################################
import os
import sys
import netaddr

class OnlBootConfig(object):
    BOOT_CONFIG_DEFAULT='/mnt/onl/boot/boot-config'

    def __init__(self):
        self.keys = {}
        self.__classmethod("init")

    def _readf(self, fname):
        with open(fname) as f:
            for line in f.readlines():
                (k,d,v) = line.partition('=')
                if d == '=':
                    self.keys[k] = v.strip()
        self._original = self.keys.copy()

    def read(self, bc=None):
        if bc:
            self._readf(bc)
        elif os.path.exists(self.BOOT_CONFIG_DEFAULT):
            self._readf(self.BOOT_CONFIG_DEFAULT)
        else:
            from onl.mounts import OnlMountContextReadOnly
            with OnlMountContextReadOnly("ONL-BOOT", logger=None):
                self._readf(self.BOOT_CONFIG_DEFAULT)

    def set(self, k, v):
        self.keys[k] = v

    def get(self, k, d=None):
        return self.keys.get(k, d)

    def delete(self, k):
        self.keys.pop(k, None)

    def _writeh(self, handle):
        for (k, v) in self.keys.iteritems():
            handle.write("%s=%s\n" % (k, v))

    def _writef(self, f):
        with open(f, "w") as f:
            self._writeh(f)

    def write(self, dst=None, force_overwrite=True):
        self.validate()
        if dst:
            self._writef(dst)
            return True
        else:
            from onl.mounts import OnlMountContextReadWrite
            with OnlMountContextReadWrite("ONL-BOOT", logger=None):
                if not os.path.exists(self.BOOT_CONFIG_DEFAULT) or force_overwrite:
                    self._writef(self.BOOT_CONFIG_DEFAULT)
                    return True
            return False


    def __classmethod(self, name, *args):
        for attr in dir(self):
            if attr.endswith("__%s" % name):
                getattr(self, attr)(*args)

    def validate(self):
        return self.__classmethod("validate")

    def argparse_init(self, ap):
        ap.add_argument("--read", help="Read the given file instead of the default [ %s ]." % OnlBootConfig.BOOT_CONFIG_DEFAULT)
        ap.add_argument("--write", help="Write the given file instead of the default [ %s ]." % OnlBootConfig.BOOT_CONFIG_DEFAULT)
        ap.add_argument("--show", help="Show the configuration.", action='store_true')
        self.__classmethod("argparse_init", ap)
        ap.add_argument("--dry", help='Show changes but do not update.', action='store_true')

    def argparse_process(self, ops):
        self.read(ops.read)
        if(ops.show):
            self._writeh(sys.stdout)
        return self.__classmethod("argparse_process", ops)

    def argparse_write(self, ops):
        try:
            if ops.dry:
                print self.keys
                self.validate()
            else:
                self.write(ops.write)
                if not ops.write and self.keys != self._original:
                    print "You must reboot the switch before these changes will take affect."

        except Exception, e:
            print e
            print "The boot configuration has not been changed."


    def main(self, name):
        import argparse
        ap = argparse.ArgumentParser("name")
        self.argparse_init(ap)
        ops = ap.parse_args()
        self.argparse_process(ops)
        self.argparse_write(ops)


class OnlBootConfigNet(OnlBootConfig):

    NET_REQUIRED = False

    def netauto_set(self):
        self.delete('NETIP')
        self.delete('NETMASK')
        self.delete('NETGW')
        self.delete('NETDNS')
        self.delete('NETDOMAIN')
        self.set('NETAUTO', 'dhcp')

    def netauto_get(self):
        return self.keys.get('NETAUTO', None)

    def netip_set(self, addr):
        self.delete('NETAUTO')
        self.keys['NETIP'] = addr

    def netip_get(self):
        return self.keys.get('NETIP', None)

    def netmask_set(self, mask):
        self.delete('NETAUTO')
        self.keys['NETMASK'] = mask

    def netmask_get(self):
        return self.keys.get('NETMASK', None)

    def netgw_set(self, gw):
        self.delete('NETAUTO')
        self.keys['NETGW'] = gw

    def netgw_get(self):
        return self.keys.get('NETGW', None)

    def netdns_set(self, dns):
        self.delete('NETAUTO')
        self.keys['NETDNS'] = dns

    def netdns_get(self):
        return self.keys.get('NETDNS', None)

    def netdomain_set(self, domain):
        self.delete('NETAUTO')
        self.keys['NETDOMAIN'] = domain

    def netdomain_get(self):
        return self.keys.get('NETDOMAIN', None)

    def __validate(self):
        if 'NETAUTO' not in self.keys:

            netip = self.keys.get('NETIP', None)
            if netip:
                if not self.is_ip_address(netip):
                    raise ValueError("NETIP=%s is not a valid ip-address" % (netip))
            elif self.NET_REQUIRED:
                raise ValueError("No IP configuration set for the management interface.")

            netmask = self.keys.get('NETMASK', None)
            if netmask:
                if not self.is_netmask(netmask):
                    raise ValueError("NETMASK=%s is not a valid netmask." % (netmask))
            elif self.NET_REQUIRED:
                raise ValueError("No Netmask configured for the management interface.")

            netgw = self.keys.get('NETGW', None)
            if netgw:
                if not self.is_ip_address(netgw):
                    raise ValueError("NETGW=%s is not a valid ip-address." % (netgw))
            elif self.NET_REQUIRED:
                raise ValueError("No gateway configured for the management interface.")

            if netip and netmask and netgw:
                net = netaddr.IPNetwork("%s/%s" % (netip, netmask))
                if netaddr.IPAddress(netgw) not in net:
                    raise ValueError("Gateway provided is not within the management network %s" % net)
            elif netip or netmask or netgw:
                raise ValueError("Incomplete static network configuration. NETIP, NETMASK, and NETGW must all be set.")

            netdns = self.keys.get('NETDNS', None)
            if netdns:
                if not self.is_ip_address(netdns):
                    raise ValueError("NETDNS=%s is not a valid ip-address" % (netdns))

        elif self.keys['NETAUTO'] not in ['dhcp', 'up', 'none', '']:
            raise ValueError("The NETAUTO value '%s' is invalid." % self.keys['NETAUTO'])
        elif self.keys['NETAUTO'] == 'up' and self.NET_REQUIRED:
            raise ValueError("NETAUTO is 'up' but non-local networking is required.")

        if 'NETDEV' not in self.keys:
            self.keys['NETDEV'] = 'ma1'

        return True

    @staticmethod
    def is_ip_address(value):
        try:
            netaddr.IPAddress(value)
            return value
        except (netaddr.core.AddrFormatError, ValueError):
            return None

    @staticmethod
    def is_netmask(value):
        try:
            if not netaddr.IPAddress(value).is_netmask():
                return False
            return value
        except (netaddr.core.AddrFormatError, ValueError):
            return False

    @staticmethod
    def argparse_type_is_ip_address(value):
        if not OnlBootConfigNet.is_ip_address(value):
            import argparse
            raise argparse.ArgumentTypeError("%s is not a valid address." % value)
        return value

    @staticmethod
    def argparse_type_is_netmask(value):
        if not OnlBootConfigNet.is_netmask(value):
            import argparse
            raise argparse.ArgumentTypeError("%s is not a valid netmask." % value)
        return value

    def __argparse_init(self, ap):
        ap.add_argument("--dhcp", action='store_true', help="Use DHCP on the management interface.")
        ap.add_argument("--ip", help='Set static IP address for the management interface.', type=OnlBootConfigNet.argparse_type_is_ip_address)
        ap.add_argument("--netmask", help='Set the static netmask for the management interface.', type=OnlBootConfigNet.argparse_type_is_netmask)
        ap.add_argument("--gateway", help='Set the gateway address.', type=OnlBootConfigNet.argparse_type_is_ip_address)
        ap.add_argument("--dns", help='Set the dns server.', type=OnlBootConfigNet.argparse_type_is_ip_address)
        ap.add_argument("--domain", help='Set the dns domain.')

    def __argparse_process(self, ops):
        if ops.dhcp:
            self.netauto_set()

        if ops.ip:
            self.netip_set(ops.ip)

        if ops.netmask:
            self.netmask_set(ops.netmask)

        if ops.gateway:
            self.netgw_set(ops.gateway)

        if ops.dns:
            self.netdns_set(ops.dns)

        if ops.domain:
            self.netdomain_set(ops.domain)


if __name__ == '__main__':
    bc = OnlBootConfigNet()
    bc.main("onl-boot-config")

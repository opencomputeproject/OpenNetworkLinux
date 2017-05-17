"""__init__.py

"""

import subprocess

class HostInfo:

    def __init__(self, host,
                 user=None, password=None,
                 port=None,
                 scope=None):
        self.user = user
        self.password = password
        self.port = port

        if scope is None and host.startswith('fe80:'):
            cmd = ('ip', '-6', '-oneline', 'route', 'show',)
            for line in subprocess.check_output(cmd).splitlines(False):
                words = line.split()
                if words[0].startswith('fe80:'):
                    scope = words[2]
                    break

        if scope is not None:
            self.host = host + '%' + scope
        else:
            self.host = host

        # try to canonicalize the scope
        # add scope to host
        # set bhost to bracketed host

        if ':' in host:
            self.bhost = '[' + self.host + ']'
        else:
            self.bhost = self.host

    @classmethod
    def fromString(cls, hinfo):

        buf = hinfo
        l, sep, r = buf.partition('@')
        if sep:
            uinfo, buf = l, r
            l, sep, r = uinfo.partition(':')
            if sep:
                u, p = l, r
            else:
                u, p = uinfo, None
        else:
            u = p = None

        if not buf.startswith('['):
            s = None
            l, sep, r = buf.partition(':')
            if sep:
                h, p = l, int(r)
            else:
                h, p = buf, None
        else:
            l, sep, r = buf.partition(']')
            if not sep:
                raise ValueError("invalid host specifier %s" % hinfo)
            h = l[1:]
            if r and r.startswith(':'):
                p = int(r[1:])
            elif not r:
                p = None
            else:
                raise ValueError("invalid host specifier %s" % hinfo)
            i = h.find('%25')
            if i > -1:
                h, s = h[:i], h[i+3:]
            else:
                l, sep, r = h.partition('%')
                if sep:
                    h, s = l, r
                else:
                    s = None

        return cls(h, port=p, user=u, password=p, scope=s)

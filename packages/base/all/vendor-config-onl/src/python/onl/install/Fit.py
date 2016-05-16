"""Fit.py

Parse FIT files.
"""

import os, sys
import logging
import struct
import argparse
import time

class FdtProperty:
    def __init__(self, name, offset, sz):
        self.name = name
        self.offset = offset
        self.sz = sz

class FdtNode:
    def __init__(self, name):
        self.name = name
        self.properties = {}
        self.nodes = {}

class Parser:

    FDT_MAGIC = 0xd00dfeed

    FDT_BEGIN_NODE = 1
    FDT_END_NODE = 2
    FDT_PROP = 3
    FDT_NOP = 4
    FDT_END = 9

    def __init__(self, path=None, stream=None, log=None):
        self.log = log or logging.getLogger(self.__class__.__name__)
        self.path = path
        self.stream = stream
        self.rootNodes = {}
        self._parse()

    def _parse(self):
        if self.stream is not None:
            try:
                pos = self.stream.tell()
                self._parseStream(self.stream)
            finally:
                self.stream.seek(pos, 0)
        elif self.path is not None:
            with open(self.path) as fd:
                self._parseStream(fd)
        else:
            raise ValueError("missing file or stream")

    def _parseStream(self, fd):
        strings = {}

        buf = fd.read(40)
        hdr = list(struct.unpack(">10I", buf))
        magic = hdr.pop(0)
        if magic != self.FDT_MAGIC:
            raise ValueError("missing magic")
        self.fdtSize = hdr.pop(0)
        self.structPos = hdr.pop(0)
        self.stringPos = hdr.pop(0)
        self.version = hdr.pop(0)
        if self.version < 17:
            raise ValueError("invalid format version")
        hdr.pop(0) # last compatible version
        hdr.pop(0) # boot cpu
        self.stringSize = hdr.pop(0)
        self.structSize = hdr.pop(0)

        fd.seek(self.structPos, 0)

        def _align():
            pos = fd.tell()
            pos = (pos+3) & ~3
            fd.seek(pos, 0)

        def _label():
            buf = ""
            while True:
                c = fd.read(1)
                if c == '\x00': break
                if c:
                    buf += c
            return buf

        def _string(off):
            if off in strings:
                return strings[off]
            pos = fd.tell()
            fd.seek(self.stringPos, 0)
            fd.seek(off, 1)
            buf = _label()
            fd.seek(pos)
            return buf

        nodeStack = []

        while True:
            buf = fd.read(4)
            s = list(struct.unpack(">I", buf))
            tag = s.pop(0)

            if tag == self.FDT_BEGIN_NODE:
                name = _label()
                _align()

                newNode = FdtNode(name)

                if nodeStack:
                    if name in nodeStack[-1].nodes:
                        raise ValueError("duplicate node")
                    nodeStack[-1].nodes[name] = newNode
                    nodeStack.append(newNode)
                else:
                    if name in self.rootNodes:
                        raise ValueError("duplicate node")
                    self.rootNodes[name] = newNode
                    nodeStack.append(newNode)

                continue

            if tag == self.FDT_PROP:
                buf = fd.read(8)
                s = list(struct.unpack(">2I", buf))
                plen = s.pop(0)
                nameoff = s.pop(0)
                name = _string(nameoff)
                pos = fd.tell()
                fd.seek(plen, 1)
                _align()

                newProp = FdtProperty(name, pos, plen)

                if nodeStack:
                    if name in nodeStack[-1].properties:
                        raise ValueError("duplicate property")
                    nodeStack[-1].properties[name] = newProp
                else:
                    raise ValueError("property with no node")

                continue

            if tag == self.FDT_END_NODE:
                if nodeStack:
                    nodeStack.pop(-1)
                else:
                    raise ValueError("missing begin node")
                continue

            if tag == self.FDT_NOP:
                print "NOP"
                continue

            if tag == self.FDT_END:
                if nodeStack:
                    raise ValueError("missing end node(s)")
                break

            raise ValueError("invalid tag %d" % tag)

    def report(self, stream=sys.stdout):
        q = [(x, "") for x in self.rootNodes.values()]
        while q:
            n, pfx = q.pop(0)

            name = n.name or "/"
            stream.write("%s%s\n" % (pfx, name,))

            if n.properties:
                stream.write("\n")
            for p in n.properties.values():
                stream.write("%s    %s (%d bytes)\n"
                             % (pfx, p.name, p.sz,))
            if n.properties:
                stream.write("\n")

            pfx2 = pfx + "    "
            q[0:0] = [(x, pfx2) for x in n.nodes.values()]

    def getNode(self, path):
        if path == '/':
            return self.rootNodes.get('', None)

        els = path.split('/')
        n = None
        while els:
            b = els.pop(0)
            if n is None:
                if b not in self.rootNodes: return None
                n = self.rootNodes[b]
            else:
                if b not in n.nodes: return None
                n = n.nodes[b]
        return n

    def getNodeProperty(self, node, propName):
        if propName not in node.properties: return None
        prop = node.properties[propName]
        def _get(fd):
            fd.seek(self.structPos, 0)
            fd.seek(prop.offset)
            buf = fd.read(prop.sz)
            if buf[-1] == '\x00':
                return buf[:-1]
            return buf
        if self.stream is not None:
            return _get(self.stream)
        else:
            with open(self.path) as fd:
                return _get(fd)

    def dumpNodeProperty(self, node, propIsh, outPath):
        if isinstance(propIsh, FdtProperty):
            prop = propIsh
        else:
            if propIsh not in node.properties:
                raise ValueError("missing property")
            prop = node.properties[propIsh]
        def _dump(fd):
            with open(outPath, "w") as wfd:
                fd.seek(prop.offset, 0)
                buf = fd.read(prop.sz)
                wfd.write(buf)
        if self.stream is not None:
            try:
                pos = self.stream.tell()
                _dump(self.stream)
            finally:
                self.stream.seek(pos, 0)
        else:
            with open(self.path) as fd:
                _dump(fd)

    def getInitrdNode(self, profile=None):
        """U-boot mechanism to retrieve boot profile."""

        node = self.getNode('/configurations')
        if node is None:
            self.log.warn("missing /configurations node")
            return None
        if profile is not None:
            if profile not in node.nodes:
                self.log.warn("missing profile %s", profile)
                return None
            node = node.nodes[profile]
        elif 'default' in node.properties:
            pf = self.getNodeProperty(node, 'default')
            self.log.debug("default profile is %s", pf)
            node = node.nodes[pf]
        else:
            pf = node.nodes.keys()[0]
            self.log.debug("using profile %s", pf)
            node = node.nodes[pf]

        if 'ramdisk' not in node.properties:
            self.log.warn("ramdisk property not found")
            return None
        rdName = self.getNodeProperty(node, 'ramdisk')

        self.log.debug("retrieving ramdisk %s", rdName)
        node = self.getNode('/images/' + rdName)
        return node

class DumpRunner:

    def __init__(self, stream,
                 log=None):
        self.log = log or logging.getLogger(self.__class__.__name__)
        self.stream = stream

    def run(self):
        p = Parser(stream=self.stream, log=self.log)
        p.report()
        return 0

    def shutdown(self):
        stream, self.stream = self.stream, None
        if stream is not None: stream.close()

class ExtractBase:

    def __init__(self, stream,
                 initrd=False, profile=None, path=None,
                 property=None,
                 log=None):
        self.log = log or logging.getLogger(self.__class__.__name__)
        self.stream = stream
        self.initrd = initrd
        self.profile = profile
        self.path = path
        self.property = property

        self.parser = None
        self.node = None
        self.dataProp = None

    def run(self):
        self.parser = Parser(stream=self.stream, log=self.log)
        if self.path is not None:
            self.node = self.parser.getNode(self.path)
            if self.node is None:
                self.log.error("cannot find path")
                return 1
        elif self.initrd:
            self.node = self.parser.getInitrdNode(profile=self.profile)
            if self.node is None:
                self.log.error("cannot find initrd")
                return 1
        else:
            self.log.error("missing path or initrd")
            return 1

        def _t(n):
            if n is None: return
            self.dataProp = self.dataProp or self.node.properties.get(n, None)
        _t(self.property)
        _t('data')
        _t('value')
        if self.dataProp is None:
            self.log.error("cannot find %s property", self.property)
            return 1

        return self._handleParsed()

    def _handleParsed(self):
        raise NotImplementedError

    def shutdown(self):
        stream, self.stream = self.stream, None
        if stream is not None: stream.close()

class ExtractRunner(ExtractBase):

    def __init__(self, stream,
                 outStream=None,
                 initrd=False, profile=None, path=None,
                 property=None,
                 text=False, numeric=False, timestamp=False, hex=False,
                 log=None):
        ExtractBase.__init__(self, stream,
                             initrd=initrd, profile=profile,
                             path=path,
                             property=property,
                             log=log)
        self.outStream = outStream
        self.text = text
        self.numeric = numeric
        self.timestamp = timestamp
        self.hex = hex

    def _handleParsed(self):
        if (self.numeric or self.timestamp) and self.dataProp.sz != 4:
            self.log.error("invalid size for number")
            return 1
        def _dump(rfd, wfd):
            rfd.seek(self.dataProp.offset, 0)
            buf = rfd.read(self.dataProp.sz)
            if self.text:
                if buf[-1:] != '\x00':
                    self.log.error("missing NUL terminator")
                    return 1
                wfd.write(buf[:-1])
                return 0
            if self.numeric:
                n = struct.unpack(">I", buf)[0]
                wfd.write(str(n))
                return 0
            if self.timestamp:
                n = struct.unpack(">I", buf)[0]
                wfd.write(time.ctime(n))
                return 0
            if self.hex:
                for c in buf:
                    wfd.write("%02x" % ord(c))
                return 0
            wfd.write(buf)
            return 0
        if self.outStream is not None:
            return _dump(self.stream, self.outStream)
        else:
            return _dump(self.stream, sys.stdout)

class OffsetRunner(ExtractBase):

    def __init__(self, stream,
                 initrd=False, profile=None, path=None,
                 property=None,
                 log=None):
        ExtractBase.__init__(self, stream,
                             initrd=initrd, profile=profile,
                             path=path,
                             property=property,
                             log=log)

    def _handleParsed(self):
        start = self.dataProp.offset
        self.log.debug("first byte is %d", start)
        end = start + self.dataProp.sz - 1
        self.log.debug("data size is %d", self.dataProp.sz)
        self.log.debug("last byte is %d", end)
        sys.stdout.write("%s %s\n" % (start, end,))
        return 0

USAGE = """\
pyfit [OPTIONS] dump|extract ...
"""

EPILOG = """\
Payload for 'offset' and 'extract' is specified as a given
PROPERTY for a tree node at PATH.

Alternately, the initrd/ramdisk can be specified with '--initrd',
using the PROFILE machine configuration. If no PROFILE is specified,
the built-in default configuration from the FDT is used.
"""

DESC="""\
Extract or examine FIT file contents.
"""

DUMP_USAGE = """\
pyfit [OPTIONS] dump FIT-FILE
"""

EXTRACT_USAGE = """\
pyfit [OPTIONS] extract [OPTIONS] FIT-FILE
"""

EXTRACT_EPILOG = """\
Extracts payload to OUTPUT or to stdout if not specified.

Output can be optionally reformatted
as a NUL-terminated string ('--text'),
as a decimal number ('--number'),
as a UNIX timestamp ('--timestamp'),
or as hex data ('--hex').

Numbers and timestamps must be 4-byte payloads.
"""

OFFSET_USAGE = """\
pyfit [OPTIONS] offset [OPTIONS] FIT-FILE
"""

OFFSET_EPILOG = """\
Outputs the first and last byte offsets, inclusive, containing the
payload.
"""

class App:

    def __init__(self, log=None):
        self.log = log or logging.getLogger("pyfit")

    def run(self):

        ap = argparse.ArgumentParser(usage=USAGE,
                                     description=DESC,
                                     epilog=EPILOG)
        ap.add_argument('-q', '--quiet', action='store_true',
                        help="Suppress log messages")
        ap.add_argument('-v', '--verbose', action='store_true',
                        help="Add more logging")

        sp = ap.add_subparsers()

        apd = sp.add_parser('dump',
                            help="Dump tree structure",
                            usage=DUMP_USAGE)
        apd.set_defaults(mode='dump')
        apd.add_argument('fit-file', type=open,
                         help="FIT file")

        apx = sp.add_parser('extract',
                            help="Extract items",
                            usage=EXTRACT_USAGE,
                            epilog=EXTRACT_EPILOG)
        apx.set_defaults(mode='extract')
        apx.add_argument('fit-file', type=open,
                         help="FIT file")
        apx.add_argument('-o', '--output',
                         type=argparse.FileType('wb', 0),
                         help="File destination")
        apx.add_argument('--initrd', action="store_true",
                         help="Extract platform initrd")
        apx.add_argument('--profile', type=str,
                         help="Platform profile for initrd selection")
        apx.add_argument('--path', type=str,
                         help="Tree path to extract")
        apx.add_argument('--property', type=str,
                         help="Node property to extract")
        apx.add_argument('--text', action='store_true',
                         help="Format property as text")
        apx.add_argument('--numeric', action='store_true',
                         help="Format property as a number")
        apx.add_argument('--hex', action='store_true',
                         help="Format property as hex")
        apx.add_argument('--timestamp', action='store_true',
                         help="Format property as a date")

        apo = sp.add_parser('offset',
                            help="Extract item offset",
                            usage=OFFSET_USAGE,
                            epilog=OFFSET_EPILOG)
        apo.set_defaults(mode='offset')
        apo.add_argument('fit-file', type=open,
                         help="FIT file")
        apo.add_argument('--initrd', action="store_true",
                         help="Extract platform initrd")
        apo.add_argument('--profile', type=str,
                         help="Platform profile for initrd selection")
        apo.add_argument('--path', type=str,
                         help="Tree path to extract")
        apo.add_argument('--property', type=str,
                         help="Node property to extract")

        try:
            args = ap.parse_args()
        except SystemExit, what:
            return what.code

        if args.quiet:
            self.log.setLevel(logging.ERROR)
        if args.verbose:
            self.log.setLevel(logging.DEBUG)

        if args.mode == 'dump':
            r = DumpRunner(getattr(args, 'fit-file'), log=self.log)
        elif args.mode == 'extract':
            r = ExtractRunner(getattr(args, 'fit-file'),
                              outStream=args.output,
                              path=args.path,
                              initrd=args.initrd, profile=args.profile,
                              property=args.property,
                              text=args.text, numeric=args.numeric,
                              timestamp=args.timestamp, hex=args.hex,
                              log=self.log)
        elif args.mode == 'offset':
            r = OffsetRunner(getattr(args, 'fit-file'),
                             path=args.path,
                             initrd=args.initrd, profile=args.profile,
                             property=args.property,
                             log=self.log)
        else:
            self.log.error("invalid mode")
            return 1

        try:
            code = r.run()
        except:
            self.log.exception("runner failed")
            code = 1
        r.shutdown()
        return code

    def shutdown(self):
        pass

    @classmethod
    def main(cls):
        logging.basicConfig()
        logger = logging.getLogger("pyfit")
        app = cls(log=logger)
        try:
            code = app.run()
        except:
            logger.exception("app failed")
            code = 1
        app.shutdown()
        sys.exit(code)

main = App.main

if __name__ == "__main__":
    main()

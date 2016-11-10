"""Legacy.py

See
http://www.isysop.com/unpacking-and-repacking-u-boot-uimage-files/
https://github.com/lentinj/u-boot/blob/master/include/image.h

"""

import os, sys
import logging
import struct
import argparse
import time

class Parser:

    MAGIC = 0x27051956

    # codes for ih_os
    IH_OS_INVALID         = 0
    IH_OS_OPENBSD         = 1
    IH_OS_NETBSD          = 2
    IH_OS_FREEBSD         = 3
    IH_OS_4_4BSD          = 4
    IH_OS_LINUX           = 5
    IH_OS_SVR4            = 6
    IH_OS_ESIX            = 7
    IH_OS_SOLARIS         = 8
    IH_OS_IRIX            = 9
    IH_OS_SCO             = 10
    IH_OS_DELL            = 11
    IH_OS_NCR             = 12
    IH_OS_LYNXOS          = 13
    IH_OS_VXWORKS         = 14
    IH_OS_PSOS            = 15
    IH_OS_QNX             = 16
    IH_OS_U_BOOT          = 17
    IH_OS_RTEMS           = 18
    IH_OS_ARTOS           = 19
    IH_OS_UNITY           = 20
    IH_OS_INTEGRITY       = 21
    IH_OS_OSE             = 22
    _IH_OS_END            = 23

    # codes for ih_arch
    IH_ARCH_INVALID           = 0
    IH_ARCH_ALPHA             = 1
    IH_ARCH_ARM               = 2
    IH_ARCH_I386              = 3
    IH_ARCH_IA64              = 4
    IH_ARCH_MIPS              = 5
    IH_ARCH_MIPS64            = 6
    IH_ARCH_PPC               = 7
    IH_ARCH_S390              = 8
    IH_ARCH_SH                = 9
    IH_ARCH_SPARC             = 10
    IH_ARCH_SPARC64           = 11
    IH_ARCH_M68K              = 12
    IH_ARCH_MICROBLAZE        = 14
    IH_ARCH_NIOS2             = 15
    IH_ARCH_BLACKFIN          = 16
    IH_ARCH_AVR32             = 17
    IH_ARCH_ST200             = 18
    IH_ARCH_SANDBOX           = 19
    IH_ARCH_NDS32             = 20
    IH_ARCH_OPENRISC          = 21
    _IH_ARCH_END              = 22

    # codes for ih_type
    IH_TYPE_INVALID = 0
    IH_TYPE_STANDALONE = 1
    IH_TYPE_KERNEL = 2
    IH_TYPE_RAMDISK = 3
    IH_TYPE_MULTI = 4
    IH_TYPE_FIRMWARE = 5
    IH_TYPE_SCRIPT = 6
    IH_TYPE_FILESYSTEM = 7
    IH_TYPE_FLATDT = 8
    IH_TYPE_KWBIMAGE = 9
    IH_TYPE_IMXIMAGE = 10
    IH_TYPE_UBLIMAGE = 11
    IH_TYPE_OMAPIMAGE = 12
    IH_TYPE_AISIMAGE =13
    IH_TYPE_KERNEL_NOLOAD = 14
    _IH_TYPE_END = 15

    # codes for ih_comp
    IH_COMP_NONE = 0
    IH_COMP_GZIP = 1
    IH_COMP_BZIP2 = 2
    IH_COMP_LZMA = 3
    IH_COMP_LZO = 4
    _IH_COMP_END = 5

    @classmethod
    def registerConstants(cls):

        cls.IH_OS = [None] * cls._IH_OS_END
        for k, v in cls.__dict__.iteritems():
            if k.startswith('IH_OS_'):
                cls.IH_OS[v] = k[6:]

        cls.IH_ARCH = [None] * cls._IH_ARCH_END
        for k, v in cls.__dict__.iteritems():
            if k.startswith('IH_ARCH_'):
                cls.IH_ARCH[v] = k[8:]

        cls.IH_TYPE = [None] * cls._IH_TYPE_END
        for k, v in cls.__dict__.iteritems():
            if k.startswith('IH_TYPE_'):
                cls.IH_TYPE[v] = k[8:]

        cls.IH_COMP = [None] * cls._IH_COMP_END
        for k, v in cls.__dict__.iteritems():
            if k.startswith('IH_COMP_'):
                cls.IH_COMP[v] = k[8:]

    def __init__(self, path=None, stream=None, log=None):

        self.log = log or logging.getLogger(self.__class__.__name__)
        self.path = path
        self.stream = stream

        self.images = []
        self._parse()

    @classmethod
    def isLegacy(cls, path=None, stream=None):
        if stream is not None:
            try:
                pos = stream.tell()
                buf = stream.read(4)
            finally:
                stream.seek(pos, 0)
        else:
            with open(path) as fd:
                buf = fd.read(4)
        if len(buf) != 4: return False
        magic = struct.unpack(">I", buf)[0]
        return magic == cls.MAGIC

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

        buf = fd.read(64)
        hdr = list(struct.unpack(">7IBBBB32s", buf))

        self.ih_magic = hdr.pop(0)
        if self.ih_magic != self.MAGIC:
            raise ValueError("missing or invalid magic")

        self.ih_hcrc = hdr.pop(0)

        self.ih_time = hdr.pop(0)
        self.log.debug("image created %s",
                       time.ctime(self.ih_time))

        self.ih_size = hdr.pop(0)
        self.log.debug("total image size %s bytes",
                       self.ih_size)

        self.ih_load = hdr.pop(0)
        self.ih_ep = hdr.pop(0)
        self.ih_dcrc = hdr.pop(0)

        self.ih_os = hdr.pop(0)
        if self.ih_os != self.IH_OS_LINUX:
            raise ValueError("invalid OS code")

        self.ih_arch = hdr.pop(0)

        self.ih_type = hdr.pop(0)
        if self.ih_type != self.IH_TYPE_MULTI:
            raise ValueError("invalid image type")

        self.ih_comp = hdr.pop(0)
        # compression is ignored here, since it applies to the first
        # image (the kernel)

        self.ih_name = hdr.pop(0).rstrip('\0')

        if self.ih_type == self.IH_TYPE_MULTI:
            self.images = []
            while True:
                buf = fd.read(4)
                sz = struct.unpack(">I", buf)[0]
                if sz == 0: break
                self.log.debug("found image header %d bytes", sz)
                self.images.append([sz, None])

        # compute absolute image offsets
        pos = fd.tell()
        for idx, rec in enumerate(self.images):
            rec[1] = pos
            pos += rec[0]

            # images are aligned at 4-byte boundaries
            pos += 3
            pos &= ~0x3

        return

Parser.registerConstants()

class DumpRunner:

    def __init__(self, stream, log=None):
        self.log = log or logging.getLogger(self.__class__.__name__)
        self.stream = stream

    def run(self):
        p = Parser(stream=self.stream, log=self.log)

        sys.stdout.write("Legacy U-Boot Image \"%s\":\n" % p.ih_name)
        sys.stdout.write("created %s, %d bytes\n"
                         % (time.ctime(p.ih_time), p.ih_size,))
        sys.stdout.write("load @0x%04x, execute @0x%04x\n"
                         % (p.ih_load, p.ih_ep,))
        sys.stdout.write("OS is %s\n" % p.IH_OS[p.ih_os])
        sys.stdout.write("architecture is %s\n" % p.IH_ARCH[p.ih_arch])
        sys.stdout.write("image type is %s\n" % p.IH_TYPE[p.ih_type])
        sys.stdout.write("compression type is %s\n" % p.IH_COMP[p.ih_comp])

        if p.ih_type == p.IH_TYPE_MULTI:
            sys.stdout.write("%d images total:\n" % len(p.images))
            for i, rec in enumerate(p.images):
                sys.stdout.write("image %d, %d bytes (offset %d)\n"
                                 % (i, rec[0], rec[1],))

        return 0

    def shutdown(self):
        strm, self.stream = self.stream, None
        if strm is not None: strm.close()

class ExtractRunner:
    """Extract a specific image.

    NOTE that image zero may be compressed.
    """

    def __init__(self, stream, index=None, outStream=None, log=None):
        self.log = log or logging.getLogger(self.__class__.__name__)
        self.stream = stream
        self.index = index
        self.outStream = outStream

    def run(self):
        p = Parser(stream=self.stream, log=self.log)

        if p.ih_type != p.IH_TYPE_MULTI:
            if self.index is not None:
                self.log.error("not a multi-file image, image index not allowed")
                return 1
            self.stream.seek(64, 0)
            buf = self.stream.read(p.ih_size)
        else:
            if self.index is None:
                self.log.error("multi-file image, image index required")
                return 1
            sz, off = p.images[self.index]
            self.stream.seek(off, 0)
            buf = self.stream.read(sz)

        strm = self.outStream or sys.stdout
        strm.write(buf)
        
        return 0

    def shutdown(self):
        strm, self.stream = self.stream, None
        if strm is not None: strm.close()
        strm, self.outStream = self.outStream, None
        if strm is not None: strm.close()

USAGE = """\
pylegacy [OPTIONS] ...
"""

DUMP_USAGE = """\
pylegacy [OPTIONS] dump IMAGE-FILE
"""

EXTRACT_USAGE = """\
pylegacy [OPTIONS] extract [OPTIONS] IMAGE-FILE [IMAGE-INDEX]
"""

class App:

    def __init__(self, log=None):
        self.log = log or logging.getLogger("pyfit")

    def run(self):

        ap = argparse.ArgumentParser(usage=USAGE)
        ap.add_argument('-q', '--quiet', action='store_true',
                        help="Suppress log messages")
        ap.add_argument('-v', '--verbose', action='store_true',
                        help="Add more logging")

        sp = ap.add_subparsers()

        apd = sp.add_parser('dump',
                            help="Dump image structure",
                            usage=DUMP_USAGE)
        apd.set_defaults(mode='dump')
        apd.add_argument('image-file', type=open,
                         help="U-Boot Legacy Image")

        apx = sp.add_parser('extract',
                            help="Extract items",
                            usage=EXTRACT_USAGE)
        apx.set_defaults(mode='extract')
        apx.add_argument('image-file', type=open,
                         help="U-Boot Legacy Image")

        apx.add_argument('-o', '--output',
                         type=argparse.FileType('wb', 0),
                         help="File destination")
        apx.add_argument('index', type=int, nargs='?',
                         help="Image index (zero-based)")

        try:
            args = ap.parse_args()
        except SystemExit, what:
            return what.code

        if args.quiet:
            self.log.setLevel(logging.ERROR)
        if args.verbose:
            self.log.setLevel(logging.DEBUG)

        if args.mode == 'dump':
            strm = getattr(args, 'image-file')
            r = DumpRunner(strm, log=self.log)
        elif args.mode == 'extract':
            strm = getattr(args, 'image-file')
            r = ExtractRunner(strm,
                              index=args.index,
                              outStream=args.output,
                              log=self.log)

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
        logger = logging.getLogger("pylegacy")
        logger.setLevel(logging.INFO)
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

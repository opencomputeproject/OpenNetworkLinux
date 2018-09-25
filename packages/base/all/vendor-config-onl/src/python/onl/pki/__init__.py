#!/usr/bin/python
############################################################
#
# ONL PKI Management
#
############################################################
import sys
import os
import argparse
import logging
import shutil
import tempfile
from onl.mounts import OnlMountManager, OnlMountContextReadOnly, OnlMountContextReadWrite
from onl.sysconfig import sysconfig
from onl.util import *



class OnlPkiContextReadWrite(OnlMountContextReadWrite):
    def __init__(self, logger):
        OnlMountContextReadWrite.__init__(self, "ONL-CONFIG", logger)

class OnlPkiContextReadOnly(OnlMountContextReadOnly):
    def __init__(self, logger):
        OnlMountContextReadOnly.__init__(self, "ONL-CONFIG", logger)


class OnlPki(OnlServiceMixin):
    """Initialize the ONL-CONFIG::PKI credentials."""

    CONFIG_PKI_DIR="/mnt/onl/config/pki"

    def __init__(self, logger=None):
        self.logger = logger

        if self.logger is None:
            self.logger = logging.getLogger("ONL:PKI")

        self.kpath = os.path.join(self.CONFIG_PKI_DIR,
                                  sysconfig.pki.key.name)

        self.cpath = os.path.join(self.CONFIG_PKI_DIR,
                             sysconfig.pki.cert.name)

    def init(self, force=False):
        self.init_key(force=force)
        self.init_cert(force=force)

    def init_key(self, force=False):
        need_key = False
        need_cert = False

        if force:
            need_key = True
        else:
            with OnlPkiContextReadOnly(self.logger):
                if not os.path.exists(self.kpath):
                    need_key = True

        if need_key:
            self.logger.info("Generating private key...")
            cmd = ('openssl', 'genrsa',
                   '-out', self.kpath,
                   str(sysconfig.pki.key.len),)
            with OnlPkiContextReadWrite(self.logger):
                if not os.path.isdir(self.CONFIG_PKI_DIR):
                    os.makedirs(self.CONFIG_PKI_DIR)
                self._execute(cmd, logLevel=logging.INFO)
            need_cert = True
        else:
            self.logger.info("Using existing private key.")

        if need_cert:
            self.init_cert(force=True)

    def init_cert(self, force=False):
        need_cert = False

        if force:
            need_cert = True
        else:
            with OnlPkiContextReadOnly(self.logger):
                if not os.path.exists(self.cpath):
                    need_cert = True

        if need_cert:
            self.logger.info("Generating self-signed certificate...")
            csr = tempfile.NamedTemporaryFile(prefix="pki-", suffix=".csr", delete=False)
            csr.close()
            fields = [ "%s=%s" % (k, v) for k,v in sysconfig.pki.cert.csr.fields.iteritems() ]
            subject = "/" + "/".join(fields)
            self.logger.debug("Subject: '%s'", subject)
            self.logger.debug("CSR: %s", csr.name)
            with OnlPkiContextReadWrite(self.logger):
                if not os.path.isdir(self.CONFIG_PKI_DIR):
                    os.makedirs(self.CONFIG_PKI_DIR)
                self._execute(('openssl', 'req',
                               '-new', '-batch',
                               '-subj', subject,
                               '-key', self.kpath,
                               '-out', csr.name,),
                              logLevel=logging.INFO)
                self._execute(('openssl', 'x509',
                               '-req',
                               '-days', str(sysconfig.pki.cert.csr.cdays),
                               '-sha256',
                               '-in', csr.name,
                               '-signkey', self.kpath,
                               '-out', self.cpath,),
                              logLevel=logging.INFO)
            os.unlink(csr.name)
        else:
            self.logger.info("Using existing certificate.")

    @staticmethod
    def main():
        ap = argparse.ArgumentParser(description="ONL PKI Management")
        ap.add_argument("--init",          action='store_true', help="Initialize  PKI files (if necessary)")
        ap.add_argument("--regen-cert",    action='store_true', help="Regenerate certificate.")
        ap.add_argument("--force",   "-f", action='store_true', help="Force regeneration of the key and certificate during initialization (--init)")
        ap.add_argument("--quiet",   "-q", action='store_true', help="Quiet output.")
        ap.add_argument("--verbose", "-v", action='store_true', help="Verbose output.")

        ops = ap.parse_args()

        logging.basicConfig()
        logger = logging.getLogger("PKI")

        if ops.verbose:
            logger.setLevel(logging.DEBUG)
        elif ops.quiet:
            logger.setLevel(logging.WARNING)
        else:
            logger.setLevel(logging.INFO)

        pki = OnlPki(logger)

        if ops.init:
            pki.init_key(force=ops.force)
            pki.init_cert(force=ops.force)
        elif ops.regen_cert:
            pki.init_cert(force=True)

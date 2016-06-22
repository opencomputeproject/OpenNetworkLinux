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
import tempfile
import shutil
import subprocess
import tempfile
import yaml
from onl.mounts import OnlMountManager, OnlMountContextReadOnly, OnlMountContextReadWrite
from onl.sysconfig import sysconfig
from onl.util import *

class OnlPki(OnlServiceMixin):
    """Initialize the ONL-CONFIG::PKI credentials."""

    CONFIG_PKI_DIR="/mnt/onl/config/pki"

    def __init__(self, logger):
        self.logger = logger
        self.kpath = os.path.join(self.CONFIG_PKI_DIR,
                                  sysconfig.pki.key.name)

        self.cpath = os.path.join(self.CONFIG_PKI_DIR,
                             sysconfig.pki.cert.name)

    def init_key(self, force=False):
        with OnlMountContextReadOnly("ONL-CONFIG", self.logger):
            if not os.path.exists(self.kpath) or force:
                self.logger.info("Generating private key...")
                cmd = "openssl genrsa -out %s %s" % (self.kpath, sysconfig.pki.key.len)
                with OnlMountContextReadWrite("ONL-CONFIG", self.logger):
                    if not os.path.isdir(self.CONFIG_PKI_DIR):
                        os.makedirs(self.CONFIG_PKI_DIR)
                    self._execute(cmd)
                self.init_cert(force=True)
            else:
                self.logger.info("Using existing private key.")

    def init_cert(self, force=False):
        with OnlMountContextReadOnly("ONL-CONFIG", self.logger):
            if not os.path.exists(self.cpath) or force:
                self.logger.info("Generating self-signed certificate...")
                csr = tempfile.NamedTemporaryFile(prefix="pki-", suffix=".csr", delete=False)
                csr.close()
                fields = [ "%s=%s" % (k, v) for k,v in sysconfig.pki.cert.csr.fields.iteritems() ]
                subject = "/" + "/".join(fields)
                self.logger.debug("Subject: '%s'", subject)
                self.logger.debug("CSR: %s", csr.name)
                with OnlMountContextReadWrite("ONL-CONFIG", self.logger):
                    if not os.path.isdir(self.CONFIG_PKI_DIR):
                        os.makedirs(self.CONFIG_PKI_DIR)
                    self._execute("""openssl req -new -batch -subj "%s" -key %s -out %s""" % (
                            subject, self.kpath, csr.name))
                    self._execute("""openssl x509 -req -days %s -sha256 -in %s -signkey %s -out %s""" % (
                            sysconfig.pki.cert.csr.cdays,
                            csr.name, self.kpath, self.cpath))
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

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

from onl.mounts import OnlDataStore

class OnlPKI(OnlDataStore):
    P_DIR='private'
    R_DIR='/private'

    PRIVATE_KEY='key.pem'
    CERTIFICATE='certificate'

    KLEN=2048
    CDAYS=3650

    CSR_FILE='/etc/onl/config/csr.yml'

    def __init__(self, logger=None):
        OnlDataStore.__init__(self, logger)
        self.kpath = os.path.join(self.R_DIR, self.PRIVATE_KEY)
        self.cpath = os.path.join(self.R_DIR, self.CERTIFICATE)

    def init_cert(self, force=False):
        if not os.path.exists(self.cpath) or force:
            self.logger.info("Generating self-signed certificate...")

            #
            # The csr.yml file allows the system integrator to customize
            # the fields for the certificate.
            #
            fdict = {}
            try:
                fdict = yaml.load(open(self.CSR_FILE))
            except Exception, e:
                self.logger.error(e);

            csr = tempfile.NamedTemporaryFile(prefix="pki-", suffix=".csr", delete=False)
            csr.close()

            fields = [ "%s=%s" % (k, v) for k,v in fdict.iteritems() ]
            subject = "/" + "/".join(fields)
            self.logger.debug("Subject: '%s'", subject)
            self.logger.debug("CSR: %s", csr.name)
            self._execute("""openssl req -new -batch -subj "%s" -key %s -out %s""" % (
                    subject, self.kpath, csr.name))
            self._execute("""openssl x509 -req -days %s -in %s -signkey %s -out %s""" % (
                    self.CDAYS,
                    csr.name, self.kpath, self.cpath))
            os.unlink(csr.name)
        else:
            self.logger.info("Using existing certificate.")

    def init_key(self, force=False):
        if not os.path.exists(self.kpath) or force:
            self.logger.info("Generating private key...")
            cmd = "openssl genrsa -out %s %s" % (self.kpath, self.KLEN)
            self._execute(cmd)
            self.init_cert(force=True)
        else:
            self.logger.info("Using existing private key.")

    def init(self, force=False):
        self.init_key(force=force)
        self.init_cert(force=force)
        self.commit_runtime()


    @staticmethod
    def main():
        ap = argparse.ArgumentParser(description="ONL PKI Management")
        ap.add_argument("--init",          action='store_true', help="Initialize /private and PKI files if necessary.")
        ap.add_argument("--regen-cert",    action='store_true', help="Regenerate certificate.")
        ap.add_argument("--force",   "-f", action='store_true', help="Force regeneration of the key and certificate during initialization (--init)")
        ap.add_argument("--commit",        action='store_true', help="Commit the runtime /private directory to the persistant storage.")
        ap.add_argument("--ls",            action='store_true', help="List contents of the peristant directory.")
        ap.add_argument("--quiet",   "-q", action='store_true', help="Quiet output.")
        ap.add_argument("--verbose", "-v", action='store_true', help="Verbose output.")
        ap.add_argument("--part",                               help='Override Data Partition (testing only).')
        ap.add_argument("--rd",                                 help='Override /private runtime directory (testing only)')

        ops = ap.parse_args()

        logging.basicConfig()
        logger = logging.getLogger("PKI")

        if ops.verbose:
            logger.setLevel(logging.DEBUG)
        elif ops.quiet:
            logger.setLevel(logging.WARNING)
        else:
            logger.setLevel(logging.INFO)

        if ops.part:
            OnlPKI.DATA_PARTITION=ops.part

        if ops.rd:
            OnlPKI.R_DIR=ops.rd

        pki = OnlPKI(logger)

        if ops.init:
            pki.init_runtime()
            pki.init_key(force=ops.force)
            pki.init_cert(force=ops.force)
            pki.commit_runtime()

        elif ops.regen_cert:
            pki.init_cert(force=True)
            pki.commit_runtime()

        elif ops.commit:
            pki.commit_runtime()
        elif ops.ls:
            pki.ls()
        else:
            pki.diff()

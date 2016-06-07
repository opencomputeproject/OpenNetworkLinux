"""RecoverApp.py

Application-level code for Switch Light recovery.
"""

import os, sys
import imp
import logging
from ConfUtils import UbootEnv

class App:

    def __init__(self, log=None):

        if log is not None:
            self.log = log
        else:
            self.log = logging.getLogger(self.__class__.__name__)

        self.recovery = None

    def run(self):

        if os.path.exists(UbootEnv.SETENV):
            self.ubootEnv = UbootEnv(log=self.log.getChild("u-boot"))
        else:
            self.ubootEnv = None

        # load the platform-specific blob
        if os.path.exists("/etc/onl/platform"):
            with open("/etc/onl/platform") as fd:
                plat = fd.read().strip()
        else:
            self.log.error("cannot recover non-ONL platform")
            return 1

        p = ("/lib/platform-config/%s/python/recover.py"
             % (plat,))
        if not os.path.exists(p):
            self.log.error("missing recover profile %s", p)
            return 1
        mod = imp.load_source("platform_recover", p)

        # run the platform-specific installer
        self.recovery = mod.Recovery(ubootEnv=self.ubootEnv,
                                     log=self.log)
        try:
            code = self.recovery.run()
        except:
            self.log.exception("recovery failed")
            code = 1
        if code: return code

        return 0

    def shutdown(self):

        recovery, self.recovery = self.recovery, None
        if recovery is not None:
            recovery.shutdown()

    @classmethod
    def main(cls):

        logging.basicConfig()
        logger = logging.getLogger("recover")
        logger.setLevel(logging.DEBUG)

        app = cls(log=logger)
        try:
            code = app.run()
        except:
            logger.exception("runner failed")
            code = 1
        app.shutdown()
        sys.exit(code)

main = App.main

if __name__ == "__main__":
    main()

import subprocess

class OnlServiceMixin(object):
    def _execute(self, cmd, root=False, ex=True):
        self.logger.debug("Executing: %s" % cmd)
        if root is True and os.getuid() != 0:
            cmd = "sudo " + cmd
        try:
            subprocess.check_call(cmd, shell=True)
        except Exception, e:
            if ex:
                self.logger.error("Command failed: %s" % e)
                raise
            else:
                return e.returncode

    def _raise(self, msg, klass):
        self.logger.critical(msg)
        raise klass(msg)


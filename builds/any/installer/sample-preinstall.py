"""sample-preinstall.py

Example Python script for pre-install hooks.

Add this as a preinstall hook to your installer via
the 'mkinstaller.py' command line:

$ mkinstaller.py ... --preinstall-plugin sample-preinstall.py ...

At install time, this script will

1. be extracted into a temporary working directory
2. be imported as a module, in the same process as the installer
   script

Importing the module should not trigger any side-effects.

At the appropriate time during the install (a chrooted invocation
of the installer Python script) will

1. scrape the top-level plugin's namespace for subclasses of
   onl.install.Plugin.Plugin.
   Implementors should declare classes here
   (inheriting from onl.install.Plugin.Plugin) to embed the plugin
   functionality.
2. instantiate an instance of each class, with the installer
   object initialized as the 'installer' attribute
3. invoke the 'run' method (which must be overridden by implementors)
   For a pre-install plugin, the 'mode' argument is set to
   PLUGIN_PREINSTALL.
4. invoke the 'shutdown' method (by default, a no-op)

The 'run' method should return zero on success. In any other case, the
installer terminates.

The 'installer' object has a handle onto the installer ZIP archive
(self.installer.zf) but otherwise the install has not been
started. That is, the install disk has not been
prepped/initialized/scanned yet. As per the ONL installer API, the
installer starts with *no* filesystems mounted, not even the ones from
a prior install.

A pre-install plugin should execute any pre-install actions when
'mode' is set to PLUGIN_PREINSTALL. If 'mode' is set to any other
value, the plugin should ignore it and return zero. The plugin run()
method is invoked multiple times during the installer with different
values of 'mode'. The 'shutdown()' method is called only once.

"""

import onl.install.Plugin

class Plugin(onl.install.Plugin.Plugin):

    def run(self, mode):

        if mode == self.PLUGIN_PREINSTALL:
            self.log.info("hello from preinstall plugin")
            return 0

        return 0

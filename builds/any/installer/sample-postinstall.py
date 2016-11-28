"""sample-postinstall.py

Example Python script for post-install hooks.

Add this as a postinstall hook to your installer via
the 'mkinstaller.py' command line:

$ mkinstaller.py ... --plugin sample-postinstall.py ...

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
   For a post-install plugin, the 'mode' argument is set to
   PLUGIN_POSTINSTALL.
4. invoke the 'shutdown' method (by default, a no-op)

The 'run' method should return zero on success. In any other case, the
installer terminates.

The post-install plugins are invoked after the installer is complete
and after the boot loader is updated.

An exception to this is for proxy GRUB configurations. In that case, the
post-install plugins are invoked after the install is finished, but before
the boot loader has been updated.

At the time the post-install plugin is invoked, none of the
filesystems are mounted. If the implementor needs to manipulate the
disk, the filesystems should be re-mounted temporarily with
e.g. MountContext. The OnlMountContextReadWrite object and their
siblings won't work here because the mtab.yml file is not populated
within the loader environment.

A post-install plugin should execute any post-install actions when
'mode' is set to PLUGIN_POSTINSTALL. If 'mode' is set to any other
value, the plugin should ignore it and return zero. The plugin run()
method is invoked multiple times during the installer with different
values of 'mode'. The 'shutdown()' method is called only once.

When using MountContxt, the system state in the installer object can help
(self.installer.blkidParts in particular).

"""

import onl.install.Plugin

class Plugin(onl.install.Plugin.Plugin):

    def run(self, mode):

        if mode == self.PLUGIN_POSTINSTALL:
            self.log.info("hello from postinstall plugin")
            return 0

        return 0

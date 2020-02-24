#!/usr/bin/python2
############################################################
#
# ONL Package Management
#
############################################################
import argparse
import os
import sys
import logging
import yaml
import tempfile
import shutil
import pprint
import subprocess
import glob
import submodules
import onlyaml
import onlu
from string import Template
import re
import json
import lsb_release
import cPickle as pickle

g_dist_codename = lsb_release.get_distro_information().get('CODENAME')

logger = onlu.init_logging('onlpm', logging.INFO)

class OnlPackageError(Exception):
    """General Package Error Exception

    This class is used to communicate user-level configuration
    and runtime errors related to package contents, package
    building, and similar operations.

    This exception should be caught at the highest level and
    the error message comminicated to the user."""

    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


class OnlPackageMissingError(OnlPackageError):
    def __init__(self, pkg):
        self.value = "Package %s does not exist." % pkg


class OnlPackageMissingFileError(OnlPackageError):
    def __init__(self, p, f):
        self.value = "Package %s does not contain the file %s." % (p, f)

class OnlPackageMissingDirError(OnlPackageError):
    def __init__(self, p, d):
        self.value = "Package %s does not contain the directory %s." % (p, d)

class OnlPackageServiceScript(object):
    SCRIPT=None
    def __init__(self, service, dir=None):
        if self.SCRIPT is None:
            raise AttributeError("The SCRIPT attribute must be provided by the deriving class.")

        with tempfile.NamedTemporaryFile(dir=dir, delete=False) as f:
            f.write(self.SCRIPT % dict(service=os.path.basename(service.replace(".init", ""))))
            self.name = f.name


class OnlPackageAfterInstallScript(OnlPackageServiceScript):
    SCRIPT = """#!/bin/sh
if [ -x "/etc/init.d/%(service)s" ]; then
    if [ -x "/usr/sbin/policy-rc.d" ]; then
        /usr/sbin/policy-rc.d
        if [ $? -eq 101 ]; then
            echo "warning: service %(service)s: postinst: ignored due to policy-rc.d"
            exit 0;
        fi
    fi
    set -e
    update-rc.d %(service)s defaults >/dev/null
    invoke-rc.d %(service)s start || exit $?
fi
"""

class OnlPackageBeforeRemoveScript(OnlPackageServiceScript):
    SCRIPT = """#!/bin/sh
set -e
if [ -x "/etc/init.d/%(service)s" ]; then
    invoke-rc.d %(service)s stop || exit $?
fi
"""

class OnlPackageAfterRemoveScript(OnlPackageServiceScript):
    SCRIPT = """#!/bin/sh
set -e
if [ "$1" = "purge" ] ; then
    update-rc.d %(service)s remove >/dev/null
fi
"""


class OnlPackage(object):
    """Individual Debian Package Builder Class

    This class builds a single debian package from a package specification.
    The package specification is a dictionary with the following keys:

      name:         The name of the package
      version:      The version of the package
      arch:         The package architecture
      copyright:    The copyright string or path to copyright file
      changelog:    The changelog string or path to changelog file
      maintainer:   The package maintainer address
      summary:      The package summary description
      *desc:        The package description (defaults to summary)
      files:        A dict containing source/dst pairs.
                       A src can be a file or a directory
                       A dst can be a file or a directory
                    A list containing src,dst pairs.
      *depends:     List of package dependencies
      *docs :       List of documentation files

    Keys marked with an asterisk are optional. All others are required."""

    # These are built-in defaults for some package keys
    DEFAULTS = {
        'vendor' : 'Open Network Linux',
        'url' : 'http://opennetlinux.org',
        'license' : 'unknown',

        # Default Python Package Installation
        'PY_INSTALL' : '/usr/lib/python2.7/dist-packages',

        # Default Builder build directory name. Must match setup.env
        'BUILD_DIR' : 'BUILD/%s' % g_dist_codename,

        # Default Templates Location
        'ONL_TEMPLATES' : "%s/packages/base/any/templates" % os.getenv("ONL"),

        # Default Distribution
        'DISTS' : g_dist_codename,
        }

    @classmethod
    def package_defaults_get(klass, pkg):
        # Default key value dictionary
        ddict = klass.DEFAULTS.copy()

        #
        # Walk up the directory heirarchy looking for either:
        #
        # [.]PKG_DEFAULTS.yml -- an onlyaml file containing default package keys.
        # [.]PKG_DEFAULTS -- An executable producing yaml containing default package keys.
        #
        # All matches are evaluated and applied in reverse order such that
        # keys deeper in the directory heirarchy override shallower keys.
        #
        try:
            searchdir = os.path.dirname(pkg)
            results = []
            while searchdir != '/':
                for prefix in [ '', '.']:
                    f = os.path.join(searchdir, "%sPKG_DEFAULTS.yml" % prefix)
                    if os.path.exists(f):
                        results.append(onlyaml.loadf(f))
                    f = os.path.join(searchdir, "%sPKG_DEFAULTS" % prefix)
                    if os.path.exists(f) and os.access(f, os.X_OK):
                        results.append(yaml.load(subprocess.check_output(f, shell=True)))
                searchdir = os.path.dirname(searchdir)

            for d in reversed(results):
                if d:
                    ddict.update(d)
        except Exception, e:
            sys.stderr.write("%s\n" % e)
            sys.stderr.write("package file: %s\n" % pkg)
            raise

        return ddict

    ############################################################
    #
    # 'pdict' is the main package dictionary.
    # 'cdict' is the "common" key dictionary (optional)
    # 'ddict' is the "default" key dictionary
    #
    # The union of 'pdict' + 'cdict' + 'ddict' must provide a complete
    # package specification containing all required keys.
    #
    # 'dir_' is the parent directory of the package.
    #        File contents for the package specification may be absolute or
    #        relative to this directory.
    #
    ############################################################
    def __init__(self, pdict, dir_, cdict=None, ddict=None):
        """Initialize package object.

        'pdict' : The main package dictionary
        'cdict' : The "common" key dictionary (optional)
        'ddict' : The "default" key dictionary
        'dir_'  : The parent directory of the package.
                  File contents for the package specification may be absolute or
                  relative to this directory.

        The union of 'pdict' + 'cdict' + 'ddict' must provide a complete
        package specification containing all required keys."""

        # Optional common value dictionary
        if cdict is None:
            cdict = {}

        #
        # The key value precedence is package dict, common dict, default dict.
        #
        self.pkg = dict(ddict.items() + cdict.items() + pdict.items())

        # Validate all required package keys are present and well-formed.
        if not 'external' in self.pkg:
            self._validate()

        # Save package working directory for future build steps.
        self.dir = dir_



    def id(self):
        """Cannonical package identifier.

        The package identifier is <basename>:<arch>

        The version numbers and suffixes related to the package are ignored
        for the purposes of the identifier. It is assumed that any given
        set of packages used in creating a particular system will only
        be at a single revision at a time."""

        return self.pkg['name'] + ':' + self.pkg['arch']

    def arch(self):
        return self.pkg['arch']

    @staticmethod
    def idparse(pkgid, ex=True):
        """Parse a cannonical package identifier.

        pkgid: string of the form <name>:<arch>
        ex: If true, a package expection is raised if it cannot be parsed.

        Returns a tuple of the form (name, arch), or (False,False)"""
        try:
            # (name, arch)
            return tuple(pkgid.split(':'))
        except ValueError:
            if ex:
                raise OnlPackageError("%s is not a valid package specification." % pkg)
            else:
                return (False,False)


    def __str__(self):
        """Package key contents."""
        return pprint.pformat(self.pkg)


    def _validate_key(self, key, type_, required=True):
        """Validate a given key (option dependent)."""

        if key not in self.pkg:
            if required:
                raise OnlPackageError("'%s' is required but not specified in the package file." % key)
            else:
                return False

        if type(self.pkg[key]) is str and type_ is list:
            self.pkg[key] = [ self.pkg[key] ]

        if type(self.pkg[key]) in [ float , int ]:
            self.pkg[key] = str(self.pkg[key])

        if type(self.pkg[key]) != type_:
            raise OnlPackageError("key '%s' is the wrong type (%s should be %s)" % (
                    key, type(self.pkg[key]), type_))

        return True


    def _validate_files(self, key, required=True):
        """Validate the existence of the required input files for the current package."""
        self.pkg[key] = onlu.validate_src_dst_file_tuples(self.dir,
                                                          self.pkg[key],
                                                          dict(PKG=self.pkg['name'], PKG_INSTALL='/usr/share/onl/packages/%s/%s' % (self.pkg['arch'], self.pkg['name'])),
                                                          OnlPackageError,
                                                          required=required)
    def _validate(self):
        """Validate the package contents."""

        self._validate_key('name', str)
        self._validate_key('arch', str)
        self._validate_key('copyright', str, False)
        self._validate_key('changelog', str, False)
        self._validate_key('version', str)
        self._validate_key('maintainer', str)
        self._validate_key('depends', list, False)
        self._validate_key('priority', str, False)
        self._validate_key('summary', str)
        self._validate_key('docs', list, False)
        self._validate_key('vendor', str)
        self._validate_key('url', str, False)
        self._validate_key('license', str, False)

        if not self._validate_key('desc', str, False):
            self.pkg['desc'] = self.pkg['summary']

        self.pkg['description'] = "%s\n%s" % (self.pkg['summary'], self.pkg['desc'])

        return True

    @staticmethod
    def copyf(src, dst, root, symlinks=False):
        if dst.startswith('/'):
            dst = dst[1:]

        if os.path.isdir(src):
            #
            # Copy entire src directory to target directory
            #
            dstpath = os.path.join(root, dst)
            logger.debug("Copytree %s -> %s" % (src, dstpath))
            shutil.copytree(src, dstpath, symlinks=symlinks)
        else:
            #
            # If the destination ends in a '/' it means copy the filename
            # as-is to that directory.
            #
            # If not, its a full rename to the destination.
            #
            if dst.endswith('/'):
                dstpath = os.path.join(root, dst)
                try:
                    os.makedirs(dstpath)
                except OSError, e:
                    if e.errno != os.errno.EEXIST:
                        raise
                shutil.copy(src, dstpath)
            else:
                dstpath = os.path.join(root, os.path.dirname(dst))
                if not os.path.exists(dstpath):
                    os.makedirs(dstpath)
                shutil.copyfile(src, os.path.join(root, dst))
                shutil.copymode(src, os.path.join(root, dst))


    def build(self, dir_=None):
        """Build the debian package.

        When this method is called it is assumed that all file
        prerequisites for the package have already been built
        or met. This is simply the packaging stage.

        'dir_' : This is the output directory in which the package
        should be left. If unspecified the package's local directory
        will contain the package file."""


        if 'external' in self.pkg:
            # Package file built externally
            epkg = self.pkg['external']
            if os.path.exists(epkg):
                return epkg
            else:
                raise OnlPackageError("The external package file '%s' does not exist." % epkg);



        # Make sure all required files exist
        if 'files' in self.pkg:
            self._validate_files('files', True)

        if 'optional-files' in self.pkg:
            self._validate_files('optional-files', False)

        # If dir_ is not specified, leave package in local package directory.
        if dir_ is None:
            dir_ = self.dir

        workdir = tempfile.mkdtemp()
        root = os.path.join(workdir, "root");
        os.mkdir(root);

        # The package file will be built into the workdir
        self.pkg['__workdir'] = workdir

        for (src,dst) in self.pkg.get('files', {}):
            OnlPackage.copyf(src, dst, root, symlinks=self.pkg.get('symlinks', False))

        for (src,dst) in self.pkg.get('optional-files', {}):
            if os.path.exists(src):
                OnlPackage.copyf(src, dst, root)

        for (link, src) in self.pkg.get('links', {}).iteritems():
            logger.info("Linking %s -> %s..." % (link, src))
            # The source must be relative to the existing root directory.
            if link.startswith('/'):
                link = "%s%s" % (root, link)
            else:
                link = "%s/%s" % (root, link)
            # The link must be relative or absolute to the final filesystem.
            os.symlink(src, link)

        #
        # FPM doesn't seem to have a doc option so we copy documentation
        # files directly into place.
        #
        docpath = os.path.join(root, "usr/share/doc/%(name)s" % self.pkg)
        if not os.path.exists(docpath):
            os.makedirs(docpath)

        for src in self.pkg.get('docs', []):
            if not os.path.exists(src):
                raise OnlPackageError("Documentation source file '%s' does not exist." % src)
                shutil.copy(src, docpath)

        changelog = os.path.join(workdir, 'changelog')
        copyright_ = os.path.join(workdir, 'copyright')

        #
        # Export changelog and copyright files from the PKG dict
        # to the workdir for processing.
        #
        # The copyright and changelog data can be embedded directly
        # int the PKG file or kept as separate files.
        #

        def copy_str_or_file(src, dst):
            if os.path.exists(src):
                shutil.copyfile(src, dst)
            else:
                with open(dst, "w") as f:
                    f.write(src)
                    f.write("\n")

            copy_str_or_file(self.pkg['copyright'], copyright_)
            copy_str_or_file(self.pkg['changelog'], changelog)


        ############################################################
        #
        # Invoke fpm with all necessary options.
        #
        ############################################################
        self.pkg['__root'] = root

        command = """fpm -p %(__workdir)s -f -C %(__root)s -s dir -t deb -n %(name)s -v %(version)s -a %(arch)s -m %(maintainer)s --description "%(description)s" --url "%(url)s" --license "%(license)s" --vendor "%(vendor)s" """ % self.pkg

        for dep in self.pkg.get('depends', []):
            command = command + "-d %s " % dep

        for dep in self.pkg.get('build-depends', []):
            command = command + "--deb-build-depends %s " % dep

        for provides in onlu.sflatten(self.pkg.get('provides', [])):
            command = command + "--provides %s " % provides

        for conflicts in onlu.sflatten(self.pkg.get('conflicts', [])):
            command = command + "--conflicts %s " % conflicts

        for replaces in onlu.sflatten(self.pkg.get('replaces', [])):
            command = command + "--replaces %s " % replaces

        if 'virtual' in self.pkg:
            command = command + "--provides %(v)s --conflicts %(v)s --replaces %(v)s " % dict(v=self.pkg['virtual'])

        if 'priority' in self.pkg:
            command = command + "--deb-priority %s " % self.pkg['priority']

        if 'init' in self.pkg:
            if not os.path.exists(self.pkg['init']):
                raise OnlPackageError("Init script '%s' does not exist." % self.pkg['init'])
            command = command + "--deb-init %s " % self.pkg['init']
            if self.pkg.get('init-after-install', True):
                command = command + "--after-install %s " % OnlPackageAfterInstallScript(self.pkg['init'], dir=workdir).name
            if self.pkg.get('init-before-remove', True):
                command = command + "--before-remove %s " % OnlPackageBeforeRemoveScript(self.pkg['init'], dir=workdir).name
            if self.pkg.get('init-after-remove', True):
                command = command + "--after-remove %s " % OnlPackageAfterRemoveScript(self.pkg['init'], dir=workdir).name

        fpm_file_commands = ['{}-{}'.format(o, a) for o in ['after', 'before'] for a in ['install', 'remove', 'upgrade']]

        fpm_file_commands.append('deb-systemd')

        for cmd in fpm_file_commands:
            if cmd in self.pkg:
                if not os.path.exists(self.pkg[cmd]):
                    raise OnlPackageError("%s script '%s' does not exist." % (cmd, self.pkg[cmd]))
                command = command + "--%s %s " % (cmd, self.pkg[cmd])

        if self.pkg.get('asr', False):
            with onlu.Profiler() as profiler:
                # Generate the ASR documentation for this package.
                sys.path.append("%s/sm/infra/tools" % os.getenv('ONL'))
                import asr
                asro = asr.AimSyslogReference()
                asro.extract(workdir)
                asro.format(os.path.join(docpath, asr.AimSyslogReference.ASR_NAME), 'json')
            profiler.log("ASR generation for %(name)s" % self.pkg)
        ############################################################

        if logger.level < logging.INFO:
            command = command + "--verbose "

        onlu.execute(command)

        # Grab the package from the workdir. There can be only one.
        files = glob.glob(os.path.join(workdir, '*.deb'))
        if len(files) == 0:
            raise OnlPackageError("No debian package.")
        elif len(files) > 1:
            raise OnlPackageError("Too many packages.")
        else:
            # Move to the target directory
            shutil.copy(files[0], dir_)

        # Remove entire work directory.
        shutil.rmtree(workdir)

        # Return the path to the final built package
        return os.path.join(dir_, os.path.basename(files[0]))


    def tagged(self, tag):
        return tag in self.pkg.get('tags',[])

class OnlPackageGroup(object):
    """Debian Package Group

    A Package Group is defined in a Package Group File. It contains common
    package settings, multiple package declarations, and a common build step.

    All packages are built through their parent PackageGroup object.
    """

    def __init__(self):
        self.filtered = False

    def archcheck(self, arches):
        if arches is None:
            return True
        else:
            for p in self.packages:
                if p.arch() not in arches:
                    return False
            return True

    def distcheck(self):
        for p in self.packages:
            if p.pkg.get("dists", None):
                if g_dist_codename not in p.pkg['dists'].split(','):
                    return False
        return True

    def buildercheck(self, builder_arches):
        for p in self.packages:
            if p.arch() not in builder_arches:
                return False
        return True

    def prerequisite_packages(self):
        rv = []
        for e in list(onlu.sflatten(self._pkgs.get('prerequisites', {}).get('packages', []))):
            rv += e.split(',')
        return rv

    def prerequisite_submodules(self):
        return self._pkgs.get('prerequisites', {}).get('submodules', [])

    def prerequisites(self):
        return self._pkgs.get('prerequisites', {})

    def load(self, pkg):
        if not os.path.exists(pkg):
            raise OnlPackageError("Package file '%s' does not exist." % pkg)

        ddict = OnlPackage.package_defaults_get(pkg)

        pkg_data = onlyaml.loadf(pkg, ddict)

        pkglist = []

        #
        # Package declarations are dicts.
        #
        if(type(pkg_data) is not dict):
            raise OnlPackageError("The package file '%s' is empty or malformed." % pkg)

        if 'packages' not in pkg_data:
            raise OnlPackageError("The package file '%s' does not contain a packages declaration." % pkg)

        if type(pkg_data['packages']) is not list:
            raise OnlPackageError("The packages declaration is not a list.")

        self.packages = []
        for p in pkg_data['packages']:
            self.packages.append(OnlPackage(p, os.path.dirname(pkg),
                                            pkg_data.get('common', None),
                                            ddict))

        # This is used for the pkg_info dump
        self._pkg_info = pkg_data.copy()
        self._pkgs = pkg_data
        self._pkgs['__source'] = os.path.abspath(pkg)
        self._pkgs['__directory'] = os.path.dirname(self._pkgs['__source'])
        self._pkgs['__mtime'] = os.path.getmtime(pkg)

    def reload(self):
        """Reload our package file if it has changed."""

        if ( (self._pkgs['__mtime'] != os.path.getmtime(self._pkgs['__source']))  or
             (self._pkgs.get('reload', False)) ):
            logger.debug("Reloading updated package file %s..." % self._pkgs['__source'])
            self.load(self._pkgs['__source'])


    def __str__(self):
        return "\n".join( self.list().keys() )

    def list(self):
        rv = {}
        lst = [ p.id() for p in self.packages ]
        for p in self.packages:
            rv[p.id()] = self.prerequisites()
        return rv

    def pkg_info(self):
        return ("** Package Group: %s\n" % self._pkgs['__source'] +
                yaml.dump(self._pkg_info, default_flow_style = False) +
                "\n")

    def __contains__(self, pkg):
        """The current package group contains the given package id."""
        for p in self.packages:
            if pkg == 'all' or p.id() == pkg:
                return True
        return False

    def is_child(self, dir_):
        return self._pkgs['__directory'].startswith(dir_)


    def gmake_locked(self, target, operation):

        buildpaths = []
        if self._pkgs.get('build', True) and not os.environ.get('NOBUILD', False):
            buildpaths = [
                os.path.join(self._pkgs['__directory'], 'builds'),
                os.path.join(self._pkgs['__directory'], 'BUILDS'),
            ]

        for bp in buildpaths:
            if os.path.exists(bp):
                MAKE = os.environ.get('MAKE', "make")
                V = " V=1 " if logger.level < logging.INFO else ""
                cmd = MAKE + V + ' -C ' + bp + " " + os.environ.get('ONLPM_MAKE_OPTIONS', "") + " " + os.environ.get('ONL_MAKE_PARALLEL', "") + " " + target
                onlu.execute(cmd,
                             ex=OnlPackageError('%s failed.' % operation))

    def build(self, dir_=None):
        """Build all packages in the current group.

        dir_ : The output directory for the package group.
               The default is the package group parent directory.

        The option to build individual packages is not provided.
        The assumption is that the packages defined in the group are
        related and should always be built together.

        It is also assumed that all packages in the group have a common
        build step. That build step is performed once, and all packages
        are then built from the artifacts as defined in the package
        specifications.

        This assures there are not mismatches in the contents of packages
        from the same group and that there are no unecessary invocations of
        the build steps.
        """

        products = []

        with onlu.Lock(os.path.join(self._pkgs['__directory'], '.lock')):
            self.gmake_locked("", 'Build')
            for p in self.packages:
                products.append(p.build(dir_=dir_))


        if 'release' in self._pkgs:
            for (src, dst) in onlu.validate_src_dst_file_tuples(self._pkgs['__directory'],
                                                                self._pkgs['release'],
                                                                dict(),
                                                                OnlPackageError):
                root = os.path.join(os.environ.get('ONLPM_OPTION_RELEASE_DIR',
                                                   os.path.join(os.environ.get('ONL', 'RELEASE'))),
                                    g_dist_codename)
                OnlPackage.copyf(src, dst, root)

        return products

    def clean(self, dir_=None):
        with onlu.Lock(os.path.join(self._pkgs['__directory'], '.lock')):
            self.gmake_locked("clean", 'Clean')

class OnlPackageRepoUnlocked(object):
    """Package Repository and Interchange Class

    This class implements access to a single package repository.
    Packages can be:
      1. Installed in the repository.
      2. Looked up in the repository.
      3. Extracted from the repository.
      4. Extracted into a local cache whose file contents
         can be used by other packages with dependencies on those
         contents."""

    def __init__(self, root, packagedir='packages'):
        """Initialize a repo object.

        root : The root directory that should be used for this repository."""

        root = os.path.join(root, g_dist_codename)

        if not os.path.exists(root):
            os.makedirs(root)

        # The package subdirectory goes here
        self.repo = os.path.join(root, packagedir)

        # The extract cache goes here
        self.extracts = os.path.join(root, 'extracts')

    def add_packages(self, pkglist):
        """Add a package or list of packages to the repository."""
        for p in pkglist if type(pkglist) is list else [ pkglist ]:
            if not os.path.exists(p):
                raise OnlPackageError("Package file '%s' does not exist." % p)
            logger.info("adding package '%s'..." % p)
            underscores = os.path.basename(p).split('_')
            # Package name is the first entry
            package = os.path.split(underscores[0])[1]
            # Architecture is the last entry (.deb)
            arch = underscores[-1].split('.')[0]
            logger.debug("+ /bin/cp %s %s/%s", p, self.repo, "binary-" + arch)
            dstdir = os.path.join(self.repo, "binary-" + arch)
            if not os.path.exists(dstdir):
                os.makedirs(dstdir)
            logger.info("dstdir=%s"% dstdir)

            # Remove any existing versions of this package.
            for existing in glob.glob(os.path.join(dstdir, "%s_*.deb" % package)):
                logger.info("Removing existing package %s" % existing)
                os.unlink(existing)

            shutil.copy(p, dstdir)
            extract_dir = os.path.join(self.extracts, arch, package)
            if os.path.exists(extract_dir):
                # Make sure the package gets re-extracted the next time it's requested by clearing any existing extract in the cache.
                logger.info("removed previous extract directory %s...", extract_dir)
                logger.debug("+ /bin/rm -fr %s", extract_dir)
                shutil.rmtree(extract_dir)

    def remove_packages(self, pkglist):
        for p in pkglist if type(pkglist) is list else [ pkglist ]:
            path = self.lookup(p)
            if path:
                logger.info("removing package %s..." % p)
                os.unlink(path)

    def lookup_all(self, pkg):
        """Lookup all packages in the repo matching the given package identifier."""
        rv = []
        (name, arch) = OnlPackage.idparse(pkg)
        dirname = os.path.join(self.repo, "binary-" + arch)
        if os.path.exists(dirname):
            manifest = os.listdir(dirname)
            rv = [ os.path.join(dirname, x) for x in manifest if arch in x and "%s_" % name in x ]
        return rv

    def __contains__(self, pkg):
        r = self.lookup_all(pkg)
        return len(r) != 0

    def lookup(self, pkg, ex=False):
        """Lookup a package in the repo. The package must be unique and exist."""
        r = self.lookup_all(pkg)
        if len(r) == 0:
            if ex:
                raise OnlPackageError("Package %s is not in the repository." % pkg)
            return False
        elif len(r) > 1:
            if ex:
                raise OnlPackageError("Multiple matches for package identifier '%s': %s" % (pkg, r))
            return False
        else:
            return r[0]

    def extract(self, pkg, dstdir=None, prefix=True, force=False, remove_ts=False, sudo=False):
        """Extract the given package.

        pkg : The package identifier. Must be unique in the repo.
        dstdir : The parent directory which will contain the extracted package contents.
                 The default is the local repo's extract cache.
        force: If True, the package will be extracted even if its contents are already valid in the extract cache."""

        PKG_TIMESTAMP = '.PKG.TIMESTAMP'

        path = self.lookup(pkg)
        if path:

            if dstdir is None:
                dstdir = self.extracts

            if prefix:
                edir = os.path.join(dstdir, pkg.replace(':', '_'))
            else:
                edir = dstdir

            if not force and os.path.exists(os.path.join(edir, PKG_TIMESTAMP)):
                if (os.path.getmtime(os.path.join(edir, PKG_TIMESTAMP)) ==
                    os.path.getmtime(path)):
                    # Existing extract is identical to source package
                    logger.debug("Existing extract for %s matches the package file." % pkg)
                else:
                    # Existing extract must be removed.
                    logger.info("Existing extract for %s does not match." % pkg)
                    force=True
            else:
                # Status unknown. Really shouldn't happen.
                force=True

            if force:
                if os.path.exists(edir) and prefix:
                    logger.debug("rm -rf %s" % edir)
                    shutil.rmtree(edir)
                if not os.path.exists(edir):
                    os.makedirs(edir)

                onlu.execute([ 'dpkg', '-x', path, edir ], sudo=sudo)
                onlu.execute([ 'touch', '-r', path, os.path.join(edir, PKG_TIMESTAMP) ], sudo=sudo)

            if remove_ts and os.path.exists(os.path.join(edir, PKG_TIMESTAMP)):
                onlu.execute([ 'rm', os.path.join(edir, PKG_TIMESTAMP) ], sudo=sudo)

            return edir

        return False

    def contents(self, pkg):
        path = self.lookup(pkg)
        if path:
            print "** %s contents:" % path
            onlu.execute(['dpkg', '-c', path])


    def get_file(self, pkg, filename, force=False, ex=True):
        """Get a file contained in the given package.

        The package will be extracted (if necessary) into the extract cache
        and the path to the requested file will be returned.

        force: Passed to extract() as the force option."""

        edir = self.extract(pkg, force=force)
        for root, dirs, files in os.walk(edir):
            for file_ in files:
                if file_ == filename:
                    return os.path.join(root, file_)

        if ex:
            raise OnlPackageMissingFileError(pkg, filename)

        return None


    def get_dir(self, pkg, dirname, force=False, ex=True):
        """Get a directory contained in the given package.

        The package will be extracted (if necessary) into the extract cache
        and the path to the requested directory will be returned.

        force: Passed to extract() as the force option."""

        edir = self.extract(pkg, force=force)
        if os.path.isabs(dirname):
            apath = os.path.join(edir, dirname[1:]);
            if os.path.isdir(apath):
                return apath
        else:
            for root, dirs, files in os.walk(edir):
                if os.path.basename(root) == dirname and root != edir:
                    return root

        if ex:
            raise OnlPackageMissingDirError(pkg, dirname)

        return None


class OnlPackageRepo(object):
    def __init__(self, root, packagedir='packages'):
        self.r = OnlPackageRepoUnlocked(root, packagedir)
        self.lock = onlu.Lock(os.path.join(root, '.lock'))

    def __contains__(self, pkg):
        with self.lock:
            return self.r.__contains__(pkg)

    def get_dir(self, pkg, dirname, force=False, ex=True):
        with self.lock:
            return self.r.get_dir(pkg, dirname, force, ex)

    def get_file(self, pkg, filename, force=False, ex=True):
        with self.lock:
            return self.r.get_file(pkg, filename, force, ex)

    def add_packages(self, pkglist):
        with self.lock:
            return self.r.add_packages(pkglist)

    def remove_packages(self, pkglist):
        with self.lock:
            return self.r.remove_packages(pkglist)

    def lookup(self, pkg, ex=False):
        with self.lock:
            return self.r.lookup(pkg, ex)

    def lookup_all(self, pkg):
        with self.lock:
            return self.r.lookup_all(pkg)

    def extract(self, pkg, dstdir=None, prefix=True, force=False, remove_ts=False, sudo=False):
        with self.lock:
            return self.r.extract(pkg, dstdir, prefix, force, remove_ts, sudo)

    def contents(self, pkg):
        with self.lock:
            return self.r.contents(pkg)

class OnlPackageManager(object):

    def __init__(self):
        # Stores all loaded package groups.
        self.package_groups = []
        self.opr = None

    def set_repo(self, repodir, packagedir='packages'):
        self.opr = OnlPackageRepo(repodir, packagedir=packagedir)


    def filter(self, subdir=None, arches=None, substr=None):

        for pg in self.package_groups:
            if subdir and not pg.is_child(subdir):
                pg.filtered = True
            if not pg.archcheck(arches):
                pg.filtered = True

    def __cache_name(self, basedir):
        return os.path.join(basedir, '.PKGs.cache.%s' % g_dist_codename)

    def __write_cache(self, basedir):
        cache = self.__cache_name(basedir)
        logger.debug("Writing the package cache %s..." % cache)
        pickle.dump(self.package_groups, open(cache, "wb"))

    def __load_cache(self, basedir, ro):
        cache=self.__cache_name(basedir)

        # Lock the cache file
        with onlu.Lock(cache + ".lock"):
            if os.path.exists(cache):
                logger.debug("Loading from package cache %s" % cache)

                try:
                    self.package_groups = pickle.load(open(cache, "rb"))
                except Exception, e:
                    logger.warn("The existing package cache is corrupted. It will be rebuilt.")
                    return False

                if ro:
                    return True

                # Validate and update the cache
                for pg in self.package_groups:
                    pg.reload()

                self.__write_cache(basedir)
                return True

        return False


    def __builder_arches(self):
        arches = [ 'all', 'amd64' ]
        arches = arches + subprocess.check_output(['dpkg', '--print-foreign-architectures']).split()
        return arches

    def __build_cache(self, basedir):
        pkgspec = [ 'PKG.yml', 'pkg.yml' ]

        builder_arches = self.__builder_arches()

        for root, dirs, files in os.walk(basedir):
            for f in files:
                if f in pkgspec:
                    if "%s.disabled" % f in files:
                        logger.warn("Skipping %s due to .disabled file)." % os.path.join(root, f))
                    else:
                        pg = OnlPackageGroup()
                        try:
                            logger.debug('Loading package file %s...' % os.path.join(root, f))
                            pg.load(os.path.join(root, f))
                            logger.debug('  Loaded package file %s' % os.path.join(root, f))
                            if pg.distcheck() and pg.buildercheck(builder_arches):
                                self.package_groups.append(pg)
                        except OnlPackageError, e:
                            logger.error("%s: " % e)
                            logger.warn("Skipping %s due to errors." % os.path.join(root, f))

    def load(self, basedir, usecache=True, rebuildcache=False, roCache=False):
        if usecache is True and rebuildcache is False:
            if self.__load_cache(basedir, roCache):
                return

        self.__build_cache(basedir)

        if usecache:
            # Write the package cache
            self.__write_cache(basedir)


    def __contains__(self, pkg):
        for pg in self.package_groups:
            if pkg == 'all' or pkg in pg:
                return True
        return False

    def build(self, pkg=None, dir_=None, filtered=True, prereqs_only=False):

        built = False

        for pg in self.package_groups:
            if pkg is None or pkg in pg:

                if filtered and pg.filtered:
                    continue

                if not prereqs_only:
                    #
                    # Process prerequisite submodules.
                    # Only due this if we are building the actual package,
                    # not processing the package dependencies.
                    #
                    for sub in pg.prerequisite_submodules():
                        root = sub.get('root', None)
                        path = sub.get('path', None)
                        depth = sub.get('depth', None)
                        recursive = sub.get('recursive', None)

                        if not root:
                            raise OnlPackageError("Submodule prerequisite in package %s does not have a root key." % pkg)

                        if not path:
                            raise OnlPackageError("Submodule prerequisite in package %s does not have a path key." % pkg)

                        try:
                            manager = submodules.OnlSubmoduleManager(root)
                            manager.require(path, depth=depth, recursive=recursive)
                        except submodules.OnlSubmoduleError, e:
                            raise OnlPackageError(e.value)

                # Process prerequisite packages
                for pr in pg.prerequisite_packages():
                    logger.info("Requiring prerequisite package %s..." % pr)
                    self.require(pr, build_missing=True)

                if not prereqs_only:
                    # Build package
                    products = pg.build(dir_=dir_)
                    if self.opr:
                        # Add results to our repo
                        self.opr.add_packages(products)

                built = True

        if not built:
            raise OnlPackageMissingError(pkg)

    def clean(self, pkg=None, dir_=None):
        for pg in self.package_groups:
            if pkg is None or pkg in pg:
                products = pg.clean(dir_=dir_)
                if self.opr:
                    # Remove results from our repo
                    self.opr.remove_packages(products)

    def require(self, pkg, force=False, build_missing=False, skip_missing=False,
                try_arches=None):

        if pkg not in self and try_arches:
            for a in try_arches:
                p = "%s:%s" % (pkg, a)
                if p in self:
                    pkg = p;
                    break

        if pkg not in self:
            if skip_missing:
                return
            # Can't require a package we don't have
            raise OnlPackageMissingError(pkg)

        if force or (pkg not in self.opr and build_missing):
            logger.info("Rebuilding %s... " % pkg)
            self.build(pkg, filtered=False)
        else:
            if pkg in self.opr:
                self.build(pkg, filtered=False, prereqs_only=True)

        if pkg not in self.opr:
            raise OnlPackageError("Package %s is required but has not been built." % pkg)


    def __str__(self):
        return "\n".join(self.list())

    def filtered_package_groups(self):
        return [ pg for pg in self.package_groups if not pg.filtered ]

    def list(self):
        rv = {}
        for pg in self.filtered_package_groups():
            for (p,d) in pg.list().iteritems():
                rv[p] = d
        return rv

    def pmake(self, handle=sys.stdout):
        packages = self.list()

        # Collect some dependency data
        TARGETS={}
        ARCHS={}

        for (p,d) in packages.iteritems():
            (name,arch) = p.split(':')
            target = p.replace(':', '_')
            depends = " ".join(d.get('packages', [])).replace(':', '_')

            TARGETS[target] = TARGETS.get(target, {})
            TARGETS[target][arch] = arch
            TARGETS[target]['name'] = target
            TARGETS[target]['depends'] = depends
            TARGETS[target]['package'] = p
            ARCHS[arch] = ARCHS.get(arch, [])
            ARCHS[arch].append(TARGETS[target])

            if d.get('broken', False):
                TARGETS[target]['stage'] = 20
            elif d.get('stage', False):
                TARGETS[target]['stage'] = d.get('stage')
            elif len(depends) == 0:
                TARGETS[target]['stage'] = 0
            else:
                TARGETS[target]['stage'] = 1

        handle.write("# -*- GNUMakefile -*-\n\n")
        handle.write("THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))\n")
        handle.write("SHELL := /bin/bash\n")
        handle.write("BUILDING := $(THIS_DIR)/building\n")
        handle.write("FINISHED := $(THIS_DIR)/finished\n")
        handle.write("$(shell mkdir -p $(BUILDING) $(FINISHED))\n\n")
        handle.write("-include Make.local\n\n")

        handle.write("############################################################\n")
        handle.write("#\n")
        handle.write("# These are the rules that build each individual package.\n")
        handle.write("#\n")
        handle.write("############################################################\n")

        for (t, d) in TARGETS.iteritems():
            handle.write("%s : %s\n" % (t, d['depends']))
            handle.write("\tset -o pipefail && onlpm.py --ro-cache --require %s |& tee $(BUILDING)/$@\n" % (d['package']))
            handle.write("\tmv $(BUILDING)/$@ $(FINISHED)/\n")

        for (arch, targets) in ARCHS.iteritems():
            handle.write("############################################################\n")
            handle.write("#\n")
            handle.write("# These rules represent the build stages for arch='%s'\n" % arch)
            handle.write("#\n")
            handle.write("############################################################\n")
            STAGES = {}
            for t in targets:
                STAGES[t['stage']] = STAGES.get(t['stage'], [])
                STAGES[t['stage']].append(t['name'])

            for stage in range(0, 10):
                handle.write("arch_%s_stage%s: %s\n\n" % (arch, stage, " ".join(STAGES.get(stage, []))))

        for arch in ARCHS.keys():
            handle.write("arch_%s:\n" % arch)
            for stage in range(0, 10):
                handle.write("\t$(MAKE) arch_%s_stage%s\n" % (arch, stage))
            handle.write("\n")



    def pkg_info(self):
        return "\n".join([ pg.pkg_info() for pg in self.package_groups if not pg.filtered ])

    def list_platforms(self, arch):
        platforms = []
        for pg in self.package_groups:
            for p in pg.packages:
                (name, pkgArch) = OnlPackage.idparse(p.id())
                m = re.match(r'onl-platform-config-(?P<platform>.*)', name)
                if m:
                    if arch in [ pkgArch, "all", None ]:
                        platforms.append(m.groups('platform')[0])
        return platforms

def defaultPm():
    repo = os.environ.get('ONLPM_OPTION_REPO', None)
    envJson = os.environ.get('ONLPM_OPTION_INCLUDE_ENV_JSON', None)
    packagedirs = os.environ['ONLPM_OPTION_PACKAGEDIRS'].split(':')
    repoPackageDir = os.environ.get('ONLPM_OPTION_REPO_PACKAGE_DIR', 'packages')
    subdir = os.getcwd()
    arches = ['amd64', 'powerpc', 'armel', 'armhf', 'arm64', 'all',]

    if envJson:
        for j in envJson.split(':'):
            data = json.load(open(j))
            for (k, v) in data.iteritems():
                try:
                    v = v.encode('ascii')
                except UnicodeEncodeError:
                    pass
                os.environ[k] = v

    pm = OnlPackageManager()
    pm.set_repo(repo, packagedir=repoPackageDir)
    for pdir in packagedirs:
        pm.load(pdir, usecache=True, rebuildcache=False)
    pm.filter(subdir = subdir, arches=arches)

    return pm

if __name__ == '__main__':

    ap = argparse.ArgumentParser("onlpm")
    ap.add_argument("--repo", default=os.environ.get('ONLPM_OPTION_REPO', None))
    ap.add_argument("--repo-package-dir", default=os.environ.get('ONLPM_OPTION_REPO_PACKAGE_DIR', 'packages'))
    ap.add_argument("--packagedirs", nargs='+', metavar='PACKAGEDIR')
    ap.add_argument("--subdir", default=os.getcwd())
    ap.add_argument("--extract-dir", nargs=2, metavar=('PACKAGE', 'DIR'), action='append')
    ap.add_argument("--force", action='store_true')
    ap.add_argument("--list", action='store_true');
    ap.add_argument("--list-all", action='store_true')
    ap.add_argument("--list-tagged")
    ap.add_argument("--list-platforms", action='store_true')
    ap.add_argument("--csv", action='store_true')
    ap.add_argument("--show-group", action='store_true')
    ap.add_argument("--arch")
    ap.add_argument("--arches", nargs='+', default=['amd64', 'powerpc', 'armel', 'armhf', 'arm64', 'all']),
    ap.add_argument("--pmake", action='store_true')
    ap.add_argument("--prereq-packages", action='store_true')
    ap.add_argument("--lookup", metavar='PACKAGE')
    ap.add_argument("--find-file", nargs=2, metavar=('PACKAGE', 'FILE'))
    ap.add_argument("--find-dir", nargs=2, metavar=('PACKAGE', 'DIR'))
    ap.add_argument("--link-file", nargs=3, metavar=('PACKAGE', 'FILE', 'DST'), action='append')
    ap.add_argument("--link-dir",  nargs=3, metavar=('PACKAGE', 'FILE', 'DST'), action='append')
    ap.add_argument("--copy-file", nargs=3, metavar=('PACKAGE', 'FILE', 'DST'), action='append')
    ap.add_argument("--build", nargs='+', metavar='PACKAGE')
    ap.add_argument("--clean", nargs='+', metavar='PACKAGE')
    ap.add_argument("--require", nargs='+', metavar='PACKAGE')
    ap.add_argument("--no-build-missing", action='store_true')
    ap.add_argument("--contents", nargs='+', metavar='PACKAGE')
    ap.add_argument("--delete", nargs='+', metavar='PACKAGE')
    ap.add_argument("--sudo", action='store_true')
    ap.add_argument("--verbose", action='store_true')
    ap.add_argument("--quiet", action='store_true')
    ap.add_argument("--rebuild-pkg-cache", action='store_true', default=os.environ.get('ONLPM_OPTION_REBUILD_PKG_CACHE', False))
    ap.add_argument("--no-pkg-cache", action='store_true', default=os.environ.get('ONLPM_OPTION_NO_PKG_CACHE', False))
    ap.add_argument("--ro-cache", action='store_true', help="Assume existing package cache is up-to-date and read-only. Should be specified for parallel builds.")
    ap.add_argument("--pkg-info", action='store_true')
    ap.add_argument("--skip-missing", action='store_true')
    ap.add_argument("--try-arches", nargs='+', metavar='ARCH')
    ap.add_argument("--in-repo", nargs='+', metavar='PACKAGE')
    ap.add_argument("--include-env-json", default=os.environ.get('ONLPM_OPTION_INCLUDE_ENV_JSON', None))
    ap.add_argument("--platform-manifest", metavar=('PACKAGE'))

    ops = ap.parse_args()

    archlist = []
    for a in ops.arches:
        al = a.split(',')
        archlist = archlist + al
    ops.arches = archlist

    if ops.include_env_json:
        for j in ops.include_env_json.split(':'):
            data = json.load(open(j))
            for (k, v) in data.iteritems():
                try:
                    v = v.encode('ascii')
                except UnicodeEncodeError:
                    pass
                os.environ[k] = v

    #
    # The packagedirs and repo options can be inherited from the environment
    # to make it easier to customize the settings for a given project.
    #
    # If these options are none it means neither was specified on the
    # commandline or environment.
    #
    if not ops.packagedirs:
        if 'ONLPM_OPTION_PACKAGEDIRS' not in os.environ:
            logger.error("No packagedirs specified. Please use the --packagedirs option or set ONLPM_OPTION_PACKAGEDIRS in the environment.")
            sys.exit(1)
        ops.packagedirs = os.environ['ONLPM_OPTION_PACKAGEDIRS'].split(':')

    if not ops.repo:
        logger.error("No repo directory specified. Please use the --repo option or set ONLPM_OPTION_REPO in the environment.")
        sys.exit(1)


    logger.setLevel(logging.INFO)
    if ops.quiet:
        logger.setLevel(logging.ERROR)
    if ops.verbose or os.environ.get('ONLPM_VERBOSE', None):
        logger.setLevel(logging.DEBUG)

    try:

        pm = OnlPackageManager()
        if ops.repo:
            logger.debug("Setting repo as '%s'..." % ops.repo)
            pm.set_repo(ops.repo, packagedir=ops.repo_package_dir)

        if ops.in_repo:
            for p in ops.in_repo:
                print "%s: %s" % (p, p in pm.opr)
            sys.exit(0)

        for pdir in ops.packagedirs:
            logger.debug("Loading package dir %s..." % pdir)
            pm.load(pdir, usecache=not ops.no_pkg_cache, rebuildcache=ops.rebuild_pkg_cache, roCache=ops.ro_cache)
            logger.debug("  Loaded package dir %s" % pdir)

        if ops.list_tagged:
            for pg in pm.package_groups:
                for p in pg.packages:
                    if p.tagged(ops.list_tagged):
                        if ops.arch in [ p.pkg['arch'], "all", None ]:
                            print "%-64s" % p.id(),
                            if ops.show_group:
                                print "[ ", pg._pkgs['__source'], "]",
                            print

        if ops.list_platforms:
            if not ops.arch:
                logger.error("missing --arch with --list-platforms")
                sys.exit(1)
            platforms = pm.list_platforms(ops.arch)
            if ops.csv:
                print ','.join(platforms)
            else:
                for p in platforms:
                    print "%-64s" % p

        # List all packages, no filtering
        if ops.list_all:
            print pm

        if ops.pmake:
            pm.pmake()


        pm.filter(subdir = ops.subdir, arches=ops.arches)

        if ops.list:
            print pm

        if ops.pkg_info:
            print pm.pkg_info()


        ############################################################
        #
        # Build all packages.
        #
        ############################################################
        if ops.clean:
            raise OnlPackageError("Clean not implemented yet.")
            for p in ops.clean:
                if p in pm:
                    pm.clean(p)
                else:
                    raise OnlPackageMissingError(p)

        if ops.build:
            for p in ops.build:
                if p in pm:
                    pm.build(p)
                else:
                    raise OnlPackageMissingError(p)

        if ops.require:
            for p in ops.require:
                pm.require(p, force=ops.force, build_missing=not ops.no_build_missing,
                           skip_missing=ops.skip_missing, try_arches=ops.try_arches)

        if ops.find_file:
            (p, f) = ops.find_file
            pm.require(p, force=ops.force, build_missing=not ops.no_build_missing)
            path = pm.opr.get_file(p, f)
            print path

        if ops.find_dir:
            (p, d) = ops.find_dir
            pm.require(p, force=ops.force, build_missing=not ops.no_build_missing)
            path = pm.opr.get_dir(p, d)
            print path

        if ops.link_file:
            for (p, f, dst) in ops.link_file:
                pm.require(p, force=ops.force, build_missing=not ops.no_build_missing)
                path = pm.opr.get_file(p, f)
                if dst == '.':
                    dst = f
                if os.path.exists(dst):
                    os.unlink(dst)
                os.symlink(path, dst)

        if ops.link_dir:
            for (p, d, dst) in ops.link_dir:
                pm.require(p, force=ops.force, build_missing=not ops.no_build_missing)
                path = pm.opr.get_dir(p, d)
                if dst == '.':
                    dst = d
                if os.path.exists(dst):
                    os.unlink(dst)
                os.symlink(path, dst)

        if ops.copy_file:
            for (p, f, dst) in ops.copy_file:
                pm.require(p, force=ops.force, build_missing=not ops.no_build_missing)
                path = pm.opr.get_file(p, f)
                if dst == '.':
                    dst = f
                if os.path.exists(dst):
                    os.unlink(dst)
                shutil.copyfile(path, dst);

        if ops.extract_dir:
            for (p, d) in ops.extract_dir:
                pm.require(p, force=ops.force, build_missing=not ops.no_build_missing)
                pm.opr.extract(p, dstdir=d, prefix=False, force=True, remove_ts=True, sudo=ops.sudo)

        ############################################################
        #
        # Show the contents of the given packages.
        #
        ############################################################
        if ops.contents:
            for p in ops.contents:
                pm.require(p, force=ops.force, build_missing=not ops.no_build_missing)
                pm.opr.contents(p)


        if ops.platform_manifest:
            pm.require(ops.platform_manifest, force=ops.force, build_missing=not ops.no_build_missing)
            path = pm.opr.get_file(ops.platform_manifest, 'manifest.json')
            if path:
                m = json.load(open(path))
                print " ".join(m['platforms'])


        ############################################################
        #
        # Delete the given packages from the repo
        #
        ############################################################
        if ops.delete:
            pm.opr.remove_packages(ops.delete)

        if ops.lookup:
            logger.debug("looking up %s", ops.lookup)
            for p in pm.opr.lookup_all(ops.lookup):
                print p

    except (OnlPackageError, onlyaml.OnlYamlError), e:
        logger.error(e)
        sys.exit(1)

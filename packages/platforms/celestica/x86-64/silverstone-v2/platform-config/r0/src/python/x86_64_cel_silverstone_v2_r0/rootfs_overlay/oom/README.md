# oom
Open Optical Monitoring - http://www.opencompute.org/wiki/Networking/SpecsAndDesigns#Open_Optical_Monitoring

This is a project to make the contents of optical module EEPROMs
accessible to python programmers.  This allows a python programmer
to query the value of dozens of keys (serial Number, module type,
temperature, transmit power, ...), for the optical module in each
port of a switch.  In addition to key/value read access, the OOM
project also supports key/value write to a limited number of EEPROM
locations, and raw read/write access directly to EEPROM.

NEWS: August 12, 2021
Two different big updates...

OOM, the python package, has been updated to support Python 3.  It also
supports Python 2.7, with the same code and interface.  (Install it
via setup.py, separately for each version of python.)  It should be
backward compatible except that the 'hexstr' function has been renamed
'get_hexstr'.

optoe, the linux driver, was significantly updated to build/run in the
5.11 upstream linux kernel.  It was submitted there, and rejected for
architectural, not technical reasons.  That code has been checked in here.
This update includes device tree support, nvmem support and regmap support.
This it can be configured via device tree files, and it can be accessed from
within the kernel.  It is also completely backward compatible with the
previous version of optoe.  Check oom/optoe/optoe.rst for documentation on
the original and new capabilities.

NEWS:  August 1, 2019
The optoe driver has been updated to support CMIS type devices.  These
include QSFP-DD, OSFP, COBO and SFP-DD at least.  They conform to the
"Common Management Interface Specification", which defines a different
EEPROM layout from either SFP (sff_8472) or QSFP (sff_8636) type devices.
See the 'optoe_doc' file in the optoe directory for details on how to
specify a CMIS device (hint, it is 'optoe3').

An OOM keyfile for CMIS devices has also been added (cmis.py).  Consider
it a sample.  The keys are believed to be correct, but only a limited
number of keys have been implemented.  Users can add their own keyfile
defining more keys, or contact the maintainer to discuss adding keys to
the cmis.py file.

NEWS:  February 2, 2018
The optoe driver has been used by several server vendors, and pushed
to at least two NOS repos.  Much has been learned in the process.  Sonic
is migrating to Linux 4.9, which has removed one interface that optoe was
using.  So, optoe has been updated to incorporate this and several other
items:
  - optoe.h merged into optoe.c (based on feedback from users)
  - removed struct memory_accessor (not in Linux 4.6 and newer)
  - accept port_name through platform_data if provided
  - minor simplification of code
  - optoe.c passes linux checkpatch.pl and is ready to submit upstream

Switch and NOS vendors are encouraged to use this version of optoe.c
If you need to change the code for your usage, please discuss this
with don@thebollingers.org so I can maintain a common driver for all.

NEWS:  December 3, 2017
The OOM graphical demo has been released in this github repo.  Used by
Finisar at the last two OCP Summits, it is now available to all.  As
with all of OOM, it works equally for any optics that conform to the
relevant standards, and works on any switch/NOS stack that supports OOM.
See the file in demo/README for instructions to install and use.

NEWS:  October 8, 2017
Beta Release of the 'optoe' driver.  Supports SFP type (SFF-8472) and
QSFP type (SFF-8436) i2c devices, provides read/write access to all
256 architected pages (tables) of these devices.  Driver, include file
and brief documentation are in the 'optoe' directory.

OOM has been updated to locate eeprom files and port names created by optoe.

NEWS:  August 25, 2017
Checked in a linux driver that demonstrates write capability, and the
ability to access up to the full 128 pages architected for SFP/QSFP
optical transceivers.  Probably not ready for production deployment,
it has undergone limited testing, and does not rigorously handle all
error paths.  Built and run successfully on a Linux 3.2 and
Linux 4.1 kernel in a Cumulus NOS environment on an AS5712. This code
has also been ported and tested on the accton AS7712-32x development
environment.  (see oom/sff_8436_eeprom_deb.c, oom_sff-8436.h)

CFP support has been implemented.  Note only 4 keys implemented so far,
but all the code is in place to support all functions.

NEWS:  March 31, 2017

Created a Python shim, that automatically identifies ports and EEPROM files
for Cumulus and ONL drivers.  This shim eliminates the need to compile
a C library for use as the NOS-specific shim.  Thus there is no requirement
to have a build environment to install OOM.  At runtime, OOM will still try
first to load a C library (.so) shim from <package_dir>/lib/oom_south.so.
This allows existing and additional C shims to be used with OOM.  If it
fails to load lib/oom_south.so, then it will load the Python shim
(oomsysfsshim.py).

The process to install OOM on a switch now is:

	cd /usr/local     <or any location you prefer, OOM doesn't care>
	mkdir oom         <or any  name you prefer, OOM doesn't care>
	cd oom
	# install OOM (can also be copied from a thumbstick or ...)
	git clone https://github.com/opencomputeproject/oom.git
	cd oom
	python setup.py install   # installation is complete
	# verify it works
	cd apps
	python inventory.py   <should list all the ports, ID the modules>

There is also now a JSON WSGI shim, and a web service, to provide OOM services
over the network, from a switch to a management server.  This is a simple
proof of concept, probably not secure enough for your production environment.
It consists of a python shim (oomjsonshim.py) which is used by the OOM
application to inventory ports, and read/write EEPROMs, over the network.
It is loaded via 'oomlib.setshim("oomjsonshim", <switch URL>'.  The URL can
be as simple as 192.168.0.100.  Fully decorated, it would be
http://192.168.0.100:5000/OOM (the web server operates at port 5000).
To start the web service, install OOM on the switch, and run
<oom_dir>/apps/oomjsonsvr.py.  This script can also be run as a service,
launched at startup of the switch.

Note, there is a new OOM interface, oomlib.setshim(shim, parms).  This
interface loads the named python shim, then calls the shim's 'setparms'
routine with 'parms' as it's only parameter.  (See the code.)

Old News: July 13, 2016

   Updated the version of OOM to 0.4, LOTS of improvements have gone in
   since the last version update.  The most recent change, which triggered
   the version roll, is a reorganization to the code, and a code cleanup
   which allows OOM to be installed as a site-specific python package.

   As of Version 0.4, there is a test directory, for scripts which test
   the OOM code.  There is an apps directory for scripts which provide
   useful user output from OOM or otherwise call on OOM but are not
   part of the package API.  The data files which drive the simulator
   SHIM have been moved to the module_data directory.  The remaining files
   in the oom directory are the core OOM library modules, plus the SHIMs
   and make files to assemble OOM.  (The SHIMs may be factored out as well
   in a future version.)

   OOM can now be installed as a package.  In the top level oom directory
   there is a setup.py script.  AFTER building oom with the desired SHIM,
   oom can be installed with 'python setup.py install'.  Building the
   appropriate SHIM (simulator(file), aardvark, or the one that matches
   the NOS on which it is installed), must be done first, so that the SHIM
   will be installed with the package.  Note that only the OOM API is
   installed as a package.  The test and apps directories contain scripts
   that can be run from anywhere, but are not installed as part of the
   package.

   There are many ways to acquire and install OOM, but this recipe is
   known to work in the Cygwin environment, to install the simulator SHIM:

	cd /usr/local     <or any location you prefer, OOM doesn't care>
	mkdir oom         <or any  name you prefer, OOM doesn't care>
	cd oom
	# install OOM (can also be copied from a thumbstick or ...)
	git clone https://github.com/opencomputeproject/oom.git
	cd oom/oom
	make SHIM=file all
	cd ..
	python setup.py install   # installation is complete
	# verify it works
	cd apps
	python inventory.py   # shows simulated switch:  SFP, QSFP+, QSFP28

   OOM can also be built for use in a native Windows environment.  The
   recipe depends on use of the x86_64-w64-mingw32-gcc compiler, and has
   only been used in a Cygwin environment.  This recipe builds with the
   Aardvark SHIM, for testing devices with an Aardvark USB/i2c adapter.
   If you are using this environment, contact Don (don@thebollingers.org)
   for additional support and documentation. The recipe:

	cd <where you want to stage oom>
	mkdir myoom
	cd myoom
	git clone https://github.com/ocpnetworking-wip/oom.git
	cd oom/oom
	make -f makewindows
	<build a ZIP file containing everything in myoom/oom>
	<move the ZIP file to the target Windows system>
	<unpack the ZIP file into the desired Windows folder>
	python setup.py install  # installation is complete
	cd apps
	python inventory.py  # shows Aardvark accessible device(s)


Old News:  February 16, 2016

   The Master branch has been updated with the latest work:
     - Added table drive oom_set_keyvalue()
     - Added write keys to sfp.py, qsfp_plus.py
     - Added memory map, function map and write map (mmap, fmap, wmap)
       to each (python) port at initialization (oom_get_portlist())
     - Cleaned up mapping of module type to 'sfp.py', 'qsfp_plus.py'.
       New decode types can just be added as new files to the package
       without modifying code.
     - Fixed linux build issues (thanks Dustin)
     - Removed all the pesky DOS <cr> at the end of each line of
       many files (thanks Dustin)
     - I will continue to use the 'dev' branch for upcoming changes
     - Old news and mundane stuff below is still accurate

Old News: Feb 11

   The new Southbound API is now committed to the master branch,
   and should be considered accepted for development going forward.

   The 'dev' branch will contain latest updates going forward, and
   is also based on the new Southbound API

   BETA, demo_code and newsouth branches have been removed.  Master
   is stable and complete, previous branches are out of date.


More mundane stuff...

There is a mock Southbound Shim, oom_south.c, among the python modules.
This make file builds the mock shim, installs it, and also installs
some data files with static but real data for some modules.

The user accessible functions are all in oom.py, which constitutes the
"Northbound Interface".  The use of all of these functions is shown
in oomdemo.py <now in the apps directory>.  oomdemo.py is very short,
demonstrating not only the functionality, but the simplicity of the
interface.

<Now in the test directory...>
keytest.py extracts and displays every key available for SFP modules,
as well as the key collections (functions) available.

qtest.py extracts and displays every key available for QSFP+ modules,
as well as the key collections (functions) available.

The build process assumes your C compiler builds libraries that your
Python interpereter can run!

Note in the makefile the process to populate the module_data directory
with data with the correct names.  Substitute different source files,
or give these different numbers in the first character to populate
different ports with different data.

Questions?  Please contact don@thebollingers.org

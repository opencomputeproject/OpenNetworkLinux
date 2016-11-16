#!/usr/bin/env python
################################################################
#
#        Copyright 2013, Big Switch Networks, Inc.
#
# Licensed under the Eclipse Public License, Version 1.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#        http://www.eclipse.org/legal/epl-v10.html
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the
# License.
#
################################################################
#
# This script generates a new code module and unit test build
# for this repository.
#
################################################################

import sys
import os

# The root of the repository
ROOT = os.path.realpath("%s/.." % (os.path.dirname(__file__)))

# Import infra
sys.path.append("%s/sm/infra/builder/unix/tools" % ROOT)

# Import bigcode
sys.path.append("%s/sm/bigcode/tools" % ROOT)

from modulegen import *

#
# Import uCli support
#
from uclimod import *


if __name__ == "__main__":

    # Generate the module in the current directory.
    ModuleGenerator.modulesBaseDir = "."

    # Use ONL config.mk in generated makefiles
    GModuleMake.INIT_MK="$(ONL)/make/config.mk"
    ModuleUnitTestTargetMake.INIT_MK="$(ONL)/make/config.mk"

    #
    # Make it happen.
    #
    ModuleGenerator.main(globals().copy())

    # Make sure the manifest gets regenerated.
    os.system("rm -rf %s/make/modules/modules*" % ROOT)








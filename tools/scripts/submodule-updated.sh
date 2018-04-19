#!/bin/bash
############################################################
#
# This script is called whenever a submodule is updated
# in the repository.
#
# When a submodule is updated there are two things that need
# to happen:
#
# 1. The module manifest needs to be regenerated.
# 2. The package cache needs to be regenerated.
#
############################################################

# Removing the manifest causes it to be regenerated.
rm -rf $ONL/make/modules/module*

# Rebuild pkg cache
$ONL/tools/onlpm.py --rebuild-pkg-cache

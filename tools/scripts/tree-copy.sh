#!/bin/bash
############################################################
# <bsn.cl fy=2015 v=onl>
#
#           Copyright 2015 Big Switch Networks, Inc.
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
# </bsn.cl>
############################################################
#
# Simple tree copy preserving paths.
# Similar to 'cp --parents' but works with absolute src
# and destination paths.
#
############################################################
SRCDIR=$1
SRCPATH=$2
DSTDIR=$3
DSTPATH=$(dirname "${SRCPATH}")

echo "Copying tree ${SRCPATH}..."
mkdir -p "${DSTDIR}/${DSTPATH}"
cp -R "${SRCDIR}/${SRCPATH}" "${DSTDIR}/${DSTPATH}"

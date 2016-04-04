#! /bin/sh
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
#
#
############################################################
set -e

KERNDIR=$1
PATCHDIR=$2

if [ -f "${PATCHDIR}/series" ]; then
    #
    # The series file contains the patch order.
    #
    for p in `cat ${PATCHDIR}/series`; do
        echo "Appying ${p}..."
        if [ -x "${PATCHDIR}/${p}" ]; then
            "${PATCHDIR}/${p}" "${KERNDIR}"
        else
            patch --batch -p 1 -d ${KERNDIR} < "${PATCHDIR}/${p}"
        fi
    done
else
    #
    # Patch order is implied by filenames.
    #
    for p in `ls ${PATCHDIR}/*.patch | sort`; do
        echo "Applying ${p}..."
        patch --batch -p 1 -d ${KERNDIR} < ${p}
    done
fi

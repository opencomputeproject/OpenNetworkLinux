######################################################################
##
## pyroute2.mk
##
######################################################################

PYTHON_PYROUTE2_VERSION			= 0.3.14
PYTHON_PYROUTE2_SOURCE			= pyroute2-$(PYTHON_PYROUTE2_VERSION).tar.gz
PYTHON_PYROUTE2_SITE			= https://pypi.python.org/packages/source/p/pyroute2/
PYTHON_PYROUTE2_LICENSE			= GPLv2+ and Apache v2
PYTHON_PYROUTE2_LICENSE_FILES		= \
  READEME.licence.md \
  LICENSE.GPL.v2 \
  LICENSE.Apache.v2 \
  # THIS LINE INTENTIONALLY LEFT BLANK
PYTHON_PYROUTE2_INSTALL_STAGING		= NO
PYTHON_PYROUTE2_INSTALL_TARGET		= YES

PYTHON_PYROUTE2_DEPENDENCIES		= python

define PYTHON_PYROUTE2_BUILD_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py build)
endef

define PYTHON_PYROUTE2_INSTALL_TARGET_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py install --prefix=$(TARGET_DIR)/usr)
endef

$(eval $(generic-package))

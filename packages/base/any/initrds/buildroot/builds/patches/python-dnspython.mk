######################################################################
##
## dnspython.mk
##
######################################################################

PYTHON_DNSPYTHON_VERSION		= 1.12.0
PYTHON_DNSPYTHON_SOURCE			= dnspython-$(PYTHON_DNSPYTHON_VERSION).tar.gz
PYTHON_DNSPYTHON_SITE			= http://www.dnspython.org/kits/1.12.0/
PYTHON_DNSPYTHON_INSTALL_STAGING	= NO
PYTHON_DNSPYTHON_INSTALL_TARGET		= YES
PYTHON_DNSPYTHON_LICENSE		= BSD-style
PYTHON_DNSPYTHON_LICENSE_FILES		= LICENSE

PYTHON_DNSPYTHON_DEPENDENCIES		= python

define PYTHON_DNSPYTHON_BUILD_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py build)
endef

define PYTHON_DNSPYTHON_INSTALL_TARGET_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py install --prefix=$(TARGET_DIR)/usr)
endef

$(eval $(generic-package))

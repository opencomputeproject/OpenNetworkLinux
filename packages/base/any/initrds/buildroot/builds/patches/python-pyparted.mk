######################################################################
##
## python-pyparted.mk
##
######################################################################

PYTHON_PYPARTED_VERSION			= 3.10.7
PYTHON_PYPARTED_SOURCE			= pyparted-$(PYTHON_PYPARTED_VERSION).tar.bz2
##PYTHON_PYPARTED_SITE			= http://pyyaml.org/download/pyyaml/
PYTHON_PYPARTED_INSTALL_STAGING		= NO
PYTHON_PYPARTED_INSTALL_TARGET		= YES
PYTHON_PYPARTED_LICENSE			= GPL
PYTHON_PYPARTED_LICENSE_FILES		= COPYING

PYTHON_PYPARTED_DEPENDENCIES		= python parted

PYTHON_PYPARTED_INCLUDES			= \
  --include-dirs $(STAGING_DIR)/usr/include:$(STAGING_DIR)/usr/include/python$(PYTHON_VERSION_MAJOR) \
  # THIS LINE INTENTIONALLY LEFT BLANK

PYTHON_PYPARTED_LIBDIRS			= \
  --library-dirs $(STAGING_DIR)/usr/lib \
  # THIS LINE INTENTIONALLY LEFT BLANK

# see python-mad.mk
PYTHON_PYPARTED_ENVIRONMENT			= \
  CC="$(TARGET_CC)"               \
  CFLAGS="$(TARGET_CFLAGS)"       \
  LDSHARED="$(TARGET_CC) -shared" \
  LDFLAGS="$(TARGET_LDFLAGS)"     \
  # THIS LINE INTENTIONALLY LEFT BLANK

define PYTHON_PYPARTED_BUILD_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py build_py)
  (cd $(@D); $(PYTHON_PYPARTED_ENVIRONMENT) $(HOST_DIR)/usr/bin/python setup.py build_ext $(PYTHON_PYPARTED_INCLUDES) $(PYTHON_PYPARTED_LIBDIRS))
endef

define PYTHON_PYPARTED_INSTALL_TARGET_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py install --prefix=$(TARGET_DIR)/usr)
endef

$(eval $(generic-package))

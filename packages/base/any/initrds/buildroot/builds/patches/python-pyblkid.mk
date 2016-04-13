######################################################################
##
## python-pyblkid.mk
##
######################################################################

PYTHON_PYBLKID_VERSION			= 0.0.1
PYTHON_PYBLKID_SOURCE			= pyblkid-$(PYTHON_PYBLKID_VERSION).tar.bz2
PYTHON_PYBLKID_INSTALL_STAGING		= NO
PYTHON_PYBLKID_INSTALL_TARGET		= YES
PYTHON_PYBLKID_LICENSE			= GPL
PYTHON_PYBLKID_LICENSE_FILES		= COPYING

PYTHON_PYBLKID_DEPENDENCIES		= python util-linux

PYTHON_PYBLKID_INCLUDES			= \
  --include-dirs $(STAGING_DIR)/usr/include:$(STAGING_DIR)/usr/include/python$(PYTHON_VERSION_MAJOR) \
  # THIS LINE INTENTIONALLY LEFT BLANK

PYTHON_PYBLKID_LIBDIRS			= \
  --library-dirs $(STAGING_DIR)/usr/lib \
  # THIS LINE INTENTIONALLY LEFT BLANK

# see python-mad.mk
PYTHON_PYBLKID_ENVIRONMENT			= \
  CC="$(TARGET_CC)"               \
  CFLAGS="$(TARGET_CFLAGS)"       \
  LDSHARED="$(TARGET_CC) -shared" \
  LDFLAGS="$(TARGET_LDFLAGS)"     \
  # THIS LINE INTENTIONALLY LEFT BLANK

define PYTHON_PYBLKID_BUILD_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py build_py)
  (cd $(@D); $(PYTHON_PYBLKID_ENVIRONMENT) $(HOST_DIR)/usr/bin/python setup.py build_ext $(PYTHON_PYBLKID_INCLUDES) $(PYTHON_PYBLKID_LIBDIRS))
endef

define PYTHON_PYBLKID_INSTALL_TARGET_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py install --prefix=$(TARGET_DIR)/usr --install-scripts=$(TARGET_DIR)/usr/bin)
endef

$(eval $(generic-package))

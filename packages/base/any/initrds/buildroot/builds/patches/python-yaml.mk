######################################################################
##
## python-yaml.mk
##
######################################################################

PYTHON_YAML_VERSION			= 3.11
PYTHON_YAML_SOURCE			= PyYAML-$(PYTHON_YAML_VERSION).tar.gz
PYTHON_YAML_SITE			= http://pyyaml.org/download/pyyaml/
PYTHON_YAML_INSTALL_STAGING		= NO
PYTHON_YAML_INSTALL_TARGET		= YES
PYTHON_YAML_LICENSE			= MIT
PYTHON_YAML_LICENSE_FILES		= LICENSE

PYTHON_YAML_DEPENDENCIES		= python libyaml

PYTHON_YAML_INCLUDES			= \
  --include-dirs $(STAGING_DIR)/usr/include:$(STAGING_DIR)/usr/include/python$(PYTHON_VERSION_MAJOR) \
  # THIS LINE INTENTIONALLY LEFT BLANK

PYTHON_YAML_LIBDIRS			= \
  --library-dirs $(STAGING_DIR)/usr/lib \
  # THIS LINE INTENTIONALLY LEFT BLANK

# see python-mad.mk
PYTHON_YAML_ENVIRONMENT			= \
  CC="$(TARGET_CC)"               \
  CFLAGS="$(TARGET_CFLAGS)"       \
  LDSHARED="$(TARGET_CC) -shared" \
  LDFLAGS="$(TARGET_LDFLAGS)"     \
  # THIS LINE INTENTIONALLY LEFT BLANK

define PYTHON_YAML_BUILD_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py build_py)
  (cd $(@D); $(PYTHON_YAML_ENVIRONMENT) $(HOST_DIR)/usr/bin/python setup.py build_ext $(PYTHON_YAML_INCLUDES) $(PYTHON_YAML_LIBDIRS))
endef

define PYTHON_YAML_INSTALL_TARGET_CMDS
  (cd $(@D); $(HOST_DIR)/usr/bin/python setup.py install --prefix=$(TARGET_DIR)/usr)
endef

$(eval $(generic-package))

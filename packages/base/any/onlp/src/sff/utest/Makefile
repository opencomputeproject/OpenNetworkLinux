###############################################################################
#
#
#
###############################################################################
include $(ONL)/make/config.amd64.mk
MODULE := sff_utest
TEST_MODULE := sff
DEPENDMODULES := AIM BigList cjson_util cjson IOF
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MODULES_INIT=1
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MAIN=1
GLOBAL_CFLAGS += -DSFF_CONFIG_INCLUDE_EXT_CC_CHECK=1
GLOBAL_CFLAGS += -DSFF_CONFIG_INCLUDE_DATABASE=1
GLOBAL_LINK_LIBS += -lrt -lpthread -lm
include $(BUILDER)/build-unit-test.mk

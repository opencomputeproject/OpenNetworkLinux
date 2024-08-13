###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_cls_ds4101_INCLUDES := -I $(THIS_DIR)inc
x86_64_cls_ds4101_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_cls_ds4101_DEPENDMODULE_ENTRIES := init:x86_64_cls_ds4101 ucli:x86_64_cls_ds4101


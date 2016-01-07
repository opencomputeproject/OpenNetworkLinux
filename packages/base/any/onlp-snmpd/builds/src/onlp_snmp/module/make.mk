###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
onlp_snmp_INCLUDES := -I $(THIS_DIR)inc
onlp_snmp_INTERNAL_INCLUDES := -I $(THIS_DIR)src
onlp_snmp_DEPENDMODULE_ENTRIES := init:onlp_snmp ucli:onlp_snmp snmp_subagent:onlp_snmp
onlp_snmp_GLOBAL_LINK_LIBS += -lnetsnmpagent -lnetsnmphelpers -lnetsnmpmibs -lnetsnmp

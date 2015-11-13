######################################################################
##
## jq
##
######################################################################

JQ_VERSION		= 1.3
JQ_SOURCE		= jq-$(JQ_VERSION).tar.gz
JQ_SITE			= http://stedolan.github.io/jq/download/source
JQ_INSTALL_STAGING	= NO
JQ_INSTALL_TARGET	= YES
JQ_CONF_OPT		= --disable-docs

$(eval $(autotools-package))

############################################################
#
# When running the ONL builder in isolation mode
# $HOME is set to the root of the ONL tree from
# which you entered the builder.
#
# The assumption here is that you are trying to build
# the tree from which you entered the builder, and
# thus we automatically source the setup.env file
# upon starting the shell inside the container.
#
############################################################

. setup.env

if [ -n "$ONL_AUTOBUILD" ]; then
    make all
else
    echo Ready to build OpenNetworkLinux.
fi

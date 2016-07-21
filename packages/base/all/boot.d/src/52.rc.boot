#!/bin/sh
#
############################################################
#
# If any rc.boot scripts are present in any mounted
# partition they will be executed at boot time.
#
############################################################
for dir in boot config images data; do
    script=/mnt/onl/$dir/rc.boot
    if [ -x "$script" ]; then
       echo "Executing $script..."
       $script
    fi
done

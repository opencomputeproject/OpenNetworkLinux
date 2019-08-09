############################################################
#
# Add additional dev tools.
#
############################################################
FROM opennetworklinux/builder9:1.4
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && apt-get upgrade -y
RUN xapt -a arm64 libedit-dev ncurses-dev libsensors4-dev libwrap0-dev libssl-dev libsnmp-dev

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

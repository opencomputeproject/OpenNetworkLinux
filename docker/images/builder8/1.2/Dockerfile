FROM opennetworklinux/builder8:1.1
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN dpkg --add-architecture armel

RUN apt-get update && apt-get install -y \
    crossbuild-essential-armel \
    gcc-arm-linux-gnueabi

RUN xapt -a armel libedit-dev ncurses-dev libsensors4-dev libwrap0-dev libssl-dev libsnmp-dev

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

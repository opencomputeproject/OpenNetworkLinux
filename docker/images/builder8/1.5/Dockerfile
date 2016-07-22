############################################################
#
# Update with aarch64 support dependencies.
#
############################################################
FROM opennetworklinux/builder8:1.4
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN dpkg --add-architecture arm64

RUN apt-get update && apt-get install -y \
    crossbuild-essential-arm64 \
    gcc-aarch64-linux-gnu

RUN xapt -a arm64 libedit-dev ncurses-dev libsensors4-dev libwrap0-dev libssl-dev libsnmp-dev

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

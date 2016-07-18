############################################################
#
# Update with ONIE build dependencies.
#
############################################################
FROM opennetworklinux/builder8:1.3
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && apt-get install -y \
    stgit \
    gperf \
    gawk \
    automake \
    libexpat1-dev \
    libtool-bin \
    xorriso

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

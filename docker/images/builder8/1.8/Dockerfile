############################################################
#
# Minor updates.
# - libelf-dev for kernel 4.14
# - cryptsetup-bin
#
############################################################
FROM opennetworklinux/builder8:1.7
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && apt-get install libelf-dev && apt-get install cryptsetup-bin

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

############################################################
#
# Minor updates.
# - libelf-dev for kernel 4.14
# - cryptsetup-bin
#
############################################################
FROM opennetworklinux/builder8:1.8
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

# LTS architecture fixes

RUN sudo sed -i s/'http:\/\/security'/'[arch=amd64,armel] http:\/\/security'/g /etc/apt/sources.list && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade -y && \
    apt-get install -y apt-transport-https

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

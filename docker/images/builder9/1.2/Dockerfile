############################################################
#
# Update Packages for arm64.
# Additional build dependencies.
#
############################################################
FROM opennetworklinux/builder9:1.1
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y apt-transport-https

# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

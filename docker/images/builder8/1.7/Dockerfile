############################################################
#
# Fix broken cross dependencies.
# Update Packages.
# Additional build dependencies.
#
############################################################
FROM opennetworklinux/builder8:1.6
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>
#ENV DEBIAN_FRONTEND noninteractive

RUN apt-get install -f && \
    apt-get update

RUN xapt -a powerpc libsnmp-dev && \
    xapt -a armel   libsnmp-dev && \
    xapt -a arm64   libsnmp-dev

RUN apt-get install -f

RUN DEBIAN_FRONTEND=noninteractive apt-get install tshark -y
RUN sudo apt-get install -yq libpcap-dev libxml2-dev python-dev g++ swig tcpreplay libusb-dev

# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

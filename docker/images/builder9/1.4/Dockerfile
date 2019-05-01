############################################################
#
# Add additional dev tools.
#
############################################################
FROM opennetworklinux/builder9:1.3
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y \
    bison byacc ctags flex glib-networking glib-networking-common glib-networking-services libglib2.0-0 libpackagekit-glib2-dev libexpat1 libexpat1-dev libexpat-gst libexpat-ocaml libexpat-ocaml-dev \
    libdb-dev devscripts debhelper iptables-dev devscripts flex libglib2.0-dev bison expat libexpat1-dev dpatch libpcre3 libpcre3-dev python-dev swig libelf-dev

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

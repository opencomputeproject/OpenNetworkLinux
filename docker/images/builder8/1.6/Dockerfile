############################################################
#
# Update with FRR support dependencies.
#
############################################################
FROM opennetworklinux/builder8:1.5
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && apt-get install -y \
chrpath devscripts dh-autoreconf dh-systemd flex \
hardening-wrapper libcap-dev libc-ares-dev libjson0 \
libjson0-dev libjson-c-dev libpam0g-dev libpcre3-dev \
libreadline-dev libsystemd-dev pkg-config \
texlive-generic-recommended texinfo texlive-latex-base

RUN xapt -a powerpc chrpath hardening-wrapper \
libcap-dev libc-ares-dev libjson0 libjson0-dev \
libjson-c-dev libpam0g-dev libpcre3-dev libreadline-dev \
libsystemd-dev pkg-config texinfo libreadline6-dev \
libtext-unidecode-perl libintl-perl libxml-libxml-perl

RUN xapt -a arm64 chrpath groff hardening-wrapper \
libcap-dev libc-ares-dev libjson0 libjson0-dev \
libjson-c-dev libpam0g-dev libpcre3-dev libreadline-dev \
libsystemd-dev pkg-config texinfo libreadline6-dev \
libintl-perl libxml-libxml-perl

RUN xapt -a armel chrpath groff hardening-wrapper \
libcap-dev libc-ares-dev libjson0 libjson0-dev \
libjson-c-dev libpam0g-dev libpcre3-dev libreadline-dev \
libsystemd-dev pkg-config texinfo libreadline6-dev \
libintl-perl libxml-libxml-perl

# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

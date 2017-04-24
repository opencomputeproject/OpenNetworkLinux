############################################################
#
# Update with FRR support dependencies.
#
############################################################
FROM opennetworklinux/builder8:1.5
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && apt-get install -y \
libreadline-dev texinfo libjson-c-dev pkg-config flex \
python-pip libc-ares-dev python3-dev texlive-latex-base \
texlive-generic-recommended libpam0g-dev libpam0g-dev \
libcap-dev hardening-wrapper libpcre3-dev chrpath \
dh-autoreconf libjson0 libjson0-dev dh-systemd \
libsystemd-dev python-ipaddr groff devscripts

# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

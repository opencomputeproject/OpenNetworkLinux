FROM debian:9.1
MAINTAINER Jeff Townsend <jeffrey.townsend@bigswitch.com>

# First round of dependences
RUN apt-get update && apt-get install -y \
        apt \
        apt-cacher-ng \
        apt-file \
        apt-utils \
        autoconf \
	automake \
        autotools-dev \
	bash-completion \
        bc \
        bind9-host \
        binfmt-support \
        binfmt-support \
        bison \
        bsdmainutils \
        build-essential \
        ccache \
        cdbs \
        cpio \
        cryptsetup-bin \
        debhelper \
        debhelper \
        debhelper \
	device-tree-compiler \
        devscripts \
        devscripts \
        dialog \
        dosfstools \
        doxygen \
        dpkg-sig \
        emacs \
        file \
        flex \
        gawk \
        gcc \
        gdb \
        genisoimage \
	git \
        gperf \
        ifupdown \
        iproute \
        iputils-ping \
	isolinux \
        kmod \
        less \
        libc6-dev \
	libcurl4-nss-dev \
	libdouble-conversion-dev \
        libedit-dev \
	libevent-dev \
        libexpat1-dev \
        libgoogle-glog-dev \
        libi2c-dev \
	libkrb5-dev \
	libnuma-dev \
	libsasl2-dev \
	libsnappy-dev \
	libpam-dev \
        libpcap-dev \
        libsnmp-dev \
	libssl-dev \
        libtool \
        libtool-bin \
        locales \
        lsof \
        make \
        mingetty \
        mtd-utils \
        mtools \
        multistrap \
        nano \
        ncurses-dev \
        netbase \
        net-tools \
        nfs-common \
        openssh-server \
        pkg-config \
        pkg-config \
        procps \
        psmisc \
        python \
        python-debian \
        python-dnspython \
        python-yaml \
        qemu \
        qemu-user-static \
        realpath \
        realpath \
        rsyslog \
	ruby \
	ruby-dev \
	rubygems \
	screen \
        squashfs-tools \
        sshpass \
        stgit \
        sudo \
	syslinux-utils \
        telnet \
        texinfo \
        traceroute \
	u-boot-tools \
        vim-tiny \
        wget \
        xorriso \
        zile \
        zip

RUN apt-get install -y \
chrpath devscripts dh-autoreconf dh-systemd flex \
libcap-dev libc-ares-dev libjson-c-dev libpam0g-dev libpcre3-dev \
libreadline-dev libsystemd-dev pkg-config \
texlive-generic-recommended texinfo texlive-latex-base

RUN     gem install --version 1.3.3 fpm

#
# The i2c-dev.h user/kernel header conflict is a nightmare.
#
# The ONLP implementation expects a new file called <linux/i2c-device.h> to be in place which contains the correct user-space driver definitions.
# This should be manually populated here after the toolchains have been installed.
#
RUN cp /usr/include/linux/i2c-dev.h /usr/include/linux/i2c-devices.h

RUN rm /etc/apt/apt.conf.d/docker-* && \
    wget "https://launchpad.net/ubuntu/+source/qemu/1.4.0+dfsg-1expubuntu3/+build/4336762/+files/qemu-user-static_1.4.0%2Bdfsg-1expubuntu3_amd64.deb" && \
    dpkg -i qemu-user-static_1.4.0+dfsg-1expubuntu3_amd64.deb

#
# Copy the docker shell init script to /bin
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

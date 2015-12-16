FROM debian:7.8
MAINTAINER Rob Sherwood <rob.sherwood@bigswitch.com>

# First round of dependences
RUN apt-get update && apt-get install -y \
        apt \
        apt-cacher-ng \
        apt-file \
        apt-utils \
        autoconf \
	automake1.9 \
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
        debhelper \
        debhelper \
        debhelper \
	device-tree-compiler \
        devscripts \
        devscripts \
        dialog \
        dosfstools \
        dpkg-sig \
        emacs \
        file \
        flex \
        gcc \
        genisoimage \
        ifupdown \
        iproute \
        iputils-ping \
        kmod \
        less \
        libc6-dev \
        libedit-dev \
	libevent-dev \
        libi2c-dev \
	libpam-dev \
        libsnmp-dev \
        libtool \
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
        python-dnspython \
        python-yaml \
        qemu \
        qemu-user-static \
        realpath \
        realpath \
        rsyslog \
	rubygems \
	screen \
        squashfs-tools \
        sshpass \
        sudo \
	syslinux \
	texinfo=4.13a.dfsg.1-10 \
        traceroute \
	uboot-mkimage \
        vim-tiny \
        wget \
        xapt \
        zile \
        zip && \
        gem install --version 1.3.3 fpm

# Now the unstable deps and cross compilers
# NOTE 1: texinfo 5.x and above breaks the buildroot build, thus the specific 4.x version
# NOTE 2: this cp is needed to fix an i2c compile problem
# NOTE 3: the /etc/apt/apt.conf.d/docker-* options break multistrap so
#       that it can't find.  Essential packages or resolve apt.opennetlinux.org
# NOTE 4: the default qemu-user-static (1.2) dies with a segfault in
#       `make onl-powerpc`; use 1.4 instead

RUN echo 'APT::Get::AllowUnauthenticated "true";\nAPT::Get::Assume-Yes "true";' | tee /etc/apt/apt.conf.d/99opennetworklinux && \
    echo "deb http://apt.opennetlinux.org/debian/ unstable main" | tee /etc/apt/sources.list.d/opennetlinux.list && \
    dpkg --add-architecture powerpc && \
    apt-get update && \
    apt-get install -y  \
        binutils-powerpc-linux-gnu=2.22-7.1 \
        gcc-4.7-powerpc-linux-gnu \
        libc6-dev-powerpc-cross \
        libgomp1-powerpc-cross=4.7.2-4  && \
    xapt -a powerpc libedit-dev ncurses-dev libsensors4-dev libwrap0-dev libssl-dev libsnmp-dev libevent-dev libpam-dev && \
    update-alternatives --install /usr/bin/powerpc-linux-gnu-gcc powerpc-linux-gnu-gcc /usr/bin/powerpc-linux-gnu-gcc-4.7 10 && \
    rm /etc/apt/apt.conf.d/docker-* && \
    wget "https://launchpad.net/ubuntu/+source/qemu/1.4.0+dfsg-1expubuntu3/+build/4336762/+files/qemu-user-static_1.4.0%2Bdfsg-1expubuntu3_amd64.deb" && \
    dpkg -i qemu-user-static_1.4.0+dfsg-1expubuntu3_amd64.deb && \
    apt-get install python-pyroute2

#
# The i2c-dev.h user/kernel header conflict is a nightmare.
#
# The ONLP implementation expects a new file called <linux/i2c-device.h> to be in place which contains the correct user-space driver definitions.
# This should be manually populated here after the toolchains have been installed.
#
RUN cp /usr/include/linux/i2c-dev.h /usr/include/linux/i2c-devices.h && \
    cp /usr/include/linux/i2c-dev.h /usr/powerpc-linux-gnu/include/linux/i2c-devices.h


#
# Necessary toolchain symlinks
#
RUN ln -s /usr/bin/ar /usr/bin/x86_64-linux-gnu-ar && \
    ln -s /usr/bin/ld /usr/bin/x86_64-linux-gnu-ld && \
    ln -s /usr/bin/objcopy /usr/bin/x86_64-linux-gnu-objcopy

#
# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

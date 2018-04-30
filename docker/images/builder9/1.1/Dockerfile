############################################################
#
# Update Packages for arm64.
# Additional build dependencies.
#
############################################################
FROM opennetworklinux/builder9:1.0
MAINTAINER Jeffrey Townsend <jeffrey.townsend@bigswitch.com>

RUN apt-get update && apt-get install -y \
    crossbuild-essential-arm64 \
    gcc-aarch64-linux-gnu

RUN dpkg --add-architecture arm64 && \
    echo 'deb [arch=arm64] http://deb.debian.org/debian stretch main'  >>  /etc/apt/sources.list && \
    echo 'deb [arch=arm64] http://deb.debian.org/debian stretch-updates main'  >>  /etc/apt/sources.list
RUN apt-get update && apt-get install -y \
    libssl-dev:arm64 \
    libnuma-dev:arm64 \
    libssl-dev:arm64

RUN wget "http://ftp.us.debian.org/debian/pool/main/e/emdebian-crush/xapt_2.2.19_all.deb" && \
	dpkg -i xapt_2.2.19_all.deb && rm xapt_2.2.19_all.deb
RUN xapt -a arm64 libsnmp-dev

RUN wget http://www.opennetlinux.org/tarballs/usr-bin-qemu-aarch64-static.tgz && tar -C / -xvzf usr-bin-qemu-aarch64-static.tgz && rm usr-bin-qemu-aarch64-static.tgz

# Docker shell and other container tools.
#
COPY docker_shell /bin/docker_shell
COPY container-id /bin/container-id

#How to Build Open Network Linux 

In case you are not interested in building ONL from scratch
(it takes a while) you can download pre-compiled binaries from
http://opennetlinux.org/binaries .


Build Hosts and Environments
------------------------------------------------------------
ONL builds with Docker so the only requirements on the build system is:

- docker			# to grab the build workspace
- binfmt-support		# kernel support for ppc builds
- About 40G of disk free space 	# to build all images
- At least 4G of ram and 4G of swap # compilation is memory intensive

All of the testing is done with Debian, other Linux distributions may work, but we suggest using Debian 8.
    # apt-get install binfmt-support
    then follow the instructions at: https://docs.docker.com/engine/installation/debian/


Build ONL Summary
------------------------------------------------------------
The easiest way to build is to use the make docker command:

    #> git clone https://github.com/opencomputeproject/OpenNetworkLinux
    #> cd OpenNetworkLinux
    #> make docker

This will build a Debian 7 based ONL from the master branch

To build a Debian 8 based ONL run:

    #> git clone https://github.com/opencomputeproject/OpenNetworkLinux
    #> cd OpenNetworkLinux
    #> export VERSION=8
    #> make docker
    

If you would like to build by hand you can do the following:

    #> git clone https://github.com/opencomputeproject/OpenNetworkLinux
    #> cd OpenNetworkLinux
    #> docker/tools/onlbuilder (-8)             # enter the docker workspace
    #> apt-cacher-ng
    #> source setup.env                         # pull in necessary environment variables
    #> make amd64 ppc                           # make onl for $platform (currently amd64 or powerpc)

The resulting ONIE installers are in
`$ONL/RELEASE/$SUITE/$ARCH/ONL-2.*INSTALLER`, i.e. 
`RELEASE/jessie/amd64/ONL-2.0.0_ONL-OS_2015-12-12.0252-ffce159_AMD64_INSTALLER`
and the SWI files (if you want them) are in
`$ONL/RELEASE/$SUITE/$ARCH/ONL*.swi`. i.e.
`RELEASE/jessie/amd64/ONL-2.0.0_ONL-OS_2015-12-12.0252-ffce159_AMD64.swi`



#Installing Docker Gotchas

Docker installer oneliner (for reference: see docker.com for details)

    # wget -qO- https://get.docker.com/ | sh


Common docker related issues:

- Check out http://docs.docker.com/installation/debian/ for detailed instructions
- You may have to update your kernel to 3.10+
- Beware that `apt-get install docker` installs a dock application not docker :-)  You want the lxc-docker package instead.
- Some versions of docker are unhappy if you use a local DNS caching resolver:
	- e.g., you have 127.0.0.1 in your /etc/resolv.conf
        - if you have this, specify `DNS="--dns 8.8.8.8"` when you enter the docker environment
 	- e.g., `make DNS="--dns 8.8.8.8" docker`

Consider enabling builds for non-privileged users with:

- `sudo usermod -aG docker $USER`
- If you run as non-root without this, you will get errors like `..: dial unix /var/run/docker.sock: permission denied`
- Building as root is fine as well (it immediately jumps into a root build shell), so this optional
    
#Additional Build Details
----------------------------------------------------------

The rest of this guide talks about how to build specific 
sub-components of the ONL ecosystem and tries to overview
all of the various elements of the build.

Build all .deb packages for all architectures
----------------------------------------------------------
    #> cd $ONL/packages
    #> make
    #> find $ONL/REPO -name \*.deb    # all of the .deb files end up here

A number of things will happen automatically, including:

- git submodule checkouts and updates for kernel, loader, and additional code repositories
- automatic builds of all debian packages and their dependencies
- automatic download of binary-only .deb packages from apt.opennetlinux.org

After all components have been built, your can build an ONL
Software Image from those components.

Adding/Removing packages from a SWI:
------------------------------------------------------------

The list of packages for a given SWI are in

    $ONL/builds/any/rootfs/jessie/common/*.yml

The "all-base-packages.yml" file is for all architectures and the rest are architecture specific package lists.


Example setup on new Debian 8.2 installation
------------------------------------------------------------
Install sudo and add yourself to the sudoers: 

As root:
```
apt-get install sudo
vi /etc/sudoers.d/username
```

Add the line:
```
username    ALL=(ALL:ALL) ALL
```

Add the docker key:
```
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D

gpg: key 2C52609D: public key "Docker Release Tool (releasedocker) <docker@docker.com>" imported
gpg: Total number processed: 1
gpg:               imported: 1  (RSA: 1)
```

Install necessary items, make, binfmt-support and apt-transport-https (for docker):
```
sudo apt-get install apt-transport-https make binfmt-support
```

Add the docker repository to your system:
```
sudo vi /etc/apt/sources.list.d/docker.list
```
Add the following line to the file:
```
deb https://apt.dockerproject.org/repo debian-jessie main
```

Install Docker:
```
sudo apt-get update
sudo apt-get install docker-engine
```

Test Docker:
```
sudo docker run hello-world

Unable to find image 'hello-world:latest' locally
latest: Pulling from library/hello-world
b901d36b6f2f: Pull complete
0a6ba66e537a: Pull complete
Digest: sha256:8be990ef2aeb16dbcb9271ddfe2610fa6658d13f6dfb8bc72074cc1ca36966a7
Status: Downloaded newer image for hello-world:latest

Hello from Docker.
This message shows that your installation appears to be working correctly.
```

Add yourself to the docker group:
```
sudo gpasswd -a user1 docker

Adding user user1 to group docker
```

logout and log back in for the group to take effect:

Clone the OpenNetworkLinux repository:
```
git clone https://github.com/opencomputeproject/OpenNetworkLinux.git

Cloning into 'OpenNetworkLinux'...
Checking connectivity... done.
```

Build OpenNetworkLinux:

    #> cd OpenNetworkLinux/
    #> make docker
    #> Pulling opennetworklinux/builder7:1.0…

Or:

    #> docker/tools/onlbuilder
    #> source setup.env
    #> apt-cacher-ng
    #> make onl-x86 onl-ppc


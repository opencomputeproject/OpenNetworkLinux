Summary
---------

The high-level boot process for ONL is fairly straight forward, but there is a lot detail.

At high-level, there are three phases
1. uBoot phase
2. the ONL Loader phase
3. The final ONL operating system


Detailed Boot Process
--------------------------

1. uBoot is the first level boot loader: http://www.denx.de/wiki/U-Boot
2. uBoot reads the 'nos_bootcmd' environmental variable from flash and runs the contents
    ('nos' is Network Operating System)
4. If $nos_boot_cmd returns, uBoot loads and runs ONIE (see below) to download the ONL installer and install the ONL loader
    a) The factory default $nos_boot_cmd is to a trival command that returned immediately, e.g., 'echo'
5. In normal operation, i.e., after ONIE has been run, $nos_boot_cmd is set to load and run the ONL Loader
6. The ONL loader boots the Linux kernel (later, the "boot kernel") 
7. The ONL loader decides which SWI to run based on the URL in the file /etc/SWI
    URL=`cat /etc/SWI`
8. The ONL loader runs `/bin/boot $URL`
9. The ONL loader retrieves the SWI file
    a) if the URL is remote (e.g., http://, ftp://, etc.), verify that there is a locally cached copy
        of the SWI in /mnt/onl/images or if not, download it
    b) if the URL is local, verify that the device is accessible
    c) if the URL is a Zero Touch Networking (ZTN) URL, the execute the ZTN protocol to get the SWI (see below)
10. The ONL loader reads the 'rootfs' file out of the SWI and mounts it using overlayfs[1] (SWI contents described below)





Partition Layout
------------------

Switches typically have two flash storage device: a smaller flash (e.g.,
64MB flash) for booting and a larger, mass storage device (e.g., compact
flash, 2+GB).


Smaller Boot Flash:

Partition 1: uBoot
Partition 2: environmental variables (e.g., $nos_boot_cmd)
Partition 3: ONIE
Partition 4+: Free space (unused)

Mass Storage Device:

Partition 1: ONL loader kernel  -- the format of this partition varies depending on what formats uBoot supports on the specific platform
Partition 2: ONL Loader configuration files (mounts as "/mnt/onl/boot" both during the loader and the main ONL phases)
Partition 3: ONL SWitch Images (SWIs) partition (mounts as "/mnt/onl/images" both during the loader and the main ONL phases)

ONL file system layout
-----------------------

    root@as5712-2:/mnt/onl/images# df
    Filesystem     1K-blocks   Used Available Use% Mounted on
    rootfs           1215292 145904   1069388  13% /
    devtmpfs            1024      0      1024   0% /dev
    none             1215292 145904   1069388  13% /
    /dev/sdb5        1032088 276700    702960  29% /mnt/onl/images
    /dev/sdb6        6313528 143612   5849200   3% /mnt/onl/data
    /dev/sdb3         126931  37007     83371  31% /mnt/onl/boot
    /dev/sdb4         126931   5651    114727   5% /mnt/onl/config
    tmpfs             810196    208    809988   1% /run
    tmpfs               5120      0      5120   0% /run/lock
    tmpfs            1620380      0   1620380   0% /run/shm

SWI
--------

Zip file contains:

    $ unzip -l ONL-2.0.0_ONL-OS_2015-12-12.0252-ffce159_PPC.swi
    Archive:  ONL-2.0.0_ONL-OS_2015-12-12.0252-ffce159_PPC.swi
    Length      Date    Time    Name
    ---------  ---------- -----   ----
     97968128  2015-12-15 20:20   rootfs-powerpc.sqsh
     1063      2015-12-15 20:20   manifest.json
    ---------                     -------
     97969191                     2 files

1. 'rootfs-$ARCH'   : the root file system for the running ONL
2. 'manifest.json'  : a list of supported platforms, version information about ONL and the architecture




Footnotes
-----------

[1] : https://kernel.googlesource.com/pub/scm/linux/kernel/git/mszeredi/vfs/+/overlayfs.current/Documentation/filesystems/overlayfs.txt

# Rationale

By default, the ONL file system is NOT persistent, meaning that if you
reboot, your changes will dissapear (!!).  While this may sound suboptimal
at first, it does have the incredibly nice property of ensuring that many
classes of configuration and other problems can go away with a reboot.
This is particularly nice when you have a switch that may be headless
(no permanently connected console cable or keyboard).

ONL accomplishes this with OverlayFS
(https://www.kernel.org/doc/Documentation/filesystems/overlayfs.txt).
As described at http://opennetlinux.org/docs/bootprocess, the ONL
switch image (.SWI file) contains a read-only root file system image.
The default ONL root file system is then a copy-on-write (using overlayfs)
view into that file system image.

It has the following properites:

* Any file that is editted/removed/etc is transparently copied into a RAM disk via overlayfs
* Thus, any changes to files appear as you would expect, until a reboot
* Any file that is uneditted remains backed by the /mnt/onl/data file system, so you 
    do not need to have enough RAM to store the entire rootfs.  This is important with
    switches that do not have much RAM to begin with.

If you want to persist files, you can either install the image directly to disk using
the Installed installer, put debian files to the /mnt/onl/data/install-debs directory.
Or use rc.boot files to do system management (/mnt/onl/$dir/rc.boot)

Packages present and listed in /mnt/onl/data/install-debs/list will be installed.

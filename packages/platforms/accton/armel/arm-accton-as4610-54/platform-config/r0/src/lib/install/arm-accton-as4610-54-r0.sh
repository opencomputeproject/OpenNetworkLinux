# The loader is installed in the fat partition of the first USB storage device
platform_bootcmd="usb start; usbiddev; ext2load usb 0:1 70000000 $ONL_PLATFORM.itb; setenv bootargs console=\$consoledev,\$baudrate onl_platform=$ONL_PLATFORM; bootm 70000000#$ONL_PLATFORM"

platform_installer() {
    # Standard installation to usb storage
    installer_standard_blockdev_install sda 128M 128M 1024M ""
}

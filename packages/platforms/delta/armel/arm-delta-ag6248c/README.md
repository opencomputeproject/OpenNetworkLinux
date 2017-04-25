#How to run ONL in DELTA AG6248C board

For the first step, it only support install the ONL to the USB and boot up.
It will be support to install the ONL to NandFlash next step.

Build the ONL
--------------------------------------------------------------------------
Please refer the $ONL/docs/Building.md

Install the ONL through ONIE
--------------------------------------------------------------------------
```
ONIE:/ # onie-discovery-stop 
discover: installer mode detected.
Stopping: discover... done.
ONIE:/ # 
ONIE:/ # ifconfig eth0 192.168.1.1   #configure the DUT IP address
ONIE:/ # tftp -r ONL-2.*_ARMEL_INSTALLED_INSTALLER -g 192.168.1.99 -b 10240
ONIE:/ # onie-nos-install ONL-2.*_ARMEL_INSTALLED_INSTALLER
```
Boot the ONL
--------------------------------------------------------------------------
Device will reboot automatically after install the ONL installer successfull.

Now it will start the ONL boot progress.

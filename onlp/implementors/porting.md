# Porting and Testing You Implementation

The ```onlps``` tool is build as a static binary for your platform (like the old ```onlpdump``` program). It builds all of the ONLP core and your platform code together so you can easily build, test, debug, and iterate on your platform.
No shared libraries or setup is required.

## Testing Attributes

While ONLPv1 used the ```sysi``` interface to expose ONIE and Platform information ONLPv2 uses the new attribute interface.
At a minimum you must support the ONIE and Asset attributes on the Chassis OID to port from V1 to V2.

### Testing the ONIE attribute

Run ```onlps chassis onie show```

```
root@as5812x-1:~# onlps chassis onie show
    Product Name: 5812-54X-O-AC-F
    Part Number: FP1ZZ5654035A
    Serial Number: 581254X1721059
    MAC: a8:2b:b5:3a:2a:e6
    Manufacturer: Accton
    Manufacture Date: 06/14/2017 16:24:50
    Vendor: Edgecore
    Platform Name: x86-64-accton-as5812-54x-r0
    Label Revision: R03A
    Country Code: TW
    Diag Version: 1.0.0.4
    Service Tag: None
    ONIE Version: 2018.02.00.02
    Device Version: 0
    CRC: 0x78da4b7e
root@as5812x-1:~#
```

### Testing the Asset attribute

Run ```onlps chassis asset show```

```
root@as5812x-1:~# ./onlps chassis asset show
    Manufacturer: Accton
    Firmware Revision: 9.6.6
root@as5812x-1:~#
```

## Testing the Environment

Run ```onlps chassis env```

```
root@as5812x-1:~# ./onlps chassis env
    Fan 1:
      Description: Chassis Fan 1
      State: Present
      Status: Running
      RPM: 8775
      Speed: 40%
    Fan 2:
      Description: Chassis Fan 2
      State: Present
      Status: Running
      RPM: 8850
      Speed: 41%
    Fan 3:
      Description: Chassis Fan 3
      State: Present
      Status: Running
      RPM: 8850
      Speed: 41%
    Fan 4:
      Description: Chassis Fan 4
      State: Present
      Status: Running
      RPM: 8850
      Speed: 41%
    Fan 5:
      Description: Chassis Fan 5
      State: Present
      Status: Running
      RPM: 8850
      Speed: 41%
    Thermal 1:
      Description: CPU Core
      State: Present
      Status: Functional
      Temperature: 19.0
    Thermal 2:
      Description: Chassis Thermal Sensor 1 (Front middle)
      State: Present
      Status: Functional
      Temperature: 24.5
    Thermal 3:
      Description: Chassis Thermal Sensor 2 (Rear right)
      State: Present
      Status: Functional
      Temperature: 24.0
    Thermal 4:
      Description: Chassis Thermal Sensor 3 (Front right)
      State: Present
      Status: Functional
      Temperature: 22.5
    PSU 1:
      Description: PSU-1
      State: Present
      Status: Failed or Unplugged.
    PSU 2:
      Description: PSU-2
      State: Present
      Status: Running
      Model: CPR-4011-4M11
      Serial: 4011411G23T1751

      Vin: 118.5
      Vout: 12.0
      Iin: 0.6
      Iout: 5.0
      Pin: 74.0
      Pout: 61.0
      Fan 7:
        Description: Chassis PSU-2 Fan 1
        State: Present
        Status: Running
        RPM: 6080
        Speed: 31%
      Thermal 6:
        Description: PSU-2 Thermal Sensor 1
        State: Present
        Status: Functional
        Temperature: 27.0
root@as5812x-1:~#
```

## Testing SFPs

## Inventory

Run ```onlps sfp inventory```

```
root@as5812x-1:~# ./onlps sfp inventory
Port  Type    Module          Media   Status  Len    Vendor            Model             S/N
----  ------  --------------  ------  ------  -----  ----------------  ----------------  ----------------
   1          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420158
   2          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420158
   3          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420305
   4          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420305
   5          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420255
   6          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420255
   7          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420092
   8          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420092
   9          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420315
  10          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420315
  11          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420165
  12          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420165
  13          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420155
  14          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420155
  15          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420115
  16          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420115
  17          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420072
  18          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420072
  19          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420335
  20          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420335
  21          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420334
  22          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420334
  23          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420190
  24          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420190
  25          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420340
  26          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420340
  27          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420124
  28          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420124
  29          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420162
  30          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420162
  31          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420239
  32          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420239
  33          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420070
  34          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420070
  35          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420273
  36          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420273
  37          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420037
  38          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420037
  39          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420164
  40          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420164
  41          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420300
  42          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420300
  43          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420140
  44          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420140
  45          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420159
  46          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420159
  47          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420023
  48          10GBASE-CR      Copper  X       1m     3M                1410-P17-00-1.00  Y10F420023
  49          40GBASE-CR4     Copper          1m     3M Company        9QA0-111-12-1.00  V10F8127
  50          40GBASE-CR4     Copper          1m     3M Company        9QA0-111-12-1.00  V10F8143
  51          40GBASE-CR4     Copper          1m     3M Company        9QA0-111-12-1.00  V10F8127
  52          40GBASE-CR4     Copper          1m     3M Company        9QA0-111-12-1.00  V10F8183
  53          40GBASE-CR4     Copper          1m     3M Company        9QA0-111-12-1.00  V10F8143
  54          40GBASE-CR4     Copper          1m     3M Company        9QA0-111-12-1.00  V10F8183
root@as5812x-1:~#
```
### Bitmaps
 ```run onlps sfp bitmaps```

```
root@as5812x-1:~# ./onlps sfp bitmaps
Presence: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53
RX_LOS: None
root@as5812x-1:~#
```

### Generic Device Reads

```run onlps sfp dev read <port> <devaddr> <addr> <len>```

```
root@as5812x-1:~# ./onlps sfp dev read 49 0x50 0 256
  0000: 0d 00 06 00 00 00 00 00 00 00 00 00 00 00 00 00
  0010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0020: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0030: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0070: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0080: 0d 00 23 08 00 00 00 00 00 00 00 00 64 00 00 00
  0090: 00 00 01 a0 33 4d 20 43 6f 6d 70 61 6e 79 20 20
  00a0: 20 20 20 20 07 08 00 21 39 51 41 30 2d 31 31 31
  00b0: 2d 31 32 2d 31 2e 30 30 30 31 02 03 04 08 00 4d
  00c0: 00 00 00 00 56 31 30 46 38 31 32 37 20 20 20 20
  00d0: 20 20 20 20 31 36 30 38 33 30 20 20 00 00 00 41
  00e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  00f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

root@as5812x-1:~# ./onlps sfp dev read 49 0x50 128 128
  0000: 0d 00 23 08 00 00 00 00 00 00 00 00 64 00 00 00
  0010: 00 00 01 a0 33 4d 20 43 6f 6d 70 61 6e 79 20 20
  0020: 20 20 20 20 07 08 00 21 39 51 41 30 2d 31 31 31
  0030: 2d 31 32 2d 31 2e 30 30 30 31 02 03 04 08 00 4d
  0040: 00 00 00 00 56 31 30 46 38 31 32 37 20 20 20 20
  0050: 20 20 20 20 31 36 30 38 33 30 20 20 00 00 00 41
  0060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0070: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
root@as5812x-1:~#
```









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

## Platform Manager

Run ```onlps platform manager run <seconds>```

```
root@as5812x-1:~# ./onlps platform manager run 60
Running the platform manager for 60 seconds...
01-14 16:41:19.141521 [onlp] Fan 1 is present.
01-14 16:41:19.141656 [onlp] Fan 2 is present.
01-14 16:41:19.141681 [onlp] Fan 3 is present.
01-14 16:41:19.141705 [onlp] Fan 4 is present.
01-14 16:41:19.141728 [onlp] Fan 5 is present.
01-14 16:41:19.141752 [onlp] Fan 7 is present.
01-14 16:41:19.193797 [onlp] PSU 1 is present.
01-14 16:41:19.193851 [onlp] PSU 1 has failed.
01-14 16:41:19.193883 [onlp] PSU 2 is present.
01-14 16:41:28.079092 [x86_64_accton_as5812_54x] Fan Speeds are now at 40%
Stopping the platform manager...01-14 16:42:18.075127 [onlp] Terminating.
done
root@as5812x-1:~#
```

## Fan Get/Set

Run ```onlps fan set rpm|percentage <id> <value>```

```
root@as5812x-1:~# ./onlps fan percentage get 1
fan-1 percentage = 41
root@as5812x-1:~# ./onlps fan percentage set 1 100
OK
root@as5812x-1:~# ./onlps fan percentage get 1
fan-1 percentage = 82
root@as5812x-1:~# ./onlps fan percentage get 1
fan-1 percentage = 86
root@as5812x-1:~# ./onlps fan percentage get 1
fan-1 percentage = 86
root@as5812x-1:~# ./onlps fan percentage get 1
fan-1 percentage = 86
root@as5812x-1:~# ./onlps fan percentage get 1
fan-1 percentage = 90
root@as5812x-1:~#
```

## OID User and Debug JSON Output

### JSON Representation of the User Output

This shows the user JSON version of an OID's info structure:

Run ```onlps oid to json <oid> user```

```
root@as5812x-1:~# ./onlps oid to json psu-2 user
{
	"Description":	"PSU-2",
	"State":	"Present",
	"Status":	"Running",
	"Model":	"CPR-4011-4M11",
	"Serial":	"4011411G23T1751\n",
	"Vin":	"118.5",
	"Vout":	"12.0",
	"Iin":	"1.3",
	"Iout":	"11.7",
	"Pin":	"162.0",
	"Pout":	"141.0",
	"Fan 7":	{
		"Description":	"Chassis PSU-2 Fan 1",
		"State":	"Present",
		"Status":	"Running",
		"RPM":	"5920",
		"Speed":	"30%"
	},
	"Thermal 6":	{
		"Description":	"PSU-2 Thermal Sensor 1",
		"State":	"Present",
		"Status":	"Functional",
		"Temperature":	"40.0"
	}
}
root@as5812x-1:~#
```

### JSON Representation of the full info structure:

This shows the full JSON version of an OID's info structure:

Run ```onlps oid to json <oid> debug```

```
{
	"hdr":	{
		"id":	"psu-2",
		"description":	"PSU-2",
		"poid":	null,
		"coids":	["fan-7", "thermal-6"],
		"status":	["PRESENT"]
	},
	"caps":	["GET_VIN", "GET_VOUT", "GET_IIN", "GET_IOUT", "GET_PIN", "GET_POUT"],
	"mvin":	118500,
	"mvout":	12000,
	"miin":	1367,
	"miout":	11750,
	"mpin":	161000,
	"mpout":	141000
}
```

### Recursive JSON representation

This shows the full JSON version of an OID's info structure and all of its childred recursively:

Run ```onlp oid top json <oid> debug-all```

```
root@as5812x-1:~# ./onlps oid to json psu-2 debug-all
{
	"hdr":	{
		"id":	"psu-2",
		"description":	"PSU-2",
		"poid":	null,
		"coids":	["fan-7", "thermal-6"],
		"status":	["PRESENT"]
	},
	"caps":	["GET_VIN", "GET_VOUT", "GET_IIN", "GET_IOUT", "GET_PIN", "GET_POUT"],
	"mvin":	118500,
	"mvout":	12000,
	"miin":	1367,
	"miout":	11750,
	"mpin":	161000,
	"mpout":	141000,
	"coids":	{
		"fan-7":	{
			"hdr":	{
				"id":	"fan-7",
				"description":	"Chassis PSU-2 Fan 1",
				"poid":	"psu-2",
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
			"rpm":	5920,
			"percentage":	30
		},
		"thermal-6":	{
			"hdr":	{
				"id":	"thermal-6",
				"description":	"PSU-2 Thermal Sensor 1",
				"poid":	"chassis-1",
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_TEMPERATURE"],
			"mcelsius":	41000
		}
	}
}
root@as5812x-1:~#
```

## Chassis Debug Output

You can dump the debug representation for the entire chassis:

Run ```onlps chassis debug show```

```
root@as5812x-1:~# ./onlps chassis debug show
    ONIE Information:
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
    Asset Information:
      Manufacturer: Accton
      Firmware Revision: 9.6.6
    chassis-1:
      hdr:
        id: chassis-1
        description: None
        poid: None
        coids:
          -           psu-1
          -           psu-2
          -           led-1
          -           led-2
          -           led-3
          -           led-4
          -           led-5
          -           led-6
          -           led-7
          -           led-8
          -           led-9
          -           led-10
          -           thermal-1
          -           thermal-2
          -           thermal-3
          -           thermal-4
          -           fan-1
          -           fan-2
          -           fan-3
          -           fan-4
          -           fan-5
          -           sfp-1
          -           sfp-2
          -           sfp-3
          -           sfp-4
          -           sfp-5
          -           sfp-6
          -           sfp-7
          -           sfp-8
          -           sfp-9
          -           sfp-10
          -           sfp-11
          -           sfp-12
          -           sfp-13
          -           sfp-14
          -           sfp-15
          -           sfp-16
          -           sfp-17
          -           sfp-18
          -           sfp-19
          -           sfp-20
          -           sfp-21
          -           sfp-22
          -           sfp-23
          -           sfp-24
          -           sfp-25
          -           sfp-26
          -           sfp-27
          -           sfp-28
          -           sfp-29
          -           sfp-30
          -           sfp-31
          -           sfp-32
          -           sfp-33
          -           sfp-34
          -           sfp-35
          -           sfp-36
          -           sfp-37
          -           sfp-38
          -           sfp-39
          -           sfp-40
          -           sfp-41
          -           sfp-42
          -           sfp-43
          -           sfp-44
          -           sfp-45
          -           sfp-46
          -           sfp-47
          -           sfp-48
          -           sfp-49
          -           sfp-50
          -           sfp-51
          -           sfp-52
          -           sfp-53
          -           sfp-54
        status:
          -           PRESENT
          -           OPERATIONAL
      coids:
        psu-1:
          hdr:
            id: psu-1
            description: PSU-1
            poid: None
            coids:
            status:
              -               PRESENT
              -               FAILED
          caps:
        psu-2:
          hdr:
            id: psu-2
            description: PSU-2
            poid: None
            coids:
              -               fan-7
              -               thermal-6
            status:
              -               PRESENT
          caps:
            -             GET_VIN
            -             GET_VOUT
            -             GET_IIN
            -             GET_IOUT
            -             GET_PIN
            -             GET_POUT
          mvin: 118500
          mvout: 12000
          miin: 1367
          miout: 11750
          mpin: 162000
          mpout: 141000
          coids:
            fan-7:
              hdr:
                id: fan-7
                description: Chassis PSU-2 Fan 1
                poid: psu-2
                coids:
                status:
                  -                   PRESENT
              caps:
                -                 SET_PERCENTAGE
                -                 GET_RPM
                -                 GET_PERCENTAGE
              rpm: 5920
              percentage: 30
            thermal-6:
              hdr:
                id: thermal-6
                description: PSU-2 Thermal Sensor 1
                poid: chassis-1
                coids:
                status:
                  -                   PRESENT
              caps:
                -                 GET_TEMPERATURE
              mcelsius: 41000
        led-1:
          hdr:
            id: led-1
            description: Chassis LED 1 (DIAG LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             OFF
            -             ORANGE
            -             GREEN
          mode: OFF
        led-2:
          hdr:
            id: led-2
            description: Chassis LED 2 (FAN LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             OFF
            -             AUTO
            -             ORANGE
            -             GREEN
          mode: AUTO
        led-3:
          hdr:
            id: led-3
            description: Chassis LED 3 (LOC LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             OFF
            -             ORANGE
            -             ORANGE_BLINKING
          mode: OFF
        led-4:
          hdr:
            id: led-4
            description: Chassis LED 4 (PSU1 LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             OFF
            -             AUTO
            -             ORANGE
            -             GREEN
          mode: AUTO
        led-5:
          hdr:
            id: led-5
            description: Chassis LED 5 (PSU2 LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             OFF
            -             AUTO
            -             ORANGE
            -             GREEN
          mode: AUTO
        led-6:
          hdr:
            id: led-6
            description: Chassis LED 6 (FAN1 LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
          mode: GREEN
        led-7:
          hdr:
            id: led-7
            description: Chassis LED 7 (FAN2 LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
          mode: GREEN
        led-8:
          hdr:
            id: led-8
            description: Chassis LED 8 (FAN3 LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
          mode: GREEN
        led-9:
          hdr:
            id: led-9
            description: Chassis LED 9 (FAN4 LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
          mode: GREEN
        led-10:
          hdr:
            id: led-10
            description: Chassis LED 10 (FAN5 LED)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
          mode: GREEN
        thermal-1:
          hdr:
            id: thermal-1
            description: CPU Core
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             GET_TEMPERATURE
          mcelsius: 14000
        thermal-2:
          hdr:
            id: thermal-2
            description: Chassis Thermal Sensor 1 (Front middle)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             GET_TEMPERATURE
          mcelsius: 22000
        thermal-3:
          hdr:
            id: thermal-3
            description: Chassis Thermal Sensor 2 (Rear right)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             GET_TEMPERATURE
          mcelsius: 22000
        thermal-4:
          hdr:
            id: thermal-4
            description: Chassis Thermal Sensor 3 (Front right)
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             GET_TEMPERATURE
          mcelsius: 21500
        fan-1:
          hdr:
            id: fan-1
            description: Chassis Fan 1
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             SET_PERCENTAGE
            -             GET_RPM
            -             GET_PERCENTAGE
          rpm: 19650
          percentage: 91
        fan-2:
          hdr:
            id: fan-2
            description: Chassis Fan 2
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             SET_PERCENTAGE
            -             GET_RPM
            -             GET_PERCENTAGE
          rpm: 19575
          percentage: 91
        fan-3:
          hdr:
            id: fan-3
            description: Chassis Fan 3
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             SET_PERCENTAGE
            -             GET_RPM
            -             GET_PERCENTAGE
          rpm: 19650
          percentage: 91
        fan-4:
          hdr:
            id: fan-4
            description: Chassis Fan 4
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             SET_PERCENTAGE
            -             GET_RPM
            -             GET_PERCENTAGE
          rpm: 19200
          percentage: 89
        fan-5:
          hdr:
            id: fan-5
            description: Chassis Fan 5
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          caps:
            -             SET_PERCENTAGE
            -             GET_RPM
            -             GET_PERCENTAGE
          rpm: 19575
          percentage: 91
        sfp-1:
          hdr:
            id: sfp-1
            description: SFP 0
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420158
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-2:
          hdr:
            id: sfp-2
            description: SFP 1
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420158
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-3:
          hdr:
            id: sfp-3
            description: SFP 2
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420305
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-4:
          hdr:
            id: sfp-4
            description: SFP 3
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420305
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-5:
          hdr:
            id: sfp-5
            description: SFP 4
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420255
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-6:
          hdr:
            id: sfp-6
            description: SFP 5
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420255
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-7:
          hdr:
            id: sfp-7
            description: SFP 6
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420092
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-8:
          hdr:
            id: sfp-8
            description: SFP 7
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420092
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-9:
          hdr:
            id: sfp-9
            description: SFP 8
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420315
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-10:
          hdr:
            id: sfp-10
            description: SFP 9
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420315
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-11:
          hdr:
            id: sfp-11
            description: SFP 10
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420165
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-12:
          hdr:
            id: sfp-12
            description: SFP 11
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420165
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-13:
          hdr:
            id: sfp-13
            description: SFP 12
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420155
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-14:
          hdr:
            id: sfp-14
            description: SFP 13
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420155
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-15:
          hdr:
            id: sfp-15
            description: SFP 14
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420115
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-16:
          hdr:
            id: sfp-16
            description: SFP 15
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420115
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-17:
          hdr:
            id: sfp-17
            description: SFP 16
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420072
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-18:
          hdr:
            id: sfp-18
            description: SFP 17
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420072
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-19:
          hdr:
            id: sfp-19
            description: SFP 18
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420335
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-20:
          hdr:
            id: sfp-20
            description: SFP 19
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420335
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-21:
          hdr:
            id: sfp-21
            description: SFP 20
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420334
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-22:
          hdr:
            id: sfp-22
            description: SFP 21
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420334
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-23:
          hdr:
            id: sfp-23
            description: SFP 22
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420190
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-24:
          hdr:
            id: sfp-24
            description: SFP 23
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420190
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-25:
          hdr:
            id: sfp-25
            description: SFP 24
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420340
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-26:
          hdr:
            id: sfp-26
            description: SFP 25
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420340
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-27:
          hdr:
            id: sfp-27
            description: SFP 26
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420124
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-28:
          hdr:
            id: sfp-28
            description: SFP 27
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420124
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-29:
          hdr:
            id: sfp-29
            description: SFP 28
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420162
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-30:
          hdr:
            id: sfp-30
            description: SFP 29
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420162
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-31:
          hdr:
            id: sfp-31
            description: SFP 30
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420239
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-32:
          hdr:
            id: sfp-32
            description: SFP 31
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420239
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-33:
          hdr:
            id: sfp-33
            description: SFP 32
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420070
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-34:
          hdr:
            id: sfp-34
            description: SFP 33
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420070
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-35:
          hdr:
            id: sfp-35
            description: SFP 34
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420273
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-36:
          hdr:
            id: sfp-36
            description: SFP 35
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420273
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-37:
          hdr:
            id: sfp-37
            description: SFP 36
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420037
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-38:
          hdr:
            id: sfp-38
            description: SFP 37
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420037
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-39:
          hdr:
            id: sfp-39
            description: SFP 38
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420164
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-40:
          hdr:
            id: sfp-40
            description: SFP 39
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420164
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-41:
          hdr:
            id: sfp-41
            description: SFP 40
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420300
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-42:
          hdr:
            id: sfp-42
            description: SFP 41
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420300
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-43:
          hdr:
            id: sfp-43
            description: SFP 42
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420140
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-44:
          hdr:
            id: sfp-44
            description: SFP 43
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420140
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-45:
          hdr:
            id: sfp-45
            description: SFP 44
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420159
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-46:
          hdr:
            id: sfp-46
            description: SFP 45
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420159
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-47:
          hdr:
            id: sfp-47
            description: SFP 46
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420023
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-48:
          hdr:
            id: sfp-48
            description: SFP 47
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M
            model: 1410-P17-00-1.00
            serial: Y10F420023
            sfp-type: SFP
            module-type: 10GBASE-CR
            media-type: Copper
            caps: F_10G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-49:
          hdr:
            id: sfp-49
            description: SFP 48
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M Company
            model: 9QA0-111-12-1.00
            serial: V10F8127
            sfp-type: QSFP+
            module-type: 40GBASE-CR4
            media-type: Copper
            caps: F_40G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-50:
          hdr:
            id: sfp-50
            description: SFP 51
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M Company
            model: 9QA0-111-12-1.00
            serial: V10F8143
            sfp-type: QSFP+
            module-type: 40GBASE-CR4
            media-type: Copper
            caps: F_40G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-51:
          hdr:
            id: sfp-51
            description: SFP 49
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M Company
            model: 9QA0-111-12-1.00
            serial: V10F8127
            sfp-type: QSFP+
            module-type: 40GBASE-CR4
            media-type: Copper
            caps: F_40G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-52:
          hdr:
            id: sfp-52
            description: SFP 52
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M Company
            model: 9QA0-111-12-1.00
            serial: V10F8183
            sfp-type: QSFP+
            module-type: 40GBASE-CR4
            media-type: Copper
            caps: F_40G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-53:
          hdr:
            id: sfp-53
            description: SFP 50
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M Company
            model: 9QA0-111-12-1.00
            serial: V10F8143
            sfp-type: QSFP+
            module-type: 40GBASE-CR4
            media-type: Copper
            caps: F_40G
            length: 1m
          dom:
            spec: UNSUPPORTED
        sfp-54:
          hdr:
            id: sfp-54
            description: SFP 53
            poid: chassis-1
            coids:
            status:
              -               PRESENT
          type: [Unknown]
          info:
            vendor: 3M Company
            model: 9QA0-111-12-1.00
            serial: V10F8183
            sfp-type: QSFP+
            module-type: 40GBASE-CR4
            media-type: Copper
            caps: F_40G
            length: 1m
          dom:
            spec: UNSUPPORTED
root@as5812x-1:~#
```







This repository is intended to commit sanity tests for Delta Network products python tests

Test Environment: Open Network Linux


Steps to execute:
	1.Configure management interface on switch.
	2.ssh login to the device and make a directory (e.g. onlpd-test)
	3.Execute the command "killall onlpd"
	4.scp copy the scripts(fan_tc1.py ,led_tc1.py,.. and libonlp folder) to the same directory(onlpd-test).
	5.Execution command: python fan_tc1.py

FAN
_____________________________________________________________________________
Functions in libFan.py:

	1.get_fans() :
		returns the list of fans
		Each fan can be accessed by using corresponding indices of the list
    Each fan object will have the following attributes which can be accessed by their corresponding names

    fan_oid:  50331649
    description:  Chassis Fan 1
    rpm:  8057
    status:  5
    caps:  60
    percentage:  42
    model:  ONLP_FAN_MODE_NORMAL

    For e.g.
			fanobj = get_fans()
			print fanobj[0].rpm

	2.get_caps():
		Member function of class fan
		Get the capabilities of a particular fan
		Parameter:Object of the class fan.For e.g. fanobj[0]
		Function call e.g.: fan.get_caps(fanobj[0])
		Return value: list of capabilities

	3.set_rpm():
		Member function of class fan
		Sets the fan's speed in rpm
		Parameter 1: Fan object
		Parameter 2: Speed in RPM
		Function call e.g.: fan.set_rpm(fanobj[0],user_rpm)
		Return value: Current rpm of the fan(after setting)

	4.get_rpm():
		Member function of class fan
		Gets the current rpm of a particular fan
		Parameter: Object of the class fan.For e.g. fanobj[0]
		Function call e.g.: fan.get_rpm(fanobj[0])
		Return value: Current rpm of the fan

	5.set_percent():
		Member function of class fan
		Sets speed in percentage
		Parameter 1: Fan object
		Parameter 2: Speed in percentage
		Function call e.g.: fan.set_percent(fanobj[0],user_percent)
		Return value: Current speed in percentage

	6.get_percent():
		Member function of class fan
		Returns the current percent of a particular fan
		Parameter:Object of the class fan.For e.g. fanobj[1]
		Function call e.g.:	fan.get_percent(fanobj[1])
		Return value:Current speed in percentage

	7.set_mode():
		Member function of class fan
		Sets the mode
		Parameter 1: Object of the class fan.For e.g. fanobj[1]
		Parameter 2: Mode
		Function call e.g.:	fan.set_mode(fanobj[1],user_mode)

	8.get_mode():
		Member function of class fan
		Get the fan's speed by mode
		Parameter: Object of the class fan.For e.g. fanobj[1]
		Return value: Current mode of the fan
		Function call e.g.:	fan.get_mode(fanobj[1])

	9.set_direction():
		Member function of class fan
		Set the direction of fan
		Parameter 1: Object of the class fan
		Parameter 2: User direction(1 for B2F, 2 for F2B)
		Function call e.g.:	fan.set_direction(fanobj[1],user_direction)

	10.print_all():
		Member function of class fan
		Prints all the attributes of a particular fan
		Function call e.g.:	fan.print_all(fanobj[1])

_____________________________________________________________________________
LED
_____________________________________________________________________________
Functions in ledLib.py

1. get_leds()
  Returns the list of LEDs
  Each LED can be accessed by using corresponding indices of the list
  Each LED object will have the below attributes and can be accessed with their corresponding names

  led_oid:  83886081
  description:  sys
  status:  5
  caps:  245760
  mode:  16
  character:  0

  For e.g.
  ledobj = get_leds()
  print ledobj[0].mode

2. get_caps()
  Member function of the class led
  Returns a list of capabilities of a particular LED
  Parameter: LED object

3. set_mode()
    Member function of class led
    Sets the mode of a particular LED
    Choose the mode from the below table.
     ONLP_LED_MODE_RED = 10
     ONLP_LED_MODE_RED_BLINKING = 11
     ONLP_LED_MODE_ORANGE = 12
     ONLP_LED_MODE_ORANGE_BLINKING = 13
     ONLP_LED_MODE_YELLOW = 14
     ONLP_LED_MODE_YELLOW_BLINKING = 15
     ONLP_LED_MODE_GREEN = 16
     ONLP_LED_MODE_GREEN_BLINKING = 17
     ONLP_LED_MODE_BLUE = 18
     ONLP_LED_MODE_BLUE_BLINKING = 19
     ONLP_LED_MODE_PURPLE = 20
     ONLP_LED_MODE_PURPLE_BLINKING = 21
     ONLP_LED_MODE_AUTO = 22
     ONLP_LED_MODE_AUTO_BLINKING = 23

    Parameter 1: LED object
    Parameter 2: user_mode(corresponding integer value from the above table)
    Note: Any LED can only be set according to their corresponding capabiity only. Use get_caps() to obtain capability of any LED

  4.set_state()
    Member function of class led
    Sets the state(ON/OFF) of LED
    Parameter 1: LED object
    Parameter 2: user_state(0 for OFF/1 for ON)

  5. set_char()
  Member function of class LED
  Parameter 1: LED object
  Parameter 2: user_char

  6. get_mode()
  Member function of class LED
  Parameter: LED object
  Return value: current mode of the LED

  7. get_state()
  Member function of class LED
  Parameter: LED object
  Return value: current state of the LED

  8. print_all()
   Prints all the attributes of a particular LED
   Parameter: LED object

Calling Member Functions:
e.g.:
led.set_state(ledobj[0],0)

_____________________________________________________________________________
Thermal
_____________________________________________________________________________
Functions in libThermal.py

1. get_thermals()
   Returns a list of thermal sensor objects

   description: Thermal Sensor 1- close to cpu //Use object.hdr.description
   status: 1
   caps: 15
   mcelcius: 33937  //temperature in millicelcius
   warning: 45000
   error: 55000
   shutdown: 60000

   Usage e.g.:
   thermalobj = get_thermals
   print thermalobj[0].status

_____________________________________________________________________________
PSU
_____________________________________________________________________________
Functions in PSULib.py

1. get_psus()
  Returns a list of PSU object in the system.
  Prints the PSU attributes
  PSU object has the following attributes and can be accessed using their corresponding names
  
  description:  PSU-1  #Use object.hdr.description
  model: 00014
  serial: 26
  status: 5
  caps: 0
  mvin: 118875
  mvout: 11937
  miin: 662
  miout: 5500
  mpin: 78500
  mpout: 65625

  Usage e.g.
  psuobj = get_psus()
  print psuobj[0].status

_____________________________________________________________________________
System
_____________________________________________________________________________
Functions used in sysLib.py

1. get_sys()
  Returns the system object
  Prints the system attributes
  System object has the following attributes and can be accessed by their corresponding names

  product_name:  AG7648
  serial_number:  A766F0DL161N00004
  mac_range:  256
  manufacturer:  DNI
  manufavture_date:  02/04/2016 14:49:01
  country_code:  CN
  diag_version:  0.1
  onie_version:  201412130048

  e.g.
  sysobj = get_sys()
  print sysobj[0].manufacture_date

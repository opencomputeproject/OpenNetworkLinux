
Test Environment: Open Network Linux

Steps to execute:
	1.Configure management interface on switch.
	2.ssh login to the device and make a directory (e.g. onlpd-test)
	3.Execute the command "killall onlpd"
	4.scp copy the scripts(fan_tc1.py ,fan_tc2.py,.. and libFan.py) to the same directory(onlpd-test).
	5.Execution command: python fan_tc1.py

Functions in libFan.py:

	1.get_fans() :
		returns the list of fans
		Each fan can be accessed by using corresponding indices of the list
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


More on test cases:
	1.fan_tc1 sets the rpm to 5000
	2.fan_tc2 sets the rpm to 12000
	3.fan_tc3 sets the rpm to 15000
	4.fan_tc4 sets the percentage to 30
	5.fan_tc5 sets the percentage to 50
	6.fan_tc6 sets the percentage to 70

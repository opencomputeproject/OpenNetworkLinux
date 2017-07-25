
Steps to execute:
	
	1.Configure management interface on switch.
	2.ssh login to the device and make a directory (e.g. onlpd-test)
	3.Execute the command "killall onlpd"
	4.scp copy the scripts(fan_tc1.py ,fan_tc2.py,…. and libFan.py) to onlpd-test . 
	5.Execution command: python fan_tc1.py

Functions in libFan.py:
	1.get_fans() : returns the list of fans 
	2.get_caps(): Returns the list of capabilities of a particular fan(Member function of class fan) 
	3.set_rpm(): Sets rpm that the user specifies. (Member function of class fan)
	4.get_rpm(): Returns the current rpm of a particular fan (Member function of class fan)
	5.set_percent(): Sets percentage of rpm that the user specifies (Member function of class fan)
	6.get_percent(): Returns the current percent of a particular fan (Member function of class fan)
	7.set_mode(): Sets the mode user specifies. (Member function of class fan)
	8.set_direction(): Sets the direction. (Member function of class fan)
	9.print_all(): Print all the attributes of a particular fan(Member function of class fan)

More on test cases:
	1.fan_tc1 sets the rpm to 5000
	2.fan_tc2 sets the rpm to 12000
	3.fan_tc3 sets the rpm to 15000
	4.fan_tc4 sets the percentage to 30
	5.fan_tc5 sets the percentage to 50
	6.fan_tc6 sets the percentage to 70


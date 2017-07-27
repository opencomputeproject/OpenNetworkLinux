"""
Test case 5: Turn led[1] to Mode 14(Yellow)
"""
user_mode = 14 #Yellow
DEBUG = True #static constant for debugging

from libonlp import led
from libonlp import get_leds
from time import sleep

ledobj = get_leds() #list of LEDs in the system
count = len(ledobj) #Total number of LEDs
print "The count is : ",count

led.set_normal(ledobj[1]) #Set state to 1 and mode to GREEN

valid = led.set_mode(ledobj[1],user_mode) #Set the LED to user_mode
if valid:
    sleep(3)
    currentState = led.get_mode(ledobj[1])
else:
    DEBUG = False
    print "Test case passed"

if DEBUG:
    if currentState == user_mode:
        print "Test case passed"

    elif currentState == 'None':
        print "Check the LED capabilities"

    else:
        print "Test case failed"

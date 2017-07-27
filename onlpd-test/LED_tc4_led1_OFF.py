"""
Test case 4: Turn led[1] to state 0(OFF)
"""
user_state = 0
DEBUG = True #static constant

from libonlp import led
from libonlp import get_leds
from time import sleep

ledobj = get_leds()  #List of LEDs
count = len(ledobj)  #Total number of LEDs
print "The count is : ",count

led.set_normal(ledobj[1]) #Set state to 1 and mode to GREEN

valid = led.set_state(ledobj[1],user_state) # Set the LED to user_state

if valid:
    sleep(3)
    currentState = led.get_mode(ledobj[1])
else:
    DEBUG = False
    print "Test case passed"

if DEBUG:
    if currentState == user_state:
        print "Test case passed"

    elif currentState == 'None':
        print "Check the LED capabilities"

    else:
        print "Test case failed"

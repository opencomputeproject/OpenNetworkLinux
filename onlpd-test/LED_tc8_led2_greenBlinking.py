"""
Test case 8: Turn led[2] to Mode 17(GREEN_BLINKING)
"""
user_mode = 17
DEBUG = True

from libonlp import led
from libonlp import get_leds
from time import sleep

ledobj = get_leds()
count = len(ledobj)
print "The count is : ",count

for x in range(count):
    led.set_normal(ledobj[x])

valid = led.set_mode(ledobj[2],user_mode)
if valid:
    sleep(3)
    currentState = led.get_mode(ledobj[2])
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

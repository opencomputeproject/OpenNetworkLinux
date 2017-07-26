"""
Test case 10: Turn led[3] to Mode 15(Yellow_Blinking)
"""
user_mode = 15
DEBUG = True

from libonlp import led
from libonlp import get_leds
from time import sleep

ledobj = get_leds()
count = len(ledobj)
print "The count is : ",count

for x in range(count):
    led.set_normal(ledobj[x])

led.set_mode(ledobj[3],user_mode)
sleep(3)
currentState = led.get_mode(ledobj[3])

if DEBUG:
    if currentState == user_mode:
        print "Test case passed"

    elif currentState == 'None':
        print "Check the LED capabilities"

    else:
        print "Test case failed"

"""
Test case 3: Turn led[2] to Mode 17(GREEN_BLINKING)
"""
from ledLib import led
from ledLib import get_leds
from time import sleep

x=0
ledobj = get_leds()
count = len(ledobj)
print "The count is : ",count
count = count - 1
while(x <= count):
    led.set_normal(ledobj[x])
    x = x + 1

user_mode = 17
led.set_mode(ledobj[2],user_mode)
sleep(3)
currentState = led.get_mode(ledobj[2])

if currentState == user_mode:
    print "Test case passed"

elif currentState == 'None':
    print "Check the LED capabilities"

else:
    print "Test case failed"

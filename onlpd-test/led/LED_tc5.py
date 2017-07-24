"""
Test case 5: Turn led[1] to Mode 14(Yellow)
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

user_mode = 14
led.set_mode(ledobj[1],user_mode)
sleep(3)
currentState = led.get_mode(ledobj[1])

if currentState == user_mode:
    print "Test case passed"

elif currentState == 'None':
    print "Check the LED capabilities"

else:
    print "Test case failed"

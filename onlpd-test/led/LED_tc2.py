"""
Test case 2: Turn led[0] to Mode 15(Yellow_Blinking)
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

user_mode = 15
led.set_mode(ledobj[0],user_mode)
sleep(3)
currentState = led.get_mode(ledobj[0])

if currentState == user_mode:
    print "Test case passed"

elif currentState == 'None':
    print "Check the LED capabilities"

else:
    print "Test case failed"

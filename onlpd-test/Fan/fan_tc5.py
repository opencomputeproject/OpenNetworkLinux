"""
Test case 4: Set percentage to 50
"""
user_percent = 50


from libFan import fan
from libFan import get_fans
from time import sleep

fanobj = get_fans() #Get all the fans and their statuses
count = len(fanobj) # Count the number of fans
print "The count is: ",count

for y in range(count):
    fan.set_normal_percent(fanobj[y])
    y = y + 1
sleep(10)

percentlist = []
for x in range(count):
    percentNew = fan.set_percent(fanobj[x],user_percent)
    if(percentNew != None):
        percentlist.append(percentNew)
    x = x + 1

DEBUG = True
if DEBUG:
    print "After setting credentials"
    print percentlist

    # Pass percentage = 5 %
    user_percent_lower = user_percent - 5
    user_percent_higher = user_percent + 5

    fail = 0
    pass1 = 0

    for i in range(len(percentlist)):
        if user_percent_lower <= percentlist[i] <= user_percent_higher:
            pass1 = pass1 + 1
        else:
            fail = fail + 1

    if fail != 0:
        final = "fail"
        print "test case failed"
    elif (len(percentlist) == 0):
        print "None of the fans have SET_PERCENT capability "
    else:
        final = "pass"
        print "Test case passed"

DEBUG = True

from libThermal import thermal
from libThermal import get_thermals
from time import sleep

Thermalobj = get_thermals()
count = len(Thermalobj)
print "The count is ",count

if DEBUG:
    if count != 0:
        print "Test case passed"
    else:
        print "Test case failed"

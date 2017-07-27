DEBUG = True

from libonlp import thermal
from libonlp import get_thermals
from time import sleep

Thermalobj = get_thermals() #List of thermals
count = len(Thermalobj)
print "The count is ",count

if DEBUG:
    if count != 0:
        print "Test case passed"
    else:
        print "Test case failed"

DEBUG = True

from PSULib import psu
from PSULib import get_psus
from time import sleep

psuobj = get_psus()
count = len(psuobj)
print "The count is ",count

if DEBUG:
    if count != 0:
        print "Test case passed"
    else:
        print "Test case failed"

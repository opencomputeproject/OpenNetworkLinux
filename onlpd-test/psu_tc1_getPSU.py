DEBUG = True

from libonlp import psu
from libonlp import get_psus
from time import sleep

psuobj = get_psus() #List of PSU object(s)
count = len(psuobj)
print "The count is ",count

if DEBUG:
    if count != 0:
        print "Test case passed"
    else:
        print "Test case failed"

DEBUG = True

from libonlp import sys
from libonlp import get_sys
from time import sleep

sysobj = get_sys()
count = len(sysobj)
print "The count is ",count

if DEBUG:
    if count != 0:
        print "Test case passed"
    else:
        print "Test case failed"

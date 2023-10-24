import watch_dog
import time

def arm_wdt():
    wdt_object = watch_dog.Watchdog()
    while True:
        wdt_object.arm(180)
        time.sleep(160)

arm_wdt()

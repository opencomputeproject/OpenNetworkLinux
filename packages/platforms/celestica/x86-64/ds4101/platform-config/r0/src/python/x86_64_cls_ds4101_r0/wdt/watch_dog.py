#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# @Time    : 2023/8/17 15:44
# @Mail    : inhuang@celestica.com
# @Author  : Ingrid Huang
# @Function: watchdog function

import os
import subprocess
import sys
import fcntl
import array

""" ioctl constants """
IO_WRITE = 0x40000000
IO_READ = 0x80000000
IO_READ_WRITE = 0xC0000000
IO_SIZE_INT = 0x00040000
IO_SIZE_40 = 0x00280000
IO_TYPE_WATCHDOG = ord('W') << 8

WDR_INT = IO_READ | IO_SIZE_INT | IO_TYPE_WATCHDOG
WDR_40 = IO_READ | IO_SIZE_40 | IO_TYPE_WATCHDOG
WDWR_INT = IO_READ_WRITE | IO_SIZE_INT | IO_TYPE_WATCHDOG

""" Watchdog ioctl commands """
WDIOC_GETSUPPORT = 0 | WDR_40
WDIOC_GETSTATUS = 1 | WDR_INT
WDIOC_GETBOOTSTATUS = 2 | WDR_INT
WDIOC_GETTEMP = 3 | WDR_INT
WDIOC_SETOPTIONS = 4 | WDR_INT
WDIOC_KEEPALIVE = 5 | WDR_INT
WDIOC_SETTIMEOUT = 6 | WDWR_INT
WDIOC_GETTIMEOUT = 7 | WDR_INT
WDIOC_SETPRETIMEOUT = 8 | WDWR_INT
WDIOC_GETPRETIMEOUT = 9 | WDR_INT
WDIOC_GETTIMELEFT = 10 | WDR_INT

""" Watchdog status constants """
WDIOS_DISABLECARD = 0x0001
WDIOS_ENABLECARD = 0x0002

STATUS_PATH = "/sys/devices/platform/cpld_wdt/status"
STATE_PATH = "/sys/devices/platform/cpld_wdt/state"
TIMEOUT_PATH = "/sys/devices/platform/cpld_wdt/timeout"
WDT_W_PATH = "/dev/cpld_wdt"
DEFAULT_TIMEOUT = 180
watchdog = 0
WDT_COMMON_ERROR = -1


class Watchdog():
    watchdog = None

    def __init__(self):
        global watchdog
        # Set default value
        state_cmd = "cat %s" % STATE_PATH
        state = self.run_command(state_cmd)
        self.armed = True if state == "active" else False
        self.timeout = DEFAULT_TIMEOUT
        if not watchdog:
            watchdog = os.open(WDT_W_PATH, os.O_RDWR)
        self.watchdog = watchdog

    @staticmethod
    def run_command(cmd):
        result = ""
        try:
            p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            raw_data, err = p.communicate()
            if err.decode("utf-8") == "":
                result = raw_data.decode("utf-8").strip()
            else:
                result = err.decode("utf-8").strip()
        except Exception:
            pass
        return result

    def _enable(self):
        """
        Turn on the watchdog timer
        """
        req = array.array('h', [WDIOS_ENABLECARD])
        fcntl.ioctl(self.watchdog, WDIOC_SETOPTIONS, req, False)

    def _disable(self):
        """
        Turn off the watchdog timer
        """
        req = array.array('h', [WDIOS_DISABLECARD])
        fcntl.ioctl(self.watchdog, WDIOC_SETOPTIONS, req, False)

    def arm(self, seconds):
        ret = WDT_COMMON_ERROR
        if seconds < 0:
            return ret

        try:
            self.timeout = self._set_timeout(seconds)
            if self.armed:
                self._keep_alive()
            else:
                self._enable()
                self.armed = True

            ret = self.timeout
        except IOError:
            pass
        return ret

    def _keep_alive(self):
        """
        Keep alive watchdog timer
        """
        fcntl.ioctl(self.watchdog, WDIOC_KEEPALIVE)

    def _set_timeout(self, seconds):
        """
        Set watchdog timer timeout
        @param seconds - timeout in seconds
        @return is the actual set timeout
        """
        req = array.array('I', [seconds])
        fcntl.ioctl(self.watchdog, WDIOC_SETTIMEOUT, req, True)
        return int(req[0])

    def get_time_left(self):
        """
        Get time left before watchdog timer expires
        @return time left in seconds
        """
        req = array.array('I', [0])
        fcntl.ioctl(self.watchdog, WDIOC_GETTIMELEFT, req, True)

        return int(req[0])

    def disarm(self):
        """
        Disarm the hardware watchdog
        Returns:
            A boolean, True if watchdog is disarmed successfully, False if not
        """
        disarmed = False
        if self.is_armed():
            try:
                self._disable()
                self.armed = False
                disarmed = True
            except IOError:
                pass

        if self.watchdog is not None:
            os.write(self.watchdog, "V")

        return disarmed

    def is_armed(self):
        """
        Retrieves the armed state of the hardware watchdog.
        Returns:
            A boolean, True if watchdog is armed, False if not
        """

        return self.armed

    def __del__(self):
        """
        Close watchdog
        """
        if self.watchdog is not None:
            os.close(self.watchdog)

def print_help():
    print("""
    Usage: ./watch_dog.py [OPTIONS] COMMAND [ARGS]...

    Commands:
        arm        Arm HW watchdog with default time
        arm -s XX  Arm HW watchdog with XX seconds, XX is number
        disarm     Disarm HW watchdog
        status     Check the watchdog status with remaining_time if it's armed
    """)


def main():
    watchdog_object = Watchdog()
    action = sys.argv[1:]
    try:
        if action[0] == "status":
            if watchdog_object.armed:
                print("\nStatus: Armed\nTime remaining: %s seconds\n" % watchdog_object.get_time_left())
            else:
                print("\nStatus: Disarmed\n")
        elif action[0] == "disarm":
            wdt_disarm = watchdog_object.disarm()
            if wdt_disarm:
                print("\nDisarm the watchdog!\n")
            else:
                print("\nDisarm watchdog failed!\n")
        elif action[0] == "arm":
            if len(action) == 3:
                if action[1] == "-s" and int(action[2]):
                    watchdog_object.arm(int(action[2]))
                    print("\nSet watchdog timeout: %s\n" % action[2])
                else:
                    print_help()
            elif len(action) == 1:
                watchdog_object.arm(DEFAULT_TIMEOUT)
                print("\nSet watchdog default timeout: %s\n" % DEFAULT_TIMEOUT)
            else:
                print_help()
        else:
            print_help()
    except Exception:
        print_help()


if __name__ == '__main__':
    main()

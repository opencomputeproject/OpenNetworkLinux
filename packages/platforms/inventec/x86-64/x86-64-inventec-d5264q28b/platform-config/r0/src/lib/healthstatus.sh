#!/bin/bash

normal='0 : normal'
unpowered='2 : unpowered'
fault='4 : fault'
notinstalled='7 : not installed'

while true
do
  psu0var=$(cat /sys/bus/i2c/devices/i2c-0/0-0055/psu0)    # bottom PSU
  psu1var=$(cat /sys/bus/i2c/devices/i2c-0/0-0055/psu1)    # top PSU
  fan1in=$(cat /sys/class/hwmon/hwmon1/device/fan1_input)  # fanmodule1, far right, back view
  fan2in=$(cat /sys/class/hwmon/hwmon1/device/fan2_input)  # fanmodule1, far right, back view
  fan3in=$(cat /sys/class/hwmon/hwmon1/device/fan3_input)  # fanmodule2
  fan4in=$(cat /sys/class/hwmon/hwmon1/device/fan4_input)  # fanmodule2
  fan5in=$(cat /sys/class/hwmon/hwmon1/device/fan5_input)  # fanmodule3
  fan6in=$(cat /sys/class/hwmon/hwmon1/device/fan6_input)  # fanmodule3
  fan7in=$(cat /sys/class/hwmon/hwmon1/device/fan7_input)  # fanmodule4, far left, back view
  fan8in=$(cat /sys/class/hwmon/hwmon1/device/fan8_input)  # fanmodule4, far left, back view

  if [ "$psu0var" = "$normal" ] &&
     [ "$psu1var" = "$normal" ] &&                         # PSU normal operation
     [ "$fan1in" -gt 0 ] && [ "$fan2in" -gt 0 ] &&         # fan on
     [ "$fan3in" -gt 0 ] && [ "$fan4in" -gt 0 ] &&
     [ "$fan5in" -gt 0 ] && [ "$fan6in" -gt 0 ] &&
     [ "$fan7in" -gt 0 ] && [ "$fan8in" -gt 0 ]
  then
    echo 1 > /sys/bus/i2c/devices/i2c-0/0-0055/ctl
    echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led     # red off
    echo 7 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led     # grn solid

  elif [ "$psu0var" = "$unpowered" ] ||
       [ "$psu1var" = "$unpowered" ] ||                    # PSU unpowered
       [ "$fan1in" -eq 0 ] || [ "$fan2in" -eq 0 ] ||       # fan off
       [ "$fan3in" -eq 0 ] || [ "$fan4in" -eq 0 ] ||
       [ "$fan5in" -eq 0 ] || [ "$fan6in" -eq 0 ] ||
       [ "$fan7in" -eq 0 ] || [ "$fan8in" -eq 0 ]
  then
    echo 1 > /sys/bus/i2c/devices/i2c-0/0-0055/ctl
    echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led     # red off
    echo 3 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led     # grn @2Hz

  elif [ "$psu0var" = "$fault" ] ||
       [ "$psu0var" = "$notinstalled" ] ||
       [ "$psu1var" = "$fault" ] ||
       [ "$psu1var" = "$notinstalled" ]                    # PSU fault or PSU not installed
  then
    echo 1 > /sys/bus/i2c/devices/i2c-0/0-0055/ctl
    echo 0 > /sys/bus/i2c/devices/i2c-0/0-0055/grn_led     # grn off
    echo 7 > /sys/bus/i2c/devices/i2c-0/0-0055/red_led     # red solid
  fi
  sleep 1
done

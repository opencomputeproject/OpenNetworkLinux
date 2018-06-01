#!/usr/bin/python2
############################################################
#
# This script produces a YAML list on stdout of all
# packages necessary to support the given init system
#
import argparse

ap = argparse.ArgumentParser(description='ONL Init Package Lister')
ap.add_argument('init', metavar='INIT-SYSTEM', choices=['sysvinit', 'systemd'], help='Init system to use')

ops = ap.parse_args()

if ops.init == 'sysvinit':
    print '- sysvinit-core'
elif ops.init == 'systemd':
    print '- systemd'
    print '- systemd-sysv'

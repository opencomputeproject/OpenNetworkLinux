#!/usr/bin/python

# oomcollectd.py --
#
# Copyright (c) 2016 Cumulus Networks, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to
# do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import oom
import collectd


class OOMValue(collectd.Values):
    def __init__(self, dtype, port, key, value):
        nkey = '%s-%s' % (key, port.port_name)
        collectd.Values.__init__(self, type=dtype, type_instance=nkey,
                                 values=[value])


class LaserBiasCurrent(OOMValue):
    def __init__(self, port, key, value):
        OOMValue.__init__(self, 'current', port, key, value)


class ReceivePower(OOMValue):
    def __init__(self, port, key, value):
        OOMValue.__init__(self, 'power', port, key, value)


class LaserOutputPower(OOMValue):
    def __init__(self, port, key, value):
        OOMValue.__init__(self, 'power', port, key, value)


class SupplyVoltage(OOMValue):
    def __init__(self, port, key, value):
        OOMValue.__init__(self, 'voltage', port, key, value)


class ModuleTemperature(OOMValue):
    def __init__(self, port, key, value):
        OOMValue.__init__(self, 'temperature', port, key, value)


def read_callback(data=None):
    portlist = oom.oom_get_portlist()
    for port in portlist:
        dom = oom.oom_get_memory(port, 'DOM')
        if dom is None:
            continue

        for key in ('TX1_POWER', 'TX2_POWER', 'TX3_POWER', 'TX4_POWER',
                    'RX1_POWER', 'RX2_POWER', 'RX3_POWER', 'RX4_POWER'):
            if key in dom:
                value = dom[key] / 1000.0
                LaserOutputPower(port, key[:3].lower(), value).dispatch()

        for key in ('TX1_BIAS', 'TX2_BIAS', 'TX3_BIAS', 'TX4_BIAS'):
            if key in dom:
                value = dom[key] / 1000.0
                LaserBiasCurrent(port, key[:3].lower(), value).dispatch()

        if 'SUPPLY_VOLTAGE' in dom:
            SupplyVoltage(port, 'supply', dom['SUPPLY_VOLTAGE']).dispatch()

        if 'TEMPERATURE' in dom:
            ModuleTemperature(port, 'module', dom['TEMPERATURE']).dispatch()

collectd.register_read(read_callback)

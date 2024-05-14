# /////////////////////////////////////////////////////////////////////
#
#  oomjsonsvr.py : implements functions to translate OOM Southbound API
#  data structures to JSON, and back
#
#  implements the switch side of an "over the network" OOM
#  southbound API (a SHIM). This routine calls the native SHIM on the
#  network, turns the response into JSON, and ships it over the network
#  (in response to calls from the server side)
#
#  Copyright 2016  Finisar Inc.
#
#  Author: Don Bollinger don@thebollingers.org
#
# ////////////////////////////////////////////////////////////////////

import os
from ctypes import *
import json
import base64
from flask import Flask, request
from oom.oomjsonshim import jpdict_to_cport, cport_to_json
from oom.oomtypes import c_port_t
from oom import *


#
# Read from EEPROM, pass it over the network as JSON
#
def oom_get_json_memory_sff(cport, address, page, offset, length):
    port = matchport(cport, portlist.list)
    data = oom_get_memory_sff(port, address, page, offset, length)
    jsdata = base64.b64encode(data)
    js_out = '{"data": "%s",\n "length": "%s"}' % (jsdata, str(length))
    return(js_out)


#
# Raw write
#
def oom_set_json_memory_sff(cport, address, page, offset, length, data):
    port = matchport(cport, portlist.list)
    retlen = oom_set_memory_sff(port, address, page, offset, length, data)
    js_out = '{"length": "%s"}' % str(retlen)
    return js_out


def matchport(cport, portlist):
    for port in portlist:
        if cport.handle == port.c_port.handle:
            return port
    return None


#
# Simulate oomsouth.oom_get_portlist(0,0)
# In fact, call oom_get_portlist(), cache the portlist, return it's length
#
def oom_get_json_portlist_zeros():
    portlist.list = oom_get_portlist()
    numports = len(portlist.list)
    return '{"numports": "%d"}' % numports


#
# Fetch portlist if needed, else use cached value
#
def oom_get_json_portlist():
    if portlist.list == []:
        portlist.list = oom_get_portlist()
    return port_list_to_json(portlist.list)


def port_list_to_json(portlist):
    js_out = '{"portlist":[\n\t'
    first = 1
    for port in portlist:
        if first == 0:
            js_out += ',\n\t'
        else:
            first = 0

        js_out += cport_to_json(port.c_port)
    js_out += '\n\t]}'
    return js_out


def json_to_numports(js):
    # json to dict, dict to value string, string to int
    return int(json.loads(js)["numports"])

#
# Start up the Flask server
#
app = Flask(__name__)


# set up the one URL that I'm listening to for requests
# receive all requests, unpack the parameters and send them to the
# right routine
@app.route('/OOM', methods=['GET'])
def getOOMdata():
    command = request.json
    print('command:')
    print(command)
    # 'ogp0' means oom_get_portlist(0, 0)
    if command['cmd'] == 'ogp0':
        js = oom_get_json_portlist_zeros()
        return js
    # 'ogp' means oom_get_portlist()
    if command['cmd'] == 'ogp':
        js = oom_get_json_portlist()
        return js
    # 'ogms' means 'oom_get_memory_sff()'
    if command['cmd'] == 'ogms':
        cport = jpdict_to_cport(json.loads(command['port']))
        address = int(command['address'])
        page = int(command['page'])
        offset = int(command['offset'])
        length = int(command['length'])
        retval = oom_get_json_memory_sff(cport, address, page, offset, length)
        return retval
    # 'osms' means 'oom_set_memory_sff()'
    if command['cmd'] == 'osms':
        cport = jpdict_to_cport(json.loads(command['port']))
        address = int(command['address'])
        page = int(command['page'])
        offset = int(command['offset'])
        length = int(command['length'])
        # the data is going to take more work...
        # fetch it from the dict, b64 decode it...
        sentdata = base64.b64decode(command['data'])
        # create a string buffer suitable for oomsouth.oom_set...
        data = create_string_buffer(length)
        # and copy the data into it
        ptr = 0
        for c in sentdata:
            data[ptr] = c
            ptr += 1
        retval = oom_set_json_memory_sff(cport, address, page, offset,
                                         length, data)
        return retval


class portlist:
    list = []

portlist = portlist()
oomlib.oom_portlist_nokeys = 1    # secret speedup for oom_get_portlist()

if __name__ == "__main__":
    # to debug locally use:
    # app.run(debug=True)
    # to be visible across the network, use:
    app.run(host='0.0.0.0')

# /////////////////////////////////////////////////////////////////////
#
#  oomjsonshim.py : An OOM Southbound SHIM, in Python, that
#  packages Southbound API requests into JSON and ships them to a switch
#  to be executed, and receives the results, unpacks the JSON, and
#  presents them in the form of the standard OOM Southbound API
#
#  To load this SHIM,COPY oomjsonshim.py into py_oomsouth.py
#  This will pre-empt any SHIM loaded in the lib directory
#
#  Copyright 2016  Finisar Inc.
#
#  Author: Don Bollinger don@thebollingers.org
#
# ////////////////////////////////////////////////////////////////////

import requests
import json
import base64
from .oomtypes import c_port_t


# Set the default URL for the remote switch
class url:
    remote = "http://localhost:5000/OOM"
url = url()


def setparms(parms):
    temp = parms
    portplus = ':5000/OOM'
    if temp[-9:] != portplus:
        temp = temp + portplus
    preamble = 'http://'
    if temp[:7] != preamble:
        temp = preamble + temp
    url.remote = temp


#
# implement oom_get_portlist over-the-network, using JSON
#
def oom_get_portlist(cport_list, numports):
    if (cport_list == 0) and (numports == 0):
        js = requests.get(url.remote, json={"cmd": "ogp0"})
        return json_to_numports(js.text)
    else:
        js = requests.get(url.remote,
                          json={'cmd': 'ogp', 'numports': numports})
        retval = json_to_cport_list(js.text, cport_list)
        return retval


#
# implement oom_get_memory_sff over-the-network, using JSON
# note, we are in oomsouth, so 'cport' is actually a c_port_t
#
def oom_get_memory_sff(cport, address, page, offset, length, data):
    strport = cport_to_json(cport)
    js = requests.get(url.remote, json={'cmd': 'ogms', 'port': strport,
                                        'address': address, 'page': page,
                                        'offset': offset, 'length': length})

    py = json.loads(js.text)
    pydata = py['data']
    retlen = int(py['length'])
    # in Python 3, type conversions add b' to the front and ' to the back
    # need to strip them off before decoding.  Bother!
    if isinstance(pydata, str):
        pydata = pydata[2:len(pydata)-1]
    retdata = base64.b64decode(pydata)
    ptr = 0

    for c in retdata:
        data[ptr] = c
        ptr += 1
    return(retlen)


#
# implement oom_set_memory_sff over-the-network, using JSON
#
def oom_set_memory_sff(port, address, page, offset, length, data):
    strport = cport_to_json(port)
    jsdata = base64.b64encode(data)
    js = requests.get(url.remote, json={'cmd': 'osms', 'port': strport,
                                        'address': address, 'page': page,
                                        'offset': offset, 'length': length,
                                        'data': jsdata})
    py = json.loads(js.text)
    retlen = int(py['length'])
    return retlen


# Unpack the json return into a port list as defined by the Southbound API
def json_to_cport_list(js, cport_list):
    jports = json.loads(js)
    ptr = 0
    for jp_dict in jports['portlist']:
        cport = jpdict_to_cport(jp_dict)
        cport_list[ptr] = cport
        ptr += 1
    return 0


def json_to_numports(js):
    # json to dict, dict to value string, string to int
    return int(json.loads(js)["numports"])


def jpdict_to_cport(jp_dict):
    cport = c_port_t()
    cport.handle = int(jp_dict['handle'])
    name = jp_dict['name'].encode('utf-8')
    for i in range(0, 32):
        if i < len(name):
            if isinstance(name, str):  # python 2.7 type
                cport.name[i] = ord(name[i])
            else:                    # python 3 type is int
                cport.name[i] = name[i]
        else:
            cport.name[i] = 0
    cport.oom_class = int(jp_dict['oom_class'])
    return cport


def cport_to_json(cport):
    fixedhandle = cport.handle  # python thinks '0' is 'None'
    if fixedhandle is None:     # for a c_void_p type object
        fixedhandle = 0         # so fix it :-(
    port_name = bytearray(cport.name).decode('utf-8').rstrip('\0')
    json_out = '{"handle": "%d", "oom_class": "%d", "name": "%s"}' % \
               (fixedhandle, cport.oom_class, port_name)
    return json_out

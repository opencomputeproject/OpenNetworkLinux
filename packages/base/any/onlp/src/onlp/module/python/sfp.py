#!/usr/bin/python
################################################################
#
#        Copyright 2016, Big Switch Networks, Inc.
#
# Licensed under the Eclipse Public License, Version 1.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#        http://www.eclipse.org/legal/epl-v10.html
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the
# License.
#
################################################################
#
# Wrapper object class for use with the onlp sfp library.
#
################################################################
import ctypes
from ctypes.util import find_library
import os
import struct
from ctypes import byref
import sff_enum

class ONLPSFPException(Exception):
    pass

class aim_bitmap_hdr(ctypes.Structure):
    _fields_ = [('wordcount', ctypes.c_int),
                ('maxbit',ctypes.c_int),
                ('allocated',ctypes.c_int),
                ('aim_bitmap_words',ctypes.c_void_p)]

class aim_256_bitmap(ctypes.Structure):
    _fields_ = [('aim_bitmap_hdr', aim_bitmap_hdr),
                ('aim_bitmap_words',ctypes.c_ubyte *32)]

class SFP(object):
    """" Light python wrapper around the ONLP SFP library """
# Loading libonlp library
    libonlp = 'libonlp.so.1'
    onlppath = os.path.join(*(os.path.split(__file__)[:-1] + (libonlp,)))
    onlp = ctypes.cdll.LoadLibrary(onlppath)

    def __init__(self):
        """  %brief This function is for initializing the onlp
        which include sys,sfp,led,psu,fan, JSON  and thermal
        %param None
        %return 0 if successful
         """
        err = SFP.onlp.onlp_init()
        if err != 0: 
            raise ONLPSFPException("sfp_init failed: " + str(err))

    def sfp_present(self, port):
        """ %brief Fucntion to check for the port is SFP or not
        %param port Integer for which port to query
        %return a boolean if this SFP is plugged into this port
         """
        return SFP.onlp.onlp_sfp_is_present(port)==1

    def sfp_valid(self, port):
        """  %brief Function to validate the SFP port number
        %param port Integer for which port to valiate
        %return a boolean if this SFP is plugged into this port
         """
        return SFP.onlp.onlp_sfp_port_valid(port)==1
    
    def __sfp_bitmap_init(self,bitmap):
        """ initialize the sfp bitmap 
        %param Bitmap 
        %returns None
         """
        SFP.onlp.onlp_sfp_bitmap_t_init(byref(bitmap))

    def __sfp_bitmap_get(self,bitmap):
        """ Get the set of valid {Q}SFP ports. 
        %param dst The receives the presence bitmap for all ports.
        %returns bitmap Returns the valid set of SFP-capable port numbers.
         """
        return SFP.onlp.onlp_sfp_bitmap_get(byref(bitmap))

    def __sfp_bitmap_rx_los_get(self,bitmap):
        """  Get the RX_LOS bitmap for all ports.. 
        %param dst Receives the RX_LOS bitmap for all ports.
        %returns bitmap Returns the valid set of SFP-capable port numbers.
         """
        return SFP.onlp.onlp_sfp_rx_los_bitmap_get(byref(bitmap))
    
    def __sfp_bitmap_free(self,bitmap):
        """ Free a bitmap structure. 
        %param bmap The bitmap structure to free.
        %returns None
         """
        SFP.onlp.aim_bitmap_free(byref(bitmap))

    def __sfp_eeprom_read(self,port):
        """ Read IEEE standard EEPROM data from the given port
        %param port The SFP Port
        %param rv Receives a buffer containing the EEPROM data.
        %returns The size of the eeprom data, if successful else -1 on error
         """
        eeprom_read = SFP.onlp.onlp_sfp_eeprom_read
        eeprom_read.argtypes = [ctypes.c_int,
                    ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte))]
        eeprom_read.restype  = ctypes.c_int
        idpom = ctypes.POINTER(ctypes.c_ubyte)()
        rv = eeprom_read(port, byref(idpom))
        if rv < 0:
            raise ONLPSFPException("Error reading eeprom  " + str(rv))
        return idpom
   
    def __onlp_sfp_post_insert(self,port,info):
        """ Call the SFP post-insertion handler.
        %param port The SFP port
        %param SFF info structure  pointer
        %return 0 if sucessful
        """ 
        err = SFP.onlp.onlp_sfp_post_insert(port,byref(info))
        if err < 0:
            raise ONLPSFPException("Error in sfp post insert  " + str(err))

    def __onlp_sfp_control_set(self,port,control,value):
        """ Set an SFP control.
        %param port The SFP port
        %param control The control
        %param value The Value
        %return 0 if sucessful
        """
        err = SFP.onlp.onlp_sfp_control_set(port,control,value)
        if err < 0:
            raise ONLPSFPException("Error in sfp control set " + str(err))

    def __onlp_sfp_control_get(self,port,control,value_ptr):
        """ Get an SFP control.
        %param port The SFP port
        %param control The control
        %param [out] value Receives the current value.
        %return 0 if sucessful
        """
        err = SFP.onlp.onlp_sfp_control_get(port,control,byref(value_ptr))
        if err < 0:
            raise ONLPSFPException("Error in sfp control get" + str(err))
 
    def __onlp_sfp_control_flags_get(self,port,flag_ptr):
        """ Get the value of all SFP controls.
        %param port The SFP port
        %param flags Receives the control flag values.
        %return 0 if sucessful
        """
        err = SFP.onlp.onlp_sfp_control_flags_get(port,byref(flag_ptr))
        if err < 0:
            raise ONLPSFPException("Error in sfp control flag get" + str(err))
        return err
 
    def __sfp_dom_read(self,port):
        """ Read the DOM data from the given port.
        %param port The SFP Port
        %param rv Receives a buffer containing the DOM data.
        %returns The size of the dom data, if successful else -1 on error
         """
        dom_read = SFP.onlp.onlp_sfp_dom_read
        dom_read.argtypes = [ctypes.c_int,
                    ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte))]
        dom_read.restype  = ctypes.c_int
        idpom = ctypes.POINTER(ctypes.c_ubyte)()
        rv = dom_read(port, byref(idpom))
        if rv < 0:
            raise ONLPSFPException("Error reading dom  " + str(rv))
        return idpom

    def __print_block_hex(self,data, length):
        """ print raw data, in hex.
        %param data from eeprom
        %returns None
         """
        return " ".join(c[2:4] for c in map(hex,data[0:length]))
    
    def __sfp_portlist(self):
        """ Read the bitmap and return the port numbers.
        %param None 
        %returns the list of the ports avaiable 
        """  
        ports =list()
        bitmap = aim_256_bitmap()
        onlp_obj._SFP__sfp_bitmap_init(bitmap)
        rv = onlp_obj._SFP__sfp_bitmap_get(bitmap)
        inx = 4
        bits = ''.join('{0:08b}'.format(x) for x in bitmap.aim_bitmap_words)
        if len(bits) != 256:
            raise ONLPSFPException("Invalid bitmap " + str(len(bits)))
        for i in bits:
            if int(i) == 1:
                ports.append(inx)
            inx+=1
        return ports
    
    def sff_info_init(self,sff_info_ptr,data_ptr):
        """ Initialize an SFF module information structure.
        %param rv [out] Receives the data.
        %param eeprom Raw EEPROM data.
        if eeprom is != NULL it will be copied into rv->eeprom first.if eeprom is NULL it is assumed the rv->eeprom buffer has already been initialized.
        %return 0 on successful parse else < 0 on error
         """
        err = SFP.onlp.sff_info_init(byref(sff_info_ptr),data_ptr)
        if err != 0:
            raise ONLPSFPException("sff_init failed: " + str(err))        

    def __sff_info_valid(self,sff_info_ptr,verbose):
        """ Determine if this is a valid SFP(whether or not we can parse it)
        %param info The info structure.
        %param verbose Whether to report errors on invalid contents.
        %returns 0 on successful parse else < 0 on error
        """
        return SFP.onlp.sff_info_valid(byref(sff_info_ptr),verbose)
        
    def __sfp_status(self,port):
        """ Gets the sfp control status string
        %param port The SFP port
        %returns sfp control status string
        """
        flag = ctypes.c_int(0)
        status = ctypes.create_string_buffer(" ",4)
        onlp_obj._SFP__onlp_sfp_control_flags_get(port,flag)
        
        if(flag.value & sff_enum.onlp_sfp_control_flag.ONLP_SFP_CONTROL_FLAG_RX_LOS):
            status.value = 'R'
        elif (flag.value & sff_enum.onlp_sfp_control_flag.ONLP_SFP_CONTROL_FLAG_TX_FAULT):
            status.value = 'T'
        elif (flag.value & sff_enum.onlp_sfp_control_flag.ONLP_SFP_CONTROL_FLAG_TX_DISABLE):
            status.value = 'X'
        elif (flag.value & sff_enum.onlp_sfp_control_flag.ONLP_SFP_CONTROL_FLAG_LP_MODE):
            status.value = 'L'
        return status.value

    def sfp_dump(self):
        """ This will output data like sfpdump sbin
        """
        portlist = list()    
        portlist = onlp_obj._SFP__sfp_portlist()
        print ""
        for port in portlist:
            if onlp_obj.sfp_present(port):
                eeprom = onlp_obj._SFP__sfp_eeprom_read(port)
                print "SFP Port:",port
                print "eeprom : \n",onlp_obj._SFP__print_block_hex(eeprom, 256)
            else:
                print "SFP Port:",port," Missing"
    
    def sfp_onlpdump(self):
        portlist = list()
        portlist = onlp_obj._SFP__sfp_portlist()
        print ""
        print "Port  Type            Media   Status  Len    Vendor            Model             S/N             \n"
        print "----  --------------  ------  ------  -----  ----------------  ----------------  ----------------\n"
        for port in portlist:
            if onlp_obj.sfp_present(port):
                eeprom = onlp_obj._SFP__sfp_eeprom_read(port)
                sff_in = sff_enum.sff_info()
                SFP.onlp.sff_info_init(byref(sff_in),eeprom)
                status_str = onlp_obj._SFP__sfp_status(port)
                print '%4d  %-14s  %-6s  %-6.6s  %-5.5s  %-16.16s  %-16.16s  %16.16s' %(port,sff_in.module_type_name,sff_in.media_type_name,status_str,sff_in.length_desc,sff_in.vendor,sff_in.model,sff_in.serial)
            else:
                print '%4d  NONE' %(port)   
if __name__ == '__main__': 
    onlp_obj = SFP()
    onlp_obj.sfp_dump()
    onlp_obj.sfp_onlpdump()

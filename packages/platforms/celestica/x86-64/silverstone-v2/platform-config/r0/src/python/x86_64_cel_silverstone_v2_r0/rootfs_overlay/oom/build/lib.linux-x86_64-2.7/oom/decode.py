# /////////////////////////////////////////////////////////////////////
#
# decode.py : decode DOM/FAWS/SerialID... to readable physical units
#
#
#
#
#
#
#
# ////////////////////////////////////////////////////////////////////

import binascii
from ctypes import create_string_buffer
from math import log10


__author__ = "Yuan Yu"
__version__ = "0.0.1"
__email__ = "yuan.yu@finisar.com"


# module ID dictionary, expressed in hex to match the spec
mod_id_dict = {0x00: 'Unknown',
               0x01: 'GBIC',
               0x02: 'Module soldered to MB',
               0x03: 'SFP/SFP+/SFP28',
               0x04: '300_Pin_XBI',
               0x05: 'XENPAK',
               0x06: 'XFP',
               0x07: 'XFF',
               0x08: 'XFP-E',
               0x09: 'XPAK',
               0x0A: 'X2',
               0x0B: 'DWDM-SFP/SFP+',
               0x0C: 'QSFP',
               0x0D: 'QSFP+',
               0x0E: 'CXP',
               0x0F: 'Sheilded Mini Multilane HD 4x',
               0x10: 'Sheilded Mini Multilane HD 8x',
               0x11: 'QSFP28',
               0x12: 'CXP2',
               0x13: 'CDFP',
               0x14: 'Sheilded Mini Multilane HD 4x Fanout Cable',
               0x15: 'Sheilded Mini Multilane HD 8x Fanout Cable',
               0x16: 'CDFP',
               0x17: 'microQSFP',
               0x18: 'QSFP-DD Double Density 8x (INF-8628)',
               0x19: 'OSFP 8x Pluggable Transceiver',
               0x1A: 'SFP-DD Double Density 2X Pluggable Transceiver',
               0x1B: 'DSFP Dual Small Form Factor Pluggable Transceiver',
               0x1C: 'x4 MiniLink/OcuLink',
               0x1D: 'x8 Minilink',
               0x1E: 'QSFP+ or later with CMIS spec',

               # next values are CFP types. Note that their spec
               # (CFP MSA Management Interface Specification
               # ver 2.4 r06b page 67)
               # values overlap with the values for i2c type devices.  OOM has
               # chosen to add 256 (0x100) to the values to make them unique

               0x10E: 'CFP',
               0x110: '168_PIN_5X7',
               0x111: 'CFP2',
               0x112: 'CFP4',
               0x113: '168_PIN_4X5',
               0x114: 'CFP2_ACO',
               }


def get_voltage(x):  # return in V
    """ Decodes and returns voltage value(in V) from the raw data"""
    if len(x) != 2:
        print("wrong voltage format")
        return
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])*256 + ord(x[1])
    else:
        temp = x[0]*256 + x[1]
    result = float(temp*0.1/1000)
    return result


def get_temperature(x):  # return in 'C
    """ Decodes and returns temperature value(in *C) from the raw data"""
    if len(x) != 2:
        print("wrong temperature format")
        return
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])*256 + ord(x[1])
    else:
        temp = x[0]*256 + x[1]
    if temp > 0x7FFF:   # if the sign bit is set
        temp -= 65536   # take two's complement
    result = float(temp/256.0)
    return result


# note:  get_voltage and get_power are actually identical
# implemented twice so as not to confuse the maintainer :-)
def get_power(x):   # return in mW
    """ Decodes and returns power value(in mW) from the raw data"""
    if len(x) != 2:
        print("wrong power format")
        return
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])*256 + ord(x[1])
    else:
        temp = x[0]*256 + x[1]
    result = float(temp*0.1/1000)
    return result


def mwtodbm(x):
    """ Converts value in mW to dbm"""
    if x < .001:
        return -30  # by convention, -30dbm is the lowest legal value
    return 10 * log10(x)


def get_power_dbm(x):   # return power in dbm (10*log10(power-in-mw))
    """ Decodes and returns power value(in dbm) from the raw data"""
    return mwtodbm(get_power(x))


def get_current(x):  # return in mA
    """ Decodes and returns current value(in mA) from the raw data"""
    if len(x) != 2:
        print("wrong bias format")
        return
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])*256 + ord(x[1])
    else:
        temp = x[0]*256 + x[1]
    result = float(temp/500.0)
    return result


def get_signed_current(x):  # return in mA
    """ Decodes and returns current value(in mA) from the raw data"""
    if len(x) != 2:
        print("wrong bias format")
        return

    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])*256 + ord(x[1])
    else:
        temp = x[0]*256 + x[1]
    if temp > 0x7FFF:   # if the sign bit is set
        temp -= 65536   # take two's complement
    result = float(temp/10.0)
    return result


def get_string(x):  # copy the cbuffer into a string
    """ Decodes and returns string from the raw data"""
    if isinstance(x, bytes):
        result = x.decode('utf-8')
    else:
        result = x.value.decode('utf-8')
    return result


def set_string(old, new):  # trivial, but needed for oom_set_keyvalue()
    """ Sets string. Trivial, but needed for oom_set_keyvalue()"""
    return new


def mod_id(x):  # return Module ID
    """ Decodes and returns module ID from the raw data"""
    return mod_id_dict.get(x, 'Reserved')


def get_bytes(x):

    if isinstance(x, bytes):
        return x
    return bytes(x[:])


def get_int(x):
    result = 0
    if len(x) > 4:
        print("too many bytes to decode into 32 bit int")
        return
    for i in x:
        if isinstance(i, int):
            result = (result * 256) + i
        else:
            result = (result * 256) + ord(i)
    return result


def get_intX10(x):
    return get_int(x)*10


# return 2 bits

def get2_bits(x, n):
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])
    else:
        temp = x[0]
    temp = temp >> n
    temp %= 4
    return temp


def get2_bit6(x):
    return get2_bits(x, 6)


def get2_bit4(x):
    return get2_bits(x, 4)


def get2_bit2(x):
    return get2_bits(x, 2)


def get2_bit0(x):
    return get2_bits(x, 0)


def get3_bit6(x):  # get bits 6, 5, 4
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])
    else:
        temp = x[0]
    temp = temp >> 4
    temp %= 8
    return temp


def get3_bit2(x):  # get bits 2, 1, 0
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])
    else:
        temp = x[0]
    temp %= 8
    return temp


# from 'x', extract 'numbits', starting at 'offset' and going DOWN
# high order is bit 7, low order is bit 0
# so, get_bits(0b00110000, 5, 2) will return 0b11, ie 3
def get_bits(x, offset, numbits):
    if (len(x) > 2) or (offset > 15) or (offset < 0) or \
            (numbits > 16) or (numbits < 1) or \
            ((offset - numbits) < -1):
        print('get_bits bad parameters - len(x): %d, offset: %d, numbits: %d'
              % (len(x), offset, numbits))
        return
    if isinstance(x[0], (bytes, str)):
        temp = ord(x[0])
        if len(x) == 2:
            temp *= 256
            temp += ord(x[1])
    else:
        temp = x[0]
        if len(x) == 2:
            temp *= 256
            temp += x[1]
    temp = temp >> ((offset + 1) - numbits)
    temp %= 2**numbits
    return temp


def get_bitrate(x):         # returns nominal bit rate IN MB/s
    if isinstance(x[0], (bytes, str)):
        rate = ord(x[0])
    else:
        rate = x[0]
    # take care here...
    # for rates <= 25.4Gb, both SFP and QSFP+ use one byte, in units of 100Mb
    # for rates >25.4Gb, both use 0xFF to indicate 'look elsewhere'
    # SFP uses byte 66 (vs byte 12), hence offset is 54 bytes
    # QSFP uses byte 222 (vs byte 140), hence offset is 82 bytes
    # both specify rate in units of 250Mb for extended byte
    if rate == 255:
        if len(x) == 55:  # SFP
            if isinstance(x[54], (bytes, str)):
                rate = ord(x[54]) * 250
            else:
                rate = x[54] * 250
        elif len(x) == 83:   # QSFP+
            if isinstance(x[82], (bytes, str)):
                rate = ord(x[82]) * 250
            else:
                rate = x[82] * 250
        else:
            print("can't decode bit rate")
            return
    else:
        rate = rate * 100
    return rate


def get_brmax(x):         # returns max bit rate IN MB/s
    if len(x) < 56:
        print("can't decode max bit rate")
        return

    if isinstance(x[0], (bytes, str)):
        rate = ord(x[0])

        # this is tricky...  If byte 12 is 0xFF, then the bit rate is in
        # byte 66 (in units of 250 MBd), and the limit range of bit rates is
        # byte 67, 'in units of +/- 1%'
        if rate == 255:          # special case, need to use byte 66, 67!
            rate = ord(x[54])    # byte 66 is rate in this case
            rate_max = rate * (250 + (2.5 * ord(x[55])))

        # if byte 12 is not 0xFF, then the upper bit rate is in byte 66,
        # 'specified in units of 1% above the nominal bit rate'
        # remember the rate here is byte 12, raw, it must be multiplied
        # by 100.  Be careful changing this formula!
        else:
            rate_max = rate * (100 + ord(x[54]))

    else:
        rate = x[0]
        if rate == 255:          # special case, need to use byte 66, 67!
            rate = x[54]    # byte 66 is rate in this case
            rate_max = rate * (250 + (2.5 * (x[55])))
        else:
            rate_max = rate * (100 + x[54])

    return rate_max


def get_brmin(x):         # returns minimum bit rate IN MB/s
    if len(x) < 56:
        print("can't decode min bit rate")
        return
    if isinstance(x[0], (bytes, str)):
        rate = ord(x[0])

        # this is tricky...  If byte 12 is 0xFF, then the bit rate is in
        # byte 66 (in units of 250 MBd), and the limit range of bit rates is
        # byte 67, 'in units of +/- 1%'
        if rate == 255:          # special case, need to use byte 66, 67!
            rate = ord(x[54])    # byte 66 is rate in this case
            rate_min = rate * (250 - (2.5 * ord(x[55])))

        # if byte 12 is not 0xFF, then the upper bit rate is in byte 66,
        # 'specified in units of 1% above the nominal bit rate'
        # remember the rate here is byte 12, raw, it must be multiplied
        # by 100.  Be careful changing this formula!
        else:
            rate_min = rate * (100 - ord(x[55]))
    else:
        rate = x[0]
        if rate == 255:          # special case, need to use byte 66, 67!
            rate = x[54]    # byte 66 is rate in this case
            rate_min = rate * (250 - (2.5 * (x[55])))
        else:
            rate_min = rate * (100 - x[55])

    return rate_min


def get_length_km(x):   # returns supported link length in meters
    if isinstance(x[0], (bytes, str)):
        return ord(x[0]) * 1000
    else:
        return x[0] * 1000


def get_length_100m(x):   # returns supported link length in meters
    if isinstance(x[0], (bytes, str)):
        return ord(x[0]) * 100
    else:
        return x[0] * 100


def get_length_10m(x):   # returns supported link length in meters
    if isinstance(x[0], (bytes, str)):
        return ord(x[0]) * 10
    else:
        return x[0] * 10


def get_length_2m(x):   # returns supported link length in meters
    if isinstance(x[0], (bytes, str)):
        return ord(x[0]) * 2
    else:
        return x[0] * 2


def get_length_omcu(x):   # SFP: length in meters, optical OR COPPER
    if len(x) < 11:
        print("can't decode OM4/CU max cable length")
        return
    if isinstance(x[0], (bytes, str)):
        valid = ord(x[0])    # get byte 8
        valid %= 16          # strip bits above 3
        valid /= 4           # lose bits below 2
        if valid == 0:       # if bits 2 and 3 are 0, then optical
            return ord(x[10]) * 10  # Optical, stored value is in 10s of meters
        return ord(x[1])    # Copper, stored value is in meters
    else:
        valid = x[0]    # get byte 8
        valid %= 16          # strip bits above 3
        valid /= 4           # lose bits below 2
        if valid == 0:       # if bits 2 and 3 are 0, then optical
            return x[10] * 10  # Optical, stored value is in 10s of meters
        return x[1]    # Copper, stored value is in meters


def get_length_omcu2(x):   # QSFP+: length in meters, optical OR COPPER
    if len(x) < 2:
        print("can't decode OM4/CU max cable length")
        return
    if isinstance(x[1], (bytes, str)):
        txtech = ord(x[1])/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech == 0:      # 850 nm VCSEL
            return ord(x[0]) * 2  # OM4, stored value is in units of 2 meters
        return ord(x[0])    # Copper, stored value is in meters
    else:
        txtech = x[1]/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech == 0:      # 850 nm VCSEL
            return x[0] * 2  # OM4, stored value is in units of 2 meters
        return x[0]    # Copper, stored value is in meters


def get_wavelength(x):   # SFP: requires byte 8 and byte 60, 61
    if len(x) < 54:
        print("can't decode wavelength")
        return

    wavelength = 0
    if isinstance(x[0], (bytes, str)):
        valid = ord(x[0])    # get byte 8
        valid %= 16          # strip bits above 3
        valid /= 4           # lose bits below 2
        if valid == 0:       # if bits 2 and 3 are 0, then calculate wavelength
            wavelength = ord(x[52])*256 + ord(x[53])
    else:
        valid = x[0]    # get byte 8
        valid %= 16          # strip bits above 3
        valid /= 4           # lose bits below 2
        if valid == 0:       # if bits 2 and 3 are 0, then calculate wavelength
            wavelength = x[52]*256 + x[53]
    return wavelength


def get_cablespec(x):    # requires byte 8 and byte 60, 61
    if len(x) < 54:
        print("can't decode cable spec")
        return
    if isinstance(x[0], (bytes, str)):
        valid = ord(x[0])    # get byte 8
    else:
        valid = x[0]    # get byte 8
    valid %= 16          # strip bits above 3
    valid /= 4           # lose bits below 2
    result = x[52:54]
    if valid == 0:       # optical, cable spec doesn't apply
        result = b'\x00\x00'
    return result


def get_wavelength2(x):   # QSFP: requires byte 147, 186, 187
    if len(x) < 41:
        print("can't decode wavelength")
        return
    if isinstance(x[1], (bytes, str)):
        txtech = ord(x[1])/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech >= 10:    # copper technology
            return 0
        wavelen = ord(x[39])*256 + ord(x[40])
        wavelen = wavelen * 0.05  # value is 20ths of a nanometer!
    else:
        txtech = x[1]/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech >= 10:    # copper technology
            return 0
        wavelen = x[39]*256 + x[40]
        wavelen = wavelen * 0.05  # value is 20ths of a nanometer!

    return wavelen


def get_wave_tol(x):   # 2 bytes, in 200ths of a nm, return value in nm
    if len(x) < 2:
        print("can't decode wavelength tolerance")
        return
    if isinstance(x[0], (bytes, str)):
        wave_tol = ord(x[0])*256 + ord(x[1])
    else:
        wave_tol = x[0]*256 + x[1]
    wave_tol = wave_tol * 0.005  # value is 200ths of a nm
    return wave_tol


def get_CU_2_5(x):    # requires byte 147, 186
    if len(x) < 40:
        print("can't decode copper attenuation")
        return
    if isinstance(x[1], (bytes, str)):
        txtech = ord(x[1])/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech >= 10:    # copper technology
            return ord(x[39])
    else:
        txtech = x[1]/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech >= 10:    # copper technology
            return x[39]
    return 0


def get_CU_5_0(x):    # requires byte 147, 187
    if len(x) < 41:
        print("can't decode copper attenuation")
        return
    if isinstance(x[1], (bytes, str)):
        txtech = ord(x[1])/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech >= 10:    # copper technology
            return ord(x[40])
    else:
        txtech = x[1]/16     # Transmitter Technology, byte 147, bits 7-4
        if txtech >= 10:    # copper technology
            return x[40]

    return 0


def get_freq(x):    # Extract frequency (CFP)
    if len(x) < 4:
        print("can't decode frequency")
        return

    if isinstance(x[0], (bytes, str)):
        # low order bits of first two words are freq in THz
        freq = ord(x[0]) * 256
        freq += ord(x[1])
        subfreq = ord(x[2]) * 256
        subfreq += ord(x[3])
    else:
        # low order bits of first two words are freq in THz
        freq = x[0] * 256
        freq += x[1]
        subfreq = x[2] * 256
        subfreq += x[3]

    subfreq *= .00005   # specified by the spec, ie .05 GHz = .00005 THz
    freq += subfreq
    return freq


def get_hexstr(x):
    result = ''
    for i in x:
        if isinstance(i, int):
            result += hex(i)
        else:
            result += hex(ord(i))
        result += ' '
    return result


# CFP/MDIO likes to use only the low byte of each word.  This function
# squeezes out the zeros in the upper bytes.
def collapse_cfp(data):
    if len(data) < 2:
        return ''
    newdata = create_string_buffer(int(len(data)/2))
    i = 0
    for c in data:
        if (i % 2) == 1:
            newdata[int(i/2)] = c
        i += 1
    return newdata


# This routine should exactly undo collapse_cfp
# (except, expand_cfp(collapse_cfp(string)) will delete the last byte
# if string is of odd length)
def expand_cfp(data):
    newdata = create_string_buffer(int(len(data)*2))
    i = 0
    for c in data:
        newdata[i*2] = '\0'
        newdata[i*2+1] = c
        i += 1
    return newdata


# Note that set_int returns a 'C string' suitable for oom_set_memory_sff
def set_int(current, new):
    retlen = len(current)
    retval = create_string_buffer(retlen)
    temp = new
    for i in range(retlen):
        try:                   # python 2.7 flavor
            retval[(retlen - 1) - i] = chr(temp % 256)
        except:                # python 3 flavor
            retval[(retlen - 1) - i] = temp % 256
        temp = int(temp/256)
    return retval


# insert the low order 'numbits' from 'new' into 'current',
# starting at 'offset'.  This is the reverse
# of get_bits(x, offset, numbits)
# high order is bit 7, low order is bit 0
def set_bits(current, new, offset, numbits):
    if (len(current) != 1) or (offset > 7) or (offset < 0) or \
            (numbits > 7) or (numbits < 1) or \
            ((offset - numbits) < -1) or (new > 0xFF):
        print('set_bits bad parameters')
        return
    tempcurrent = ord(current[0])

    # Set the target bits in tempcurrent to all 1s
    mask = 0xFF >> 8 - numbits
    mask = mask << ((offset + 1) - numbits)
    tempcurrent = tempcurrent | mask

    # align the new bits, and mask the non-target bits
    tempnew = new << ((offset + 1) - numbits)
    mask = ~mask & 0xFF
    tempnew = tempnew | (mask)

    # mash them together
    newval = tempnew & tempcurrent

    # package the result for oom_set_memory_sff
    retval = create_string_buffer(1)
    try:                   # python 2.7 flavor
        retval[0] = chr(newval % 256)
    except:                # python 3 flavor
        retval[0] = newval % 256

    return retval


# turn a temperature (floating point python) into a 2 byte module
# temperature.  [Reverses the calculations of get_temperature()]
def set_temperature(current, new):
    if len(current) != 2:
        print("wrong temperature format")
        return
    try:
        retval = create_string_buffer(2)
        temp = new * 256.0
        if temp < 0:
            temp += 65536
        retval[0] = chr(int(temp / 256))
        retval[1] = chr(int(temp % 256))
    except Exception as err:
        print("Failed to set temperature. Error: "+str(err))
        return

    return retval

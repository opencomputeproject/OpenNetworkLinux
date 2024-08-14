# cmis.py
# memory map for the CMIS spec (qsfp-dd, OFSP 8X, QSFP+ using CMIS)
# based on "Common Management Interface Working DRAFT Specification
# for 8x/16x pluggable transceivers.  Rev 2.7, Feb 23, 2018
#
# Note that values that range from 0-n are returned as integers (get_int)
# Keys encoded as multi-byte fields are returned as raw bytes (get_bytes)
# Keys encoded as bit fields of 8 bits or less are integers (get_bits)
#     with the bit field in the low order bits
#
# Key attributes are:
#   Dynamic: 0 if not dynamic, 1 if it is (don't cache dynamic keys)
#   Decoder: name of the decoder function (these are in decode.py)
#   Addr: i2c address (usually 0xA0 or 0xA2)
#   Page: page the data lives in
#   Offset: byte address where the data starts
#       Note: if offset < 128, then the page value is not used
#       Offsets from 0-127 are in the low memory of Addr
#   Length: number of bytes of data for this key
#   Bit Offset: (optional), highest order bit, within a byte, of a bit field
#   Bit Length: (optional), number of bits in a bit field
#       Note: Bit offset and bit length are only used by get_bits
#       LSB is bit 0, MSB is bit 7


new_mm_keys = {        # dynamic?, decoder, addr, page, offset,length, BO, BL
    # ID and status bytes (0-2)
    # Note, per the spec: Page 00h Byte 0 and Page 00h Byte 128 shall
    # contain the same parameter values.
    # Use the other one, because it is on a page of static (cachable) data
    #    'IDENTIFIER':       (0, 'get_int', 0xA0, 0, 0, 1),
    'REV_COMPLIANCE':   (0, 'get_int', 0xA0, 0, 1, 1),
    'FLAT_MEM':         (0, 'get_bits', 0xA0, 0, 2, 1, 7, 1),

    'L_TEMP_ALARM_WARN':   (1, 'get_bits', 0xA0, 0, 9, 1, 3, 4),  # all 4 temps
    'L_TEMP_HIGH_ALARM':   (1, 'get_bits', 0xA0, 0, 9, 1, 0, 1),
    'L_TEMP_LOW_ALARM':    (1, 'get_bits', 0xA0, 0, 9, 1, 1, 1),
    'L_TEMP_HIGH_WARNING': (1, 'get_bits', 0xA0, 0, 9, 1, 2, 1),
    'L_TEMP_LOW_WARNING':  (1, 'get_bits', 0xA0, 0, 9, 1, 3, 1),

    'L_VCC_ALARM_WARN': (1, 'get_bits', 0xA0, 0, 9, 1, 7, 4),  # all 4 VCC
    'L_VCC_HIGH_ALARM': (1, 'get_bits', 0xA0, 0, 9, 1, 4, 1),
    'L_VCC_LOW_ALARM':  (1, 'get_bits', 0xA0, 0, 9, 1, 5, 1),
    'L_VCC_HIGH_WARN':  (1, 'get_bits', 0xA0, 0, 9, 1, 6, 1),
    'L_VCC_LOW_WARN':   (1, 'get_bits', 0xA0, 0, 9, 1, 7, 1),

    # Free Side Device Monitors Bytes
    'TEMPERATURE':            (1, 'get_temperature', 0xA0, 0, 14, 2),
    'SUPPLY_VOLTAGE':         (1, 'get_voltage', 0xA0, 0, 16, 2),

    # Channel Monitors Bytes (current values, not alarms/warnings)
    # These are on the 'bank' pages, we are only looking at bank 1
    'RX1_POWER':        (1, 'get_power', 0xA0, 0x11, 186, 2),
    'RX1_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 186, 2),
    'RX2_POWER':        (1, 'get_power', 0xA0, 0x11, 188, 2),
    'RX2_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 188, 2),
    'RX3_POWER':        (1, 'get_power', 0xA0, 0x11, 190, 2),
    'RX3_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 190, 2),
    'RX4_POWER':        (1, 'get_power', 0xA0, 0x11, 192, 2),
    'RX4_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 192, 2),
    'RX5_POWER':        (1, 'get_power', 0xA0, 0x11, 194, 2),
    'RX5_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 194, 2),
    'RX6_POWER':        (1, 'get_power', 0xA0, 0x11, 196, 2),
    'RX6_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 196, 2),
    'RX7_POWER':        (1, 'get_power', 0xA0, 0x11, 198, 2),
    'RX7_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 198, 2),
    'RX8_POWER':        (1, 'get_power', 0xA0, 0x11, 200, 2),
    'RX8_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 200, 2),
    'TX1_BIAS':         (1, 'get_current', 0xA0, 0x11, 170, 2),
    'TX2_BIAS':         (1, 'get_current', 0xA0, 0x11, 172, 2),
    'TX3_BIAS':         (1, 'get_current', 0xA0, 0x11, 174, 2),
    'TX4_BIAS':         (1, 'get_current', 0xA0, 0x11, 176, 2),
    'TX5_BIAS':         (1, 'get_current', 0xA0, 0x11, 178, 2),
    'TX6_BIAS':         (1, 'get_current', 0xA0, 0x11, 180, 2),
    'TX7_BIAS':         (1, 'get_current', 0xA0, 0x11, 182, 2),
    'TX8_BIAS':         (1, 'get_current', 0xA0, 0x11, 184, 2),
    'TX1_POWER':        (1, 'get_power', 0xA0, 0x11, 154, 2),
    'TX1_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 154, 2),
    'TX2_POWER':        (1, 'get_power', 0xA0, 0x11, 156, 2),
    'TX2_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 156, 2),
    'TX3_POWER':        (1, 'get_power', 0xA0, 0x11, 158, 2),
    'TX3_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 158, 2),
    'TX4_POWER':        (1, 'get_power', 0xA0, 0x11, 160, 2),
    'TX4_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 160, 2),
    'TX5_POWER':        (1, 'get_power', 0xA0, 0x11, 162, 2),
    'TX5_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 162, 2),
    'TX6_POWER':        (1, 'get_power', 0xA0, 0x11, 164, 2),
    'TX6_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 164, 2),
    'TX7_POWER':        (1, 'get_power', 0xA0, 0x11, 166, 2),
    'TX7_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 166, 2),
    'TX8_POWER':        (1, 'get_power', 0xA0, 0x11, 168, 2),
    'TX8_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0x11, 168, 2),

    # Page 0, Serial ID fields
    # Note, per the spec: Page 00h Byte 0 and Page 00h Byte 128 shall
    # contain the same parameter values.
    # Use this one, because it is on a page of static (cachable) data
    'IDENTIFIER':       (0, 'get_int', 0xA0, 0, 128, 1),
    'VENDOR_NAME':      (0, 'get_string', 0xA0, 0, 129, 16),
    'VENDOR_OUI':       (0, 'get_bytes', 0xA0, 0, 145, 3),
    'VENDOR_PN':        (0, 'get_string', 0xA0, 0, 148, 16),
    'VENDOR_REV':       (0, 'get_string', 0xA0, 0, 164, 2),
    'VENDOR_SN':        (0, 'get_string', 0xA0, 0, 166, 16),
    'DATE_CODE':        (0, 'get_string', 0xA0, 0, 182, 8),

    'TEMP_HIGH_ALARM':  (0, 'get_temperature', 0xA0, 2, 128, 2),
    'TEMP_LOW_ALARM':   (0, 'get_temperature', 0xA0, 2, 130, 2),
    'TEMP_HIGH_WARN':   (0, 'get_temperature', 0xA0, 2, 132, 2),
    'TEMP_LOW_WARN':    (0, 'get_temperature', 0xA0, 2, 134, 2),

    'VOLTAGE_HIGH_ALARM': (0, 'get_voltage', 0xA0, 2, 136, 2),
    'VOLTAGE_LOW_ALARM':  (0, 'get_voltage', 0xA0, 2, 138, 2),
    'VOLTAGE_HIGH_WARN':  (0, 'get_voltage', 0xA0, 2, 140, 2),
    'VOLTAGE_LOW_WARN':   (0, 'get_voltage', 0xA0, 2, 142, 2),

    'BIAS_HIGH_ALARM':  (0, 'get_current', 0xA0, 2, 184, 2),
    'BIAS_LOW_ALARM':   (0, 'get_current', 0xA0, 2, 186, 2),
    'BIAS_HIGH_WARN':   (0, 'get_current', 0xA0, 2, 188, 2),
    'BIAS_LOW_WARN':    (0, 'get_current', 0xA0, 2, 190, 2),

    'TX_POWER_HIGH_ALARM': (0, 'get_power_dbm', 0xA0, 2, 176, 2),
    'TX_POWER_LOW_ALARM':  (0, 'get_power_dbm', 0xA0, 2, 178, 2),
    'TX_POWER_HIGH_WARN':  (0, 'get_power_dbm', 0xA0, 2, 180, 2),
    'TX_POWER_LOW_WARN':   (0, 'get_power_dbm', 0xA0, 2, 182, 2),

    'RX_POWER_HIGH_ALARM': (0, 'get_power_dbm', 0xA0, 2, 192, 2),
    'RX_POWER_LOW_ALARM':  (0, 'get_power_dbm', 0xA0, 2, 194, 2),
    'RX_POWER_HIGH_WARN':  (0, 'get_power_dbm', 0xA0, 2, 196, 2),
    'RX_POWER_LOW_WARN':   (0, 'get_power_dbm', 0xA0, 2, 198, 2),
    }


new_fm_keys = {
    'SERIAL_ID': ('IDENTIFIER',
                  # 'EXT_IDENTIFIER',
                  # 'CONNECTOR',
                  # 'SPEC_COMPLIANCE',
                  # 'ENCODING',
                  # 'BR_NOMINAL',
                  # 'EXT_RATE_COMPLY',
                  # 'LENGTH_SMF_KM',
                  # 'LENGTH_OM3_50UM',
                  # 'LENGTH_OM2_50UM',
                  # 'LENGTH_OM1_62_5UM',
                  # 'LENGTH_OM4_OR_CU',
                  # 'DEVICE_TECH',
                  'VENDOR_NAME',
                  # 'EXTENDED_MODULE',
                  'VENDOR_OUI',
                  'VENDOR_PN',
                  'VENDOR_REV',
                  'VENDOR_SN',
                  'DATE_CODE',
                  # 'WAVELENGTH',
                  # 'CU_ATTENUATE_2_5',
                  # 'CU_ATTENUATE_5_0',
                  # 'WAVELEN_TOLERANCE',
                  # 'MAX_CASE_TEMP'
                  ),


    'DOM':      ('TEMPERATURE',
                 'SUPPLY_VOLTAGE',
                 'TX1_BIAS',
                 'TX2_BIAS',
                 'TX3_BIAS',
                 'TX4_BIAS',
                 'TX5_BIAS',
                 'TX6_BIAS',
                 'TX7_BIAS',
                 'TX8_BIAS',
                 'TX1_POWER',
                 'TX2_POWER',
                 'TX3_POWER',
                 'TX4_POWER',
                 'TX5_POWER',
                 'TX6_POWER',
                 'TX7_POWER',
                 'TX8_POWER',
                 'RX1_POWER',
                 'RX2_POWER',
                 'RX3_POWER',
                 'RX4_POWER',
                 'RX5_POWER',
                 'RX6_POWER',
                 'RX7_POWER',
                 'RX8_POWER',
                 )
    }


new_wm_keys = {
        # 'TX4_DISABLE': 'set_bits',
        # 'TX3_DISABLE': 'set_bits',
        # 'TX2_DISABLE': 'set_bits',
        # 'TX1_DISABLE': 'set_bits',
        # 'PASSWORD_CHANGE': 'set_int',
        # 'PASSWORD_ENTRY': 'set_int',
       }


# add these keys to the memory map, function map, write map for
# QSFP-DD (OSFP, COBO) type devices
def add_keys(port):
    QSFP_DD = 0x18
    OSFP = 0x19
    QSFPwCMIS = 0x1E
    if (port.port_type != QSFP_DD) and \
       (port.port_type != OSFP) and \
       (port.port_type != QSFPwCMIS):
        return
    port.mmap.update(new_mm_keys)
    port.fmap.update(new_fm_keys)
    port.wmap.update(new_wm_keys)

# sfp.py
# sfp, dwdm_sfp memory map
# based on SFF-8472, ver 12.2 and SFF-8024
# Table 4-1, page 14 and table 4-2, page 15
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


new_mm_keys = {         # dynamic?, decoder, addr, page, offset,length, BO, BL
     'IDENTIFIER':       (0, 'get_int', 0xA0, 0, 0, 1),
     'EXT_IDENTIFIER':   (0, 'get_int', 0xA0, 0, 1, 1),
     'CONNECTOR':        (0, 'get_int', 0xA0, 0, 2, 1),
     'TRANSCEIVER':      (0, 'get_bytes', 0xA0, 0, 3, 8),       # see table 5-3
     'ENCODING':         (0, 'get_int', 0xA0, 0, 11, 1),
     'BR_NOMINAL':       (0, 'get_bitrate', 0xA0, 0, 12, 55),  # bytes 12 & 66!
     'RATE_IDENTIFIER':  (0, 'get_int', 0xA0, 0, 13, 1),

     'LENGTH_SMF_KM':    (0, 'get_length_km', 0xA0, 0, 14, 1),
     'LENGTH_SMF':       (0, 'get_length_100m', 0xA0, 0, 15, 1),
     'LENGTH_50UM':      (0, 'get_length_10m', 0xA0, 0, 16, 1),
     'LENGTH_62_5UM':    (0, 'get_length_10m', 0xA0, 0, 17, 1),
     'LENGTH_OM4_OR_CU': (0, 'get_length_omcu', 0xA0, 0, 8, 11),  # bytes 8&18
     'LENGTH_OM3':       (0, 'get_length_10m', 0xA0, 0, 19, 1),

     'VENDOR_NAME':      (0, 'get_string', 0xA0, 0, 20, 16),
     'TRANSCEIVER_EXT':  (0, 'get_int', 0xA0, 0, 36, 1),      # 8024, table 4-4
     'VENDOR_OUI':       (0, 'get_bytes', 0xA0, 0, 37, 3),
     'VENDOR_PN':        (0, 'get_string', 0xA0, 0, 40, 16),
     'VENDOR_REV':       (0, 'get_string', 0xA0, 0, 56, 4),
     'WAVELENGTH':       (0, 'get_wavelength', 0xA0, 0, 8, 54),  # 1 field,
     'CABLE_SPEC':       (0, 'get_cablespec', 0xA0, 0, 8, 54),   # 2 keys

     'OPTIONS':          (0, 'get_bytes', 0xA0, 0, 64, 2),
     'BR_MAX':           (0, 'get_brmax', 0xA0, 0, 12, 56),  # bytes 12, 66, 67
     'BR_MIN':           (0, 'get_brmin', 0xA0, 0, 12, 56),  # bytes 12, 66, 67
     'VENDOR_SN':        (0, 'get_string', 0xA0, 0, 68, 16),
     'DATE_CODE':        (0, 'get_string', 0xA0, 0, 84, 8),
     'DIAGNOSTIC_MONITORING_TYPE': (0, 'get_int', 0xA0, 0, 92, 1),
     'ENHANCED_OPTIONS': (0, 'get_int', 0xA0, 0, 93, 1),
     'SFF_8472_COMPLIANCE': (0, 'get_int', 0xA0, 0, 94, 1),
     'VENDOR_SPECIFIC_96': (0, 'get_bytes', 0xA0, 0, 96, 32),

     'TEMPERATURE':      (1, 'get_temperature', 0xA2, 0, 96, 2),
     'VCC':              (1, 'get_voltage', 0xA2, 0, 98, 2),
     'TX_BIAS':          (1, 'get_current', 0xA2, 0, 100, 2),
     'TX_POWER':         (1, 'get_power', 0xA2, 0, 102, 2),
     'TX_POWER_DBM':     (1, 'get_power_dbm', 0xA2, 0, 102, 2),
     'RX_POWER':         (1, 'get_power', 0xA2, 0, 104, 2),
     'RX_POWER_DBM':     (1, 'get_power_dbm', 0xA2, 0, 104, 2),
     'OPT_LASER_TEMP':   (1, 'get_temperature', 0xA2, 0, 106, 2),
     'OPT_TEC':          (1, 'get_signed_current', 0xA2, 0, 108, 2),

     'STATUS_CONTROL':         (1, 'get_bits', 0xA2, 0, 110, 1, 7, 8),
     'TX_DISABLE_STATE':       (1, 'get_bits', 0xA2, 0, 110, 1, 7, 1),
     'SOFT_TX_DISABLE_SELECT': (1, 'get_bits', 0xA2, 0, 110, 1, 6, 1),
     'RS_1_STATE':             (1, 'get_bits', 0xA2, 0, 110, 1, 5, 1),
     'RATE_SELECT_STATE':      (1, 'get_bits', 0xA2, 0, 110, 1, 4, 1),
     'SOFT_RATE_SELECT':       (1, 'get_bits', 0xA2, 0, 110, 1, 3, 1),
     'TX_FAULT_STATE':         (1, 'get_bits', 0xA2, 0, 110, 1, 2, 1),
     'RX_LOS_STATE':           (1, 'get_bits', 0xA2, 0, 110, 1, 1, 1),
     'DATA_READY_BAR_STATE':   (1, 'get_bits', 0xA2, 0, 110, 1, 0, 1),

     # Thresholds and latched alarms/warnings for:
     # {temperature, voltage, laser bias, Tx_Power, Rx_Power}
     # {High, Low} {Alarm, Warning}
     # In other words, 5 values have high/low alarm/warning thresholds
     # And each has a latched value (L_*) indicating whether it is set
     # The latched values are integers: bit 0 is always the 'low' alarm
     # or warning, bit 1 is always the 'high' alarm or warning
     # so if L_TEMP_ALARM is 2, that means there is a high temp alarm
     'TEMP_HIGH_ALARM':        (0, 'get_temperature', 0xA2, 0, 0, 2),
     'TEMP_LOW_ALARM':         (0, 'get_temperature', 0xA2, 0, 2, 2),
     'TEMP_HIGH_WARN':         (0, 'get_temperature', 0xA2, 0, 4, 2),
     'TEMP_LOW_WARN':          (0, 'get_temperature', 0xA2, 0, 6, 2),
     'L_TEMP_ALARM':           (1, 'get_bits', 0xA2, 0, 112, 1, 7, 2),
     'L_TEMP_WARN':            (1, 'get_bits', 0xA2, 0, 116, 1, 7, 2),

     'VOLTAGE_HIGH_ALARM':     (0, 'get_voltage', 0xA2, 0, 8, 2),
     'VOLTAGE_LOW_ALARM':      (0, 'get_voltage', 0xA2, 0, 10, 2),
     'VOLTAGE_HIGH_WARN':      (0, 'get_voltage', 0xA2, 0, 12, 2),
     'VOLTAGE_LOW_WARN':       (0, 'get_voltage', 0xA2, 0, 14, 2),
     'L_VCC_ALARM':            (1, 'get_bits', 0xA2, 0, 112, 1, 5, 2),
     'L_VCC_WARN':             (1, 'get_bits', 0xA2, 0, 116, 1, 5, 2),

     'BIAS_HIGH_ALARM':        (0, 'get_current', 0xA2, 0, 16, 2),
     'BIAS_LOW_ALARM':         (0, 'get_current', 0xA2, 0, 18, 2),
     'BIAS_HIGH_WARN':         (0, 'get_current', 0xA2, 0, 20, 2),
     'BIAS_LOW_WARN':          (0, 'get_current', 0xA2, 0, 22, 2),
     'L_BIAS_ALARM':           (1, 'get_bits', 0xA2, 0, 112, 1, 3, 2),
     'L_BIAS_WARN':            (1, 'get_bits', 0xA2, 0, 116, 1, 3, 2),

     'TX_POWER_HIGH_ALARM':    (0, 'get_power_dbm', 0xA2, 0, 24, 2),
     'TX_POWER_LOW_ALARM':     (0, 'get_power_dbm', 0xA2, 0, 26, 2),
     'TX_POWER_HIGH_WARN':     (0, 'get_power_dbm', 0xA2, 0, 28, 2),
     'TX_POWER_LOW_WARN':      (0, 'get_power_dbm', 0xA2, 0, 30, 2),
     'L_TX_POWER_ALARM':       (1, 'get_bits', 0xA2, 0, 112, 1, 1, 2),
     'L_TX_POWER_WARN':        (1, 'get_bits', 0xA2, 0, 116, 1, 1, 2),

     'RX_POWER_HIGH_ALARM':    (0, 'get_power_dbm', 0xA2, 0, 32, 2),
     'RX_POWER_LOW_ALARM':     (0, 'get_power_dbm', 0xA2, 0, 34, 2),
     'RX_POWER_HIGH_WARN':     (0, 'get_power_dbm', 0xA2, 0, 36, 2),
     'RX_POWER_LOW_WARN':      (0, 'get_power_dbm', 0xA2, 0, 38, 2),
     'L_RX_POWER_ALARM':       (1, 'get_bits', 0xA2, 0, 113, 1, 7, 2),
     'L_RX_POWER_WARN':        (1, 'get_bits', 0xA2, 0, 117, 1, 7, 2),

     # all of the latched alarms and warnings in 6 consecutive bytes
     # (includes 'unallocated' byte 114, and 'CDR unlocked' byte 115)
     # If the first two and last two are zeros, then no alarms or warnings
     # are currently pending
     'L_ALARM_WARN':           (1, 'get_bytes', 0xA2, 0, 112, 6),
    }

new_fm_keys = {
    'SERIAL_ID': ('IDENTIFIER',
                  'EXT_IDENTIFIER',
                  'CONNECTOR',
                  'TRANSCEIVER',
                  'ENCODING',
                  'BR_NOMINAL',
                  'RATE_IDENTIFIER',
                  'LENGTH_SMF_KM',
                  'LENGTH_SMF',
                  'LENGTH_50UM',
                  'LENGTH_62_5UM',
                  'LENGTH_OM4_OR_CU',
                  'LENGTH_OM3',
                  'VENDOR_NAME',
                  'TRANSCEIVER_EXT',
                  'VENDOR_OUI',
                  'VENDOR_PN',
                  'VENDOR_REV',
                  'WAVELENGTH',
                  'CABLE_SPEC',
                  'OPTIONS',
                  'BR_MAX',
                  'BR_MIN',
                  'VENDOR_SN',
                  'DATE_CODE',
                  'DIAGNOSTIC_MONITORING_TYPE',
                  'ENHANCED_OPTIONS',
                  'SFF_8472_COMPLIANCE'
                  ),

    'DOM': ('TEMPERATURE', 'VCC', 'TX_BIAS', 'TX_POWER',
            'RX_POWER'),
    }


new_wm_keys = {
         'SOFT_TX_DISABLE_SELECT': 'set_bits',
       }


# add these keys to the memory map, function map, write map for
# SFP and DWDM_SFP devices
def add_keys(port):
    SFP = 0x3
    DWDM_SFP = 0xB
    if (port.port_type != SFP) and (port.port_type != DWDM_SFP):
        return
    port.mmap.update(new_mm_keys)
    port.fmap.update(new_fm_keys)
    port.wmap.update(new_wm_keys)

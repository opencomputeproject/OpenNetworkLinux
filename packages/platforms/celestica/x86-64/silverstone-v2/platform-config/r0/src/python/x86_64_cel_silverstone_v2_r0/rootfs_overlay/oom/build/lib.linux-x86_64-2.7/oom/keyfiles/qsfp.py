# qsfp.py
# qsfp+, qsfp28 memory map
# based on SFF-8636, rev 2.6, Section 6, and SFF-8024
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
    'FLAT_MEM':         (0, 'get_bits', 0xA0, 0, 2, 1, 2, 1),
    'INT_L':            (1, 'get_bits', 0xA0, 0, 2, 1, 1, 1),
    'DATA_NOT_READY':   (1, 'get_bits', 0xA0, 0, 2, 1, 0, 1),

    # Interrupt Flags bytes (3-21)
    # The prefix L_ is for 'latched', all of these are indicators of
    # (generally) bad things, TX (transmit), RX (receive), bias (laser bias)
    # temp (temperature), VCC (voltage), etc can trigger high or low,
    # alarms or warnings.  All but temp and VCC are per-channel.
    #
    'L_TX_RX_LOS':      (1, 'get_bits', 0xA0, 0, 3, 1, 7, 8),  # 8 LOS bits
    'L_TX4_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 7, 1),
    'L_TX3_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 6, 1),
    'L_TX2_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 5, 1),
    'L_TX1_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 4, 1),
    'L_RX4_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 3, 1),
    'L_RX3_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 2, 1),
    'L_RX2_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 1, 1),
    'L_RX1_LOS':        (1, 'get_bits', 0xA0, 0, 3, 1, 0, 1),

    'L_TX_FAULT':       (1, 'get_bits', 0xA0, 0, 4, 1, 7, 8),  # 8 Fault bits
    'L_TX4_ADAPT_EQ_FAULT': (1, 'get_bits', 0xA0, 0, 4, 1, 7, 1),
    'L_TX3_ADAPT_EQ_FAULT': (1, 'get_bits', 0xA0, 0, 4, 1, 6, 1),
    'L_TX2_ADAPT_EQ_FAULT': (1, 'get_bits', 0xA0, 0, 4, 1, 5, 1),
    'L_TX1_ADAPT_EQ_FAULT': (1, 'get_bits', 0xA0, 0, 4, 1, 4, 1),
    'L_TX4_FAULT':      (1, 'get_bits', 0xA0, 0, 4, 1, 3, 1),
    'L_TX3_FAULT':      (1, 'get_bits', 0xA0, 0, 4, 1, 2, 1),
    'L_TX2_FAULT':      (1, 'get_bits', 0xA0, 0, 4, 1, 1, 1),
    'L_TX1_FAULT':      (1, 'get_bits', 0xA0, 0, 4, 1, 0, 1),

    'L_TX_RX_LOL':      (1, 'get_bits', 0xA0, 0, 5, 1, 7, 8),  # 8 LOL bits
    'L_TX4_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 7, 1),  # Laugh Out Loud
    'L_TX3_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 6, 1),
    'L_TX2_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 5, 1),
    'L_TX1_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 4, 1),
    'L_RX4_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 3, 1),
    'L_RX3_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 2, 1),
    'L_RX2_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 1, 1),
    'L_RX1_LOL':        (1, 'get_bits', 0xA0, 0, 5, 1, 0, 1),

    'L_TEMP_ALARM_WARN':   (1, 'get_bits', 0xA0, 0, 6, 1, 7, 4),  # all 4 temps
    'L_TEMP_HIGH_ALARM':   (1, 'get_bits', 0xA0, 0, 6, 1, 7, 1),
    'L_TEMP_LOW_ALARM':    (1, 'get_bits', 0xA0, 0, 6, 1, 6, 1),
    'L_TEMP_HIGH_WARNING': (1, 'get_bits', 0xA0, 0, 6, 1, 5, 1),
    'L_TEMP_LOW_WARNING':  (1, 'get_bits', 0xA0, 0, 6, 1, 4, 1),
    'INIT_COMPLETE':    (1, 'get_bits', 0xA0, 0, 6, 1, 0, 1),

    'L_VCC_ALARM_WARN': (1, 'get_bits', 0xA0, 0, 7, 1, 7, 4),  # all 4 VCC
    'L_VCC_HIGH_ALARM': (1, 'get_bits', 0xA0, 0, 7, 1, 7, 1),
    'L_VCC_LOW_ALARM':  (1, 'get_bits', 0xA0, 0, 7, 1, 6, 1),
    'L_VCC_HIGH_WARN':  (1, 'get_bits', 0xA0, 0, 7, 1, 5, 1),
    'L_VCC_LOW_WARN':   (1, 'get_bits', 0xA0, 0, 7, 1, 4, 1),
    'VENDOR_SPECIFIC_8':  (0, 'get_bytes', 0xA0, 0, 8, 1),

    'L_RX1_RX2_POWER': (1, 'get_bits', 0xA0, 0, 9, 1, 7, 8),  # 8 power bits
    'L_RX1_POWER': (1, 'get_bits', 0xA0, 0, 9, 1, 7, 4),  # RX1 power bits
    'L_RX1_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 9, 1, 7, 1),
    'L_RX1_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 9, 1, 6, 1),
    'L_RX1_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 9, 1, 5, 1),
    'L_RX1_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 9, 1, 4, 1),
    'L_RX2_POWER': (1, 'get_bits', 0xA0, 0, 9, 1, 3, 4),  # RX2 power bits
    'L_RX2_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 9, 1, 3, 1),
    'L_RX2_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 9, 1, 2, 1),
    'L_RX2_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 9, 1, 1, 1),
    'L_RX2_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 9, 1, 0, 1),

    'L_RX3_RX4_POWER': (1, 'get_bits', 0xA0, 0, 10, 1, 7, 8),  # 8 power bits
    'L_RX3_POWER': (1, 'get_bits', 0xA0, 0, 10, 1, 7, 4),  # RX3 power bits
    'L_RX3_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 10, 1, 7, 1),
    'L_RX3_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 10, 1, 6, 1),
    'L_RX3_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 10, 1, 5, 1),
    'L_RX3_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 10, 1, 4, 1),
    'L_RX4_POWER': (1, 'get_bits', 0xA0, 0, 10, 1, 3, 4),  # RX4 power bits
    'L_RX4_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 10, 1, 3, 1),
    'L_RX4_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 10, 1, 2, 1),
    'L_RX4_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 10, 1, 1, 1),
    'L_RX4_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 10, 1, 0, 1),

    'L_TX1_TX2_BIAS': (1, 'get_bits', 0xA0, 0, 11, 1, 7, 8),  # 8 bias bits
    'L_TX1_BIAS': (1, 'get_bits', 0xA0, 0, 11, 1, 7, 4),  # TX1 bias bits
    'L_TX1_BIAS_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 11, 1, 7, 1),
    'L_TX1_BIAS_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 11, 1, 6, 1),
    'L_TX1_BIAS_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 11, 1, 5, 1),
    'L_TX1_BIAS_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 11, 1, 4, 1),
    'L_TX2_BIAS': (1, 'get_bits', 0xA0, 0, 11, 1, 3, 4),  # TX2 bias bits
    'L_TX2_BIAS_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 11, 1, 3, 1),
    'L_TX2_BIAS_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 11, 1, 2, 1),
    'L_TX2_BIAS_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 11, 1, 1, 1),
    'L_TX2_BIAS_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 11, 1, 0, 1),

    'L_TX3_TX4_BIAS': (1, 'get_bits', 0xA0, 0, 12, 1, 7, 8),  # 8 bias bits
    'L_TX3_BIAS': (1, 'get_bits', 0xA0, 0, 12, 1, 7, 4),  # TX3 bias bits
    'L_TX3_BIAS_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 12, 1, 7, 1),
    'L_TX3_BIAS_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 12, 1, 6, 1),
    'L_TX3_BIAS_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 12, 1, 5, 1),
    'L_TX3_BIAS_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 12, 1, 4, 1),
    'L_TX4_BIAS': (1, 'get_bits', 0xA0, 0, 12, 1, 3, 4),  # TX4 bias bits
    'L_TX4_BIAS_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 12, 1, 3, 1),
    'L_TX4_BIAS_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 12, 1, 2, 1),
    'L_TX4_BIAS_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 12, 1, 1, 1),
    'L_TX4_BIAS_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 12, 1, 0, 1),

    'L_TX1_TX2_POWER':  (1, 'get_bits', 0xA0, 0, 13, 1, 7, 8),  # 8 POWER bits
    'L_TX1_POWER':  (1, 'get_bits', 0xA0, 0, 13, 1, 7, 4),  # TX1 POWER bits
    'L_TX1_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 13, 1, 7, 1),
    'L_TX1_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 13, 1, 6, 1),
    'L_TX1_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 13, 1, 5, 1),
    'L_TX1_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 13, 1, 4, 1),
    'L_TX2_POWER':  (1, 'get_bits', 0xA0, 0, 13, 1, 3, 4),  # TX2 POWER bits
    'L_TX2_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 13, 1, 3, 1),
    'L_TX2_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 13, 1, 2, 1),
    'L_TX2_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 13, 1, 1, 1),
    'L_TX2_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 13, 1, 0, 1),

    'L_TX3_TX4_POWER': (1, 'get_bits', 0xA0, 0, 14, 1, 7, 8),  # 8 POWER bits
    'L_TX3_POWER':  (1, 'get_bits', 0xA0, 0, 14, 1, 7, 4),  # TX3 POWER bits
    'L_TX3_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 14, 1, 7, 1),
    'L_TX3_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 14, 1, 6, 1),
    'L_TX3_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 14, 1, 5, 1),
    'L_TX3_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 14, 1, 4, 1),
    'L_TX4_POWER':  (1, 'get_bits', 0xA0, 0, 14, 1, 3, 4),  # TX4 POWER bits
    'L_TX4_POWER_HIGH_ALARM':  (1, 'get_bits', 0xA0, 0, 14, 1, 3, 1),
    'L_TX4_POWER_LOW_ALARM':   (1, 'get_bits', 0xA0, 0, 14, 1, 2, 1),
    'L_TX4_POWER_HIGH_WARN':   (1, 'get_bits', 0xA0, 0, 14, 1, 1, 1),
    'L_TX4_POWER_LOW_WARN':    (1, 'get_bits', 0xA0, 0, 14, 1, 0, 1),

    'VENDOR_SPECIFIC_19':     (0, 'get_bytes', 0xA0, 0, 19, 3),

    # Free Side Device Monitors Bytes (22-33)
    'TEMPERATURE':            (1, 'get_temperature', 0xA0, 0, 22, 2),
    'SUPPLY_VOLTAGE':         (1, 'get_voltage', 0xA0, 0, 26, 2),
    'VENDOR_SPECIFIC_30':     (1, 'get_bytes', 0xA0, 0, 30, 4),

    # Channel Monitors Bytes (current values, not alarms/warnings) (34-81)
    'RX1_POWER':        (1, 'get_power', 0xA0, 0, 34, 2),
    'RX1_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 34, 2),
    'RX2_POWER':        (1, 'get_power', 0xA0, 0, 36, 2),
    'RX2_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 36, 2),
    'RX3_POWER':        (1, 'get_power', 0xA0, 0, 38, 2),
    'RX3_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 38, 2),
    'RX4_POWER':        (1, 'get_power', 0xA0, 0, 40, 2),
    'RX4_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 40, 2),
    'TX1_BIAS':         (1, 'get_current', 0xA0, 0, 42, 2),
    'TX2_BIAS':         (1, 'get_current', 0xA0, 0, 44, 2),
    'TX3_BIAS':         (1, 'get_current', 0xA0, 0, 46, 2),
    'TX4_BIAS':         (1, 'get_current', 0xA0, 0, 48, 2),
    'TX1_POWER':        (1, 'get_power', 0xA0, 0, 50, 2),
    'TX1_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 50, 2),
    'TX2_POWER':        (1, 'get_power', 0xA0, 0, 52, 2),
    'TX2_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 52, 2),
    'TX3_POWER':        (1, 'get_power', 0xA0, 0, 54, 2),
    'TX3_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 54, 2),
    'TX4_POWER':        (1, 'get_power', 0xA0, 0, 56, 2),
    'TX4_POWER_DBM':    (1, 'get_power_dbm', 0xA0, 0, 56, 2),
    'VENDOR_SPECIFIC_74':     (0, 'get_bytes', 0xA0, 0, 74, 8),

    # Control Bytes (86-98)
    'TX_DISABLE':       (1, 'get_bits', 0xA0, 0, 86, 1, 3, 4),  # low nibble
    'TX4_DISABLE':      (1, 'get_bits', 0xA0, 0, 86, 1, 3, 1),
    'TX3_DISABLE':      (1, 'get_bits', 0xA0, 0, 86, 1, 2, 1),
    'TX2_DISABLE':      (1, 'get_bits', 0xA0, 0, 86, 1, 1, 1),
    'TX1_DISABLE':      (1, 'get_bits', 0xA0, 0, 86, 1, 0, 1),

    'RX_RATE_SELECT':   (1, 'get_bytes', 0xA0, 0, 87, 1),  # all 8 Rate Select
    'RX4_RATE_SELECT':  (1, 'get2_bit6', 0xA0, 0, 87, 1),
    'RX3_RATE_SELECT':  (1, 'get2_bit4', 0xA0, 0, 87, 1),
    'RX2_RATE_SELECT':  (1, 'get2_bit2', 0xA0, 0, 87, 1),
    'RX1_RATE_SELECT':  (1, 'get2_bit0', 0xA0, 0, 87, 1),

    'TX_RATE_SELECT':   (1, 'get_bytes', 0xA0, 0, 88, 1),  # all 8 Rate Select
    'TX4_RATE_SELECT':  (1, 'get2_bit6', 0xA0, 0, 88, 1),
    'TX3_RATE_SELECT':  (1, 'get2_bit4', 0xA0, 0, 88, 1),
    'TX2_RATE_SELECT':  (1, 'get2_bit2', 0xA0, 0, 88, 1),
    'TX1_RATE_SELECT':  (1, 'get2_bit0', 0xA0, 0, 88, 1),

    'RX4_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 89, 1),
    'RX3_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 90, 1),
    'RX2_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 91, 1),
    'RX1_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 92, 1),

    'HIGH_POWER_CLASS_ENABLE': (1, 'get_bits', 0xA0, 0, 93, 1, 2, 1),
    'POWER_SET':        (1, 'get_bits', 0xA0, 0, 93, 1, 1, 1),
    'POWER_OVERRIDE':   (1, 'get_bits', 0xA0, 0, 93, 1, 0, 1),

    'TX4_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 94, 1),
    'TX3_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 95, 1),
    'TX2_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 96, 1),
    'TX1_APPLICATION_SELECT': (1, 'get_bytes', 0xA0, 0, 97, 1),

    'TX_RX_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 7, 8),  # 8 CDR bits
    'TX4_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 7, 1),
    'TX3_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 6, 1),
    'TX2_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 5, 1),
    'TX1_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 4, 1),
    'RX4_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 3, 1),
    'RX3_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 2, 1),
    'RX2_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 1, 1),
    'RX1_CDR_CONTROL': (1, 'get_bits', 0xA0, 0, 98, 1, 0, 1),


    # Free Side Device and Channel Masks (100-104)
    'M_TX_RX_LOS':       (1, 'get_bytes', 0xA0, 0, 100, 1),  # all 8 LOS bits
    'M_TX_ADAPT_EQ_FAULT':  (1, 'get_bits', 0xA0, 0, 101, 1, 7, 4),  # all 4
    'M_TX_FAULT':        (1, 'get_bits', 0xA0, 0, 101, 1, 3, 4),  # all 4 FAULT
    'M_TX_RX_CDR_LOL':   (1, 'get_bytes', 0xA0, 0, 102, 1),  # all 8 LOS bits
    'M_TEMP_ALARM_WARN': (1, 'get_bits', 0xA0, 0, 103, 1, 7, 4),
    'M_VCC_ALARM_WARN':  (1, 'get_bits', 0xA0, 0, 104, 1, 7, 4),
    'VENDOR_SPECIFIC_105': (0, 'get_bytes', 0xA0, 0, 105, 2),

    # Free Side Device Properties
    'PROPAGATION_DELAY': (0, 'get_intX10', 0xA0, 0, 108, 2),
    'ADVANCED_LOW_POWER_MODE': (0, 'get_bits', 0xA0, 0, 110, 1, 7, 4),
    'FAR_SIDE_MANAGED': (0, 'get3_bit2', 0xA0, 0, 110, 1),  # Gary Larson?
    'FAR_END_IMPLEMENT': (0, 'get3_bit6', 0xA0, 0, 113, 1),
    'NEAR_END_IMPLEMENT': (0, 'get_bits', 0xA0, 0, 113, 1, 3, 4),


    # Passwords (note, these are write-only, they can't be read!)
    # These keys are here to enable their write side counterparts
    'PASSWORD_CHANGE':     (0, 'get_int', 0xA0, 0, 119, 4),
    'PASSWORD_ENTRY':      (0, 'get_int', 0xA0, 0, 123, 4),


    # Page 0, Serial ID fields
    # Note, per the spec: Page 00h Byte 0 and Page 00h Byte 128 shall
    # contain the same parameter values.
    # Use this one, because it is on a page of static (cachable) data
    'IDENTIFIER':       (0, 'get_int', 0xA0, 0, 128, 1),
    'EXT_IDENTIFIER':   (0, 'get_int', 0xA0, 0, 129, 1),
    'CONNECTOR':        (0, 'get_int', 0xA0, 0, 130, 1),
    'SPEC_COMPLIANCE':  (0, 'get_bytes', 0xA0, 0, 131, 8),    # see table 33
    'ENCODING':         (0, 'get_int', 0xA0, 0, 139, 1),
    'BR_NOMINAL':       (0, 'get_bitrate', 0xA0, 0, 140, 83),  # bytes 140, 222
    'EXT_RATE_COMPLY':  (0, 'get_bits', 0xA0, 0, 141, 1, 0, 1),

    'LENGTH_SMF_KM':    (0, 'get_length_km', 0xA0, 0, 142, 1),
    'LENGTH_OM3_50UM':  (0, 'get_length_2m', 0xA0, 0, 143, 1),
    'LENGTH_OM2_50UM':  (0, 'get_int', 0xA0, 0, 144, 1),
    'LENGTH_OM1_62_5UM':  (0, 'get_int', 0xA0, 0, 145, 1),
    'LENGTH_OM4_OR_CU':    (0, 'get_length_omcu2', 0xA0, 0, 146, 2),

    'DEVICE_TECH':      (0, 'get_bytes', 0xA0, 0, 147, 1),
    'VENDOR_NAME':      (0, 'get_string', 0xA0, 0, 148, 16),
    'EXTENDED_MODULE':  (0, 'get_bytes', 0xA0, 0, 164, 1),
    'VENDOR_OUI':       (0, 'get_bytes', 0xA0, 0, 165, 3),
    'VENDOR_PN':        (0, 'get_string', 0xA0, 0, 168, 16),
    'VENDOR_REV':       (0, 'get_string', 0xA0, 0, 184, 2),

    'WAVELENGTH':     (0, 'get_wavelength2', 0xA0, 0, 147, 41),  # 1 field
    'CU_ATTENUATE_2_5': (0, 'get_CU_2_5', 0xA0, 0, 147, 41),     # 3 keys
    'CU_ATTENUATE_5_0': (0, 'get_CU_5_0', 0xA0, 0, 147, 41),     # this is 3
    'WAVELEN_TOLERANCE': (0, 'get_wave_tol', 0xA0, 0, 188, 2),
    'MAX_CASE_TEMP':    (0, 'get_int', 0xA0, 0, 190, 1),

    # Page 0, Extended Serial ID fields
    'LINK_CODES':       (0, 'get_int', 0xA0, 0, 192, 1),
    'OPTIONS':          (0, 'get_bytes', 0xA0, 0, 193, 3),
    'VENDOR_SN':        (0, 'get_string', 0xA0, 0, 196, 16),
    'DATE_CODE':        (0, 'get_string', 0xA0, 0, 212, 8),
    'DIAG_MONITOR_TYPE': (0, 'get_int', 0xA0, 0, 220, 1),
    'ENHANCED_OPTIONS': (0, 'get_int', 0xA0, 0, 221, 1),
    # note, byte 222 is bit rate in units of 250Mb, see BR_NOMINAL key

    'VENDOR_SPECIFIC_224': (0, 'get_bytes', 0xA0, 0, 224, 32),

    # Thresholds for:
    # {temperature, voltage, laser bias, Tx_Power, Rx_Power}
    # {High, Low} {Alarm, Warning}
    # In other words, 5 values have high/low alarm/warning thresholds
    # Note - not channel specific, all channels have the same thresholds

    'TEMP_HIGH_ALARM':  (0, 'get_temperature', 0xA0, 3, 128, 2),
    'TEMP_LOW_ALARM':   (0, 'get_temperature', 0xA0, 3, 130, 2),
    'TEMP_HIGH_WARN':   (0, 'get_temperature', 0xA0, 3, 132, 2),
    'TEMP_LOW_WARN':    (0, 'get_temperature', 0xA0, 3, 134, 2),

    'VOLTAGE_HIGH_ALARM': (0, 'get_voltage', 0xA0, 3, 144, 2),
    'VOLTAGE_LOW_ALARM':  (0, 'get_voltage', 0xA0, 3, 146, 2),
    'VOLTAGE_HIGH_WARN':  (0, 'get_voltage', 0xA0, 3, 148, 2),
    'VOLTAGE_LOW_WARN':   (0, 'get_voltage', 0xA0, 3, 150, 2),

    'BIAS_HIGH_ALARM':  (0, 'get_current', 0xA0, 3, 184, 2),
    'BIAS_LOW_ALARM':   (0, 'get_current', 0xA0, 3, 186, 2),
    'BIAS_HIGH_WARN':   (0, 'get_current', 0xA0, 3, 188, 2),
    'BIAS_LOW_WARN':    (0, 'get_current', 0xA0, 3, 190, 2),

    'TX_POWER_HIGH_ALARM': (0, 'get_power_dbm', 0xA0, 3, 192, 2),
    'TX_POWER_LOW_ALARM':  (0, 'get_power_dbm', 0xA0, 3, 194, 2),
    'TX_POWER_HIGH_WARN':  (0, 'get_power_dbm', 0xA0, 3, 196, 2),
    'TX_POWER_LOW_WARN':   (0, 'get_power_dbm', 0xA0, 3, 198, 2),

    'RX_POWER_HIGH_ALARM': (0, 'get_power_dbm', 0xA0, 3, 176, 2),
    'RX_POWER_LOW_ALARM':  (0, 'get_power_dbm', 0xA0, 3, 178, 2),
    'RX_POWER_HIGH_WARN':  (0, 'get_power_dbm', 0xA0, 3, 180, 2),
    'RX_POWER_LOW_WARN':   (0, 'get_power_dbm', 0xA0, 3, 182, 2),
    }


new_fm_keys = {
    'SERIAL_ID': ('IDENTIFIER',
                  'EXT_IDENTIFIER',
                  'CONNECTOR',
                  'SPEC_COMPLIANCE',
                  'ENCODING',
                  'BR_NOMINAL',
                  'EXT_RATE_COMPLY',
                  'LENGTH_SMF_KM',
                  'LENGTH_OM3_50UM',
                  'LENGTH_OM2_50UM',
                  'LENGTH_OM1_62_5UM',
                  'LENGTH_OM4_OR_CU',
                  'DEVICE_TECH',
                  'VENDOR_NAME',
                  'EXTENDED_MODULE',
                  'VENDOR_OUI',
                  'VENDOR_PN',
                  'VENDOR_REV',
                  'WAVELENGTH',
                  'CU_ATTENUATE_2_5',
                  'CU_ATTENUATE_5_0',
                  'WAVELEN_TOLERANCE',
                  'MAX_CASE_TEMP'
                  ),

    'SERIAL_ID_EXT':    ('LINK_CODES',
                         'OPTIONS',
                         'VENDOR_SN',
                         'DATE_CODE',
                         'DIAG_MONITOR_TYPE',
                         'ENHANCED_OPTIONS'
                         ),

    'DOM':      ('TEMPERATURE',
                 'SUPPLY_VOLTAGE',
                 'TX1_BIAS',
                 'TX2_BIAS',
                 'TX3_BIAS',
                 'TX4_BIAS',
                 'TX1_POWER',
                 'TX2_POWER',
                 'TX3_POWER',
                 'TX4_POWER',
                 'RX1_POWER',
                 'RX2_POWER',
                 'RX3_POWER',
                 'RX4_POWER',
                 )
    }


new_wm_keys = {
        'TX4_DISABLE': 'set_bits',
        'TX3_DISABLE': 'set_bits',
        'TX2_DISABLE': 'set_bits',
        'TX1_DISABLE': 'set_bits',
        'PASSWORD_CHANGE': 'set_int',
        'PASSWORD_ENTRY': 'set_int',
       }


# add these keys to the memory map, function map, write map for
# QSFP+ and QSFP28 devices
def add_keys(port):
    QSFP_PLUS = 0xD
    QSFP28 = 0x11
    if (port.port_type != QSFP_PLUS) and (port.port_type != QSFP28):
        return
    port.mmap.update(new_mm_keys)
    port.fmap.update(new_fm_keys)
    port.wmap.update(new_wm_keys)

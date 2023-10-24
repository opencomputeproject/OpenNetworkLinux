"""
addonsample.py - demonstrate how to add keys to OOM

This file (or your own addon file) must be placed in the
'<oom directory>/oom/oom/keyfiles'directory where the OOM files
have been loaded on the system.  Then OOM must be reinstalled
(cd <oom directory>/oom; python setup.py install).  This will
place your addon file in the keyfiles directory in the OOM site package.

When OOM is started (when oomlib.py is initialized), the list of files
matching './keyfiles/*.py' and './keyfiles/*.pyc' is created (if both
suffixes exist for the same module, the import library picks one)
The 'add_keys' function from each module is added to a list.

Whenever oom_get_portlist() is called, OOM builds a class of type Port
for each port in the switch.  As part of building each Port, OOM
will call each of the 'add_keys' functions in the list, as
add_keys(port).  The port will be initialized, including its
port_type.  The port is readable, so add_keys() can read any keys
or directly read any EEPROM location to determine whether the
specific keys being added would apply to that module.

At the least, add_keys() should verify that the port_type
is appropriate for the keys being added.  Other likely
screens might include keys like VENDOR_NAME, VENDOR_PN, VENDOR_REV,
REV_COMPLIANCE, etc.

There is no architectural limit on the number of addon files that
can be added to the keyfiles directory.
Multiple addons can be applied to each port.
Different addons can be applied to different ports.
Each addon decides whether to apply itself to each port, based
on any characteristic it can determine about that module, and it
can read all the keys, and all accessible EEPROM to decide.

NOTE:  A possible future use of addons is to provide different
or added keys to match different spec versions with different
modules.  The base OOM keys might match 'rev 3' of 'the spec'.  An
addon could add additional keys defined in 'rev 4'.  That addon could
check the REV_COMPLIANCE key of the module to decide whether or
not to add the additional keys.  This would allow one release of
OOM to support modules with different spec revs, even in the
same switch at the same time.

In this sample file we will add two keys to a QSFP+ module.
   COOLED_TRANSMITTER is one bit, indicating whether the transmitter
   is cooled (=1) or uncooled (=0)
   TUNABLE_TRANSMITTER is one bit, indicating whether the transmitter
   is tunable (=1) or not tunable (=0)
These bits are described in the Table 6-19 of SFF-8636, rev 2.6.  They
are available from OOM in the DEVICE_TECH key, but are not decoded.
They are in the EEPROM at address 0Ah, page 0, byte 147, bits 2 and 0.
This keyfile will add these keys to OOM.

We will also add a new function TRANSMITTER_TECH that returns both keys

We will also add write keys to modify our new keys.  These write keys
will actually fail because that is not a writable field in the EEPROM,
but it will demonstrate the method of adding a write key.

To activate this file (add these keys to QSFP+ modules), simply copy
the file into the keyfiles directory: 'cp addonsample.py ./keyfiles/'
Then re-install OOM: 'cd ..; python setup.py install'
The additional keys will be available to all subsequent users of OOM
To deactivate, remove the file: 'rm ./keyfiles/addonsample.py', and
re-install: 'cd ..; python setup.py install'

See the files already in the keyfiles directory for a rich list of
keys and their extractor functions.
See decode.py for the exhaustive list of extractor functions in OOM
Note: The implementation requires that extractor functions reside in
decode.py.
"""
# Create dictionaries of new keys to be inserted as necessary
# into port.mmap, port.fmap, port.wmap
#
# mmap keys begin with the user-visible key name, followed by key attributes:
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
#
# Note that writable keys take their location from port.mmap
# so there can be no keys in new_wmap_keys that are not also
# in new_mmap_keys
#

new_mmap_keys = {
    'COOLED_TRANSMITTER':  (0, 'get_bits', 0xA0, 0, 147, 1, 2, 1),
    'TUNABLE_TRANSMITTER': (0, 'get_bits', 0xA0, 0, 147, 1, 0, 1),
    }

# A new function key to group these together
# call as ttech = oom_get_memory(port, 'TRANSMITTER_TECH')
new_fmap_keys = {
    'TRANSMITTER_TECH': ('COOLED_TRANSMITTER',
                         'TUNABLE_TRANSMITTER',
                         )
                }


# the writable keys to match
# remember, writable keys use the mmap key value for the location
# to write the data to.  Every wmap key requires a matching mmap key.
# note, this location in QSFP+ is not actually writable, so actually
# trying to use these keys
#    - eg oom_set_keyvalue(port, 'COOLED_TRANSMITTER', 1)
# would fail, probably silently
new_wmap_keys = {
    'COOLED_TRANSMITTER': 'set_bits',
    'TUNABLE_TRANSMITTER': 'set_bits'
    }


# add the new keys to mmap, fmap, wmap for QSFP+ ports
# note 'add_keys(port)' is an architected API, you can't change
# the name and expect to get called.
# You CAN add additional code here, and additional functions.  Call
# them from add_keys() to get executed once for each port, each
# time oom_get_portlist() is invoked.
def add_keys(port):
    QSFP_PLUS = 0xD

    # only add these keys to QSFP+ ports (add other filters here)
    if port.port_type != QSFP_PLUS:
        return
    port.mmap.update(new_mmap_keys)
    port.fmap.update(new_fmap_keys)
    port.wmap.update(new_wmap_keys)

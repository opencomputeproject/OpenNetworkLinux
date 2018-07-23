"""aim.py

Roll-up of structs in AIM that we need.
"""

import ctypes

from AIM import aim_weakref

libonlp = None
libc = None

# AIM/aim_memory.h

class aim_void_p(aim_weakref.AimPointer):

    @classmethod
    def deletePointer(cls, aimPtr):
        libonlp.aim_free(aimPtr)

class aim_char_p(aim_void_p):
    """AIM data that is a printable string."""

    def __init__(self, stringOrAddress):

        if stringOrAddress is None:
            aim_void_p.__init__(self, stringOrAddress)
            return

        if isinstance(stringOrAddress, aim_void_p):
            aim_void_p.__init__(self, stringOrAddress)
            return

        if isinstance(stringOrAddress, basestring):
            cs = ctypes.c_char_p(stringOrAddress)
            ptr = libonlp.aim_malloc(len(stringOrAddress)+1)
            libc.strcpy(ptr, ctypes.addressof(cs))
            aim_void_p.__init__(self, ptr)
            return

        if type(stringOrAddress) == int:
            aim_void_p.__init__(self, stringOrAddress)
            return

        raise ValueError("invalid initializer for aim_char_p: %s"
                         % repr(stringOrAddress))

    def string_at(self):
        if self.value:
            return ctypes.string_at(self.value)
        else:
            return None

    def __eq__(self, other):
        return (isinstance(other, aim_char_p)
                and (self.string_at()==other.string_at()))

    def __neq__(self, other):
        return not self == other

    def __hash__(self):
        return hash(self.string_at())

def aim_memory_init_prototypes(dll):

    dll.aim_malloc.restype = aim_void_p
    dll.aim_malloc.argtypes = (ctypes.c_size_t,)

    dll.aim_free.restype = None
    dll.aim_free.argtypes = (aim_void_p,)

# AIM/aim_object.h

aim_object_dtor = ctypes.CFUNCTYPE(None, ctypes.c_void_p)

class aim_object(ctypes.Structure):
    _fields_ = [("_id", ctypes.c_char_p,),
                ("subtype", ctypes.c_int,),
                ("cookie", ctypes.c_void_p,),
                ("destructor", aim_object_dtor,),]

# AIM/aim_pvs.h
# AIM/aim_pvs_*.h

aim_vprintf_f = ctypes.CFUNCTYPE(ctypes.c_int)

class aim_pvs(ctypes.Structure):
    _fields_ = [("object", aim_object,),
                ("description", ctypes.c_char_p,),
                ("vprintf", aim_vprintf_f,),
                ("enabled", ctypes.c_int,),
                ("counter", ctypes.c_uint32,),
                ("isatty", aim_vprintf_f,),]

def aim_pvs_init_prototypes(dll):

    dll.aim_pvs_buffer_create.restype = ctypes.POINTER(aim_pvs)

    dll.aim_pvs_destroy.restype = None
    dll.aim_pvs_destroy.argtypes = (ctypes.POINTER(aim_pvs),)

    dll.aim_pvs_buffer_size.restype = ctypes.c_int
    dll.aim_pvs_buffer_size.argtypes = (ctypes.POINTER(aim_pvs),)

    dll.aim_pvs_buffer_get.restype = aim_char_p
    dll.aim_pvs_buffer_get.argtypes = (ctypes.POINTER(aim_pvs),)

    dll.aim_pvs_buffer_reset.restype = None
    dll.aim_pvs_buffer_reset.argtypes = (ctypes.POINTER(aim_pvs),)

# AIM/aim_bitmap.h

aim_bitmap_word = ctypes.c_uint32
AIM_BITMAP_BITS_PER_WORD = 32

def AIM_BITMAP_WORD_COUNT(bitcount):
    return ((bitcount // AIM_BITMAP_BITS_PER_WORD)
            + (1 if (bitcount % AIM_BITMAP_BITS_PER_WORD) else 0))

# ugh, most of aim_bitmap.h is inline C

def AIM_BITMAP_HDR_WORD_GET(hdr, word):
    """Return a specific ctypes word."""
    return hdr.words[word]

def AIM_BITMAP_HDR_BIT_WORD_GET(hdr, bit):
    """Return the ctypes word holding this bit."""
    return hdr.words[bit/AIM_BITMAP_BITS_PER_WORD]

def AIM_BITMAP_HDR_BIT_WORD_SET(hdr, bit, word):
    """Return the ctypes word holding this bit."""
    hdr.words[bit/AIM_BITMAP_BITS_PER_WORD] = word

def AIM_BITMAP_BIT_POS(bit):
    return (1<<(bit % AIM_BITMAP_BITS_PER_WORD))

def AIM_BITMAP_INIT(bitmap, count):
    """Initialize a static bitmap."""
    libc.memset(ctypes.byref(bitmap), 0, ctypes.sizeof(bitmap))
    bitmap.hdr.maxbit = count
    bitmap.hdr.words = ctypes.cast(ctypes.byref(bitmap.words), ctypes.POINTER(ctypes.c_uint))
    bitmap.hdr.wordcount = AIM_BITMAP_WORD_COUNT(count)

class aim_bitmap_hdr(ctypes.Structure):
    _fields_ = [("wordcount", ctypes.c_int,),
                ("words", ctypes.POINTER(aim_bitmap_word),),
                ("maxbit", ctypes.c_int,),
                ("allocated", ctypes.c_int,),]

class aim_bitmap(ctypes.Structure):
    _fields_ = [("hdr", aim_bitmap_hdr,),]

class aim_bitmap_ref(aim_weakref.AimReference):
    """Dynamically allocated aim_bitmap."""

    _fields_ = aim_bitmap._fields_

    def __init__(self, bitcount):
        ptr = libonlp.aim_bitmap_alloc(None, bitcount)
        super(aim_bitmap_ref, self).__init__(ptr)

    @classmethod
    def deleteReference(self, aimPtr):
        libonlp.aim_free(aimPtr)

class aim_bitmap256(aim_bitmap):
    """Statically-allocated AIM bitmap."""
    _fields_ = [("words", aim_bitmap_word * AIM_BITMAP_WORD_COUNT(256),),]

    def __init__(self):
        super(aim_bitmap256, self).__init__()
        AIM_BITMAP_INIT(self, 255)

def aim_bitmap_set(hdr, bit):
    word = AIM_BITMAP_HDR_BIT_WORD_GET(hdr, bit)
    word |= AIM_BITMAP_BIT_POS(bit)
    AIM_BITMAP_HDR_BIT_WORD_SET(hdr, bit, word)

def aim_bitmap_clr(hdr, bit):
    word = AIM_BITMAP_HDR_BIT_WORD_GET(hdr, bit)
    word &= ~(AIM_BITMAP_BIT_POS(bit))
    AIM_BITMAP_HDR_BIT_WORD_SET(hdr, bit, word)

def aim_bitmap_mod(hdr, bit, value):
    if value:
        aim_bitmap_set(hdr, bit)
    else:
        aim_bitmap_clr(hdr, bit)

def aim_bitmap_get(hdr, bit):
    val = AIM_BITMAP_HDR_BIT_WORD_GET(hdr,bit) & AIM_BITMAP_BIT_POS(bit)
    return 1 if val else 0

# Huh, these is inline too, but calls into glibc memset

def aim_bitmap_set_all(hdr):
    libc.memset(ctypes.byref(hdr.words), 0xFF, hdr.wordcount*ctypes.sizeof(aim_bitmap_word))

def aim_bitmap_clr_all(hdr):
    libc.memset(ctypes.byref(hdr.words), 0x00, hdr.wordcount*ctypes.sizeof(aim_bitmap_word))

# XXX aim_bitmap_count is left out

def aim_bitmap_init_prototypes(dll):

    dll.aim_bitmap_alloc.restype = ctypes.POINTER(aim_bitmap)
    dll.aim_bitmap_alloc.argtypes = (ctypes.POINTER(aim_bitmap), ctypes.c_int,)

    dll.aim_bitmap_free.restype = None
    dll.aim_bitmap_free.argtypes = (ctypes.POINTER(aim_bitmap),)


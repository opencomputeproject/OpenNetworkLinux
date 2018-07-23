"""cjson_util.py

Bindings for a subset of bigcode's cjson_util.
"""

import ctypes
import json

libonlp = None
libjson_c = None

from AIM import aim, aim_weakref

class cJSON(ctypes.Structure):
    """Opaque structure for cJSON data."""

    @property
    def data(self):
        """Coerce into Python data."""
        cjStr = libonlp.cjson_util_json_str(ctypes.byref(self))
        cjBuf = ctypes.cast(cjStr, ctypes.c_char_p).value
        cjData = json.loads(cjBuf)
        libonlp.aim_free(cjStr)
        return cjData

class cJSONHandle(aim_weakref.AimPointer):
    """Maintain a handle to a cJSON object allocated from memory."""

    def __init__(self, aimPtr=None):
        super(cJSONHandle, self).__init__(aimPtr)

    @classmethod
    def deletePointer(cls, aimPtr):
        """Override this with the proper delete semantics."""
        if aimPtr is not None and aimPtr != 0:
            libonlp.cJSON_Delete(aimPtr)

    @property
    def hnd(self):
        hnd = ctypes.byref(self)
        return ctypes.cast(hnd, ctypes.POINTER(ctypes.POINTER(cJSON)))

    @property
    def contents(self):
        if self.value is None or self.value == 0:
            raise ValueError("NULL pointer")
        ptr = ctypes.cast(self.value, ctypes.POINTER(cJSON))
        return ptr.contents

def cjson_util_init_prototypes(dll, jsondll):

    dll.cJSON_Delete.argtypes = (ctypes.POINTER(cJSON),)

    dll.cjson_util_yaml_pvs.restype = ctypes.c_int
    dll.cjson_util_yaml_pvs.argtypes = (ctypes.POINTER(aim.aim_pvs), ctypes.POINTER(cJSON),)

    dll.cjson_util_yaml_str.restype = ctypes.POINTER(ctypes.c_char)
    dll.cjson_util_yaml_str.argtypes = (ctypes.POINTER(cJSON),)

    dll.cjson_util_json_pvs.restype = ctypes.c_int
    dll.cjson_util_json_pvs.argtypes = (ctypes.POINTER(aim.aim_pvs), ctypes.POINTER(cJSON),)

    dll.cjson_util_json_str.restype = ctypes.POINTER(ctypes.c_char)
    dll.cjson_util_json_str.argtypes = (ctypes.POINTER(cJSON),)

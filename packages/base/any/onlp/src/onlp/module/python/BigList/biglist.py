"""biglist.py

Subset of BigList/biglist.h.
"""

import logging
import ctypes

libonlp = None

# BigList/biglist.h (see BigCode .. BigData .. BigList)

biglist_iter_f = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p)
biglist_free_f = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p)

class BigListVisitor(object):

    def __init__(self, log=None):
        self.log = log or logging.getLogger("visit")

    def visit(self, data, cookie):
        return ONLP_STATUS.OK

    def cvisit(self):
        def _v(data, cookie):
            try:
                return self.visit(data, cookie)
            except:
                self.log.exception("visitor failed")
                return ONLP_STATUS.E_GENERIC
        return biglist_iter_f(_v)

    def foreach(self, blp, cookie):
        return libonlp.biglist_foreach(blp, self.cvisit(), cookie)

class BigListIterator(object):

    data_klass = None

    def __init__(self, blp, castType=None):
        self.blp = blp
        self.castType = castType or self.data_klass

    def next(self):

        # Hurr, pointer()/POINTER() types are not directly comparable
        p = ctypes.cast(self.blp, ctypes.c_void_p)
        if p.value is None:
            raise StopIteration

        blp, self.blp = self.blp, libonlp.biglist_next(self.blp)
        cur = blp.contents.data
        if self.castType is not None:
            cur = ctypes.cast(cur, ctypes.POINTER(self.castType))
            return cur.contents
        else:
            return cur

    def __iter__(self):
        return self

class biglist(ctypes.Structure):
    pass

biglist._fields_ = [("data", ctypes.c_void_p,),
                    ("next", ctypes.POINTER(biglist),),
                    ("previous", ctypes.POINTER(biglist),),]

biglist_handle = ctypes.POINTER(ctypes.POINTER(biglist))

def biglist_init_prototypes(dll):

    ##dll.biglist_alloc.restype = ctypes.POINTER(biglist)
    ##dll.biglist_alloc.argtypes = (ctypes.c_void_p, ctypes.POINTER(biglist), ctypes.POINTER(biglist),)

    dll.biglist_next.restype = ctypes.POINTER(biglist)
    dll.biglist_next.argtypes = (ctypes.POINTER(biglist),)

    dll.biglist_foreach.restype = ctypes.c_int
    dll.biglist_foreach.argtypes = (ctypes.POINTER(biglist), biglist_iter_f, ctypes.c_void_p,)

    dll.biglist_length.restype = ctypes.c_int
    dll.biglist_length.argtypes = (ctypes.POINTER(biglist),)

    ##dll.biglist_free_all.restype = ctypes.c_int
    ##dll.biglist_free_all.argtypes = (ctypes.POINTER(biglist), biglist_free_f,)

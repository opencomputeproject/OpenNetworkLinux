"""aim_weakref.py

Use weakref to implement smart AIM pointers.

See e.g.
http://code.activestate.com/recipes/577242-calling-c-level-finalizers-without-__del__/
"""

import ctypes
import logging
import weakref

logger = logging.getLogger("weakref")

def getLogger():
    global logger
    return logger

class AimOwnerRef(weakref.ref):
    pass

def _run_finalizer(ref):
    """Internal weakref callback to run finalizers"""
    del _finalize_refs[id(ref)]
    finalizer = ref.finalizer
    item = ref.item
    try:
        getLogger().info("finalizing object at %s", item)
        finalizer(item)
    except Exception:
        getLogger().exception("finalizer failed")

_finalize_refs = {}

def track_for_finalization(owner, item, finalizer):
    """Register an object for finalization.

    ``owner`` is the the object which is responsible for ``item``.
    ``finalizer`` will be called with ``item`` as its only argument when
    ``owner`` is destroyed by the garbage collector.
    """
    getLogger().info("tracking object at %s", item)
    ref = AimOwnerRef(owner, _run_finalizer)
    ref.item = item
    ref.finalizer = finalizer
    _finalize_refs[id(ref)] = ref

class AimReference(object):
    """Manage an AIM pointer using reference semantics."""

    @classmethod
    def deleteReference(cls, aimPtr):
        """Override this with the proper delete semantics."""
        raise NotImplementedError

    def __init__(self, aimPtr):
        self.ptr = aimPtr
        track_for_finalization(self, self.ptr, self.deleteReference)

    def __getattr__(self, attr, dfl='__none__'):
        if dfl == '__none__':
            return getattr(self.ptr.contents, attr)
        else:
            return getattr(self.ptr.contents, attr, dfl)

    def __setattr___(self, attr, val):
        setattr(self.ptr.contents, attr, val)

class AimPointer(ctypes.c_void_p):
    """Manage an AIM pointer using pointer semantics."""

    @classmethod
    def deletePointer(cls, aimPtr):
        """Override this with the proper delete semantics."""
        raise NotImplementedError

    def __init__(self, aimPtr):

        super(ctypes.c_void_p, self).__init__(aimPtr)
        # XXX roth -- casting may be necessary

        track_for_finalization(self, aimPtr, self.deletePointer)

"""OnlpApiTest.py

Test the API bindings.
"""

import ctypes
import unittest
import logging
import re
import time
import subprocess
import random
import tempfile
import os
import json

import onlp.onlp
import onlp.onlplib
import sff.sff

libonlp = onlp.onlp.libonlp

from AIM import aim, aim_weakref
from BigList import biglist
from cjson_util import cjson_util

def isVirtual():
    with open("/etc/onl/platform") as fd:
        buf = fd.read()
    return "bigswitch" in buf

def skipIfVirtual(reason="this test only works with a physical device"):
    return unittest.skipIf(isVirtual(), reason)

class aim_pvs_buffer(aim_weakref.AimReference):

    def __init__(self):
        ptr = libonlp.aim_pvs_buffer_create()
        super(aim_pvs_buffer, self).__init__(ptr)

    @classmethod
    def deleteReference(self, ptr):
        libonlp.aim_pvs_destroy(ptr)

    def string_at(self):
        buf = libonlp.aim_pvs_buffer_get(self.ptr)
        return buf.string_at()

class WithoutOnlpd(object):

    def __init__(self, log=None):
        self.log = log or logging.getLogger("onlpd")

    def __enter__(self):
        subprocess.check_call(('service', 'onlpd', 'stop',))
        return self

    def __exit__(self, exc_type, exc_value, tb):
        try:
            subprocess.check_call(('service', 'onlpd', 'start',))
        except subprocess.CalledProcessError as ex:
            raise AssertionError("cannot start onlpd")
        return False

class OnlpTestMixin(object):

    def setUp(self):

        self.log = logging.getLogger("onlp")
        self.log.setLevel(logging.DEBUG)

        self.aim_pvs_buffer = aim_pvs_buffer()

        aim_weakref.logger = self.log.getChild("weakref")

    def tearDown(self):
        pass

    def assertStatusOK(self, sts):
        if sts == onlp.onlp.ONLP_STATUS.OK:
            return
        raise AssertionError("invalid ONLP status %s" % onlp.onlp.ONLP_STATUS.name(sts))

    def withoutOnlpd(self):
        return WithoutOnlpd(log=self.log)

def flags2str(cls, val):
    pos = 1
    vals = []
    while val:
        if val & 0x1:
            pval = cls.name(pos)
            if pval is None:
                raise ValueError("invalid enum for %s: %s"
                                 % (str(cls), pos,))
            vals.append(cls.name(pos))
        val >>= 1
        pos <<= 1
    if vals:
        return ",".join(vals)
    else:
        return ""

class InitTest(OnlpTestMixin,
               unittest.TestCase):

    def setUp(self):
        OnlpTestMixin.setUp(self)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testInit(self):
        """Verify that the library can be loaded."""
        pass

    def testBuffer(self):
        """Verify that the AIM buffer type is usable."""

        self.assertIsNone(self.aim_pvs_buffer.string_at())

        libonlp.aim_printf(self.aim_pvs_buffer.ptr, "hello\n")
        self.assertEqual("hello\n", self.aim_pvs_buffer.string_at())

        libonlp.aim_printf(self.aim_pvs_buffer.ptr, "world\n")
        self.assertEqual("hello\nworld\n", self.aim_pvs_buffer.string_at())

        libonlp.aim_printf(self.aim_pvs_buffer.ptr, "%d\n", 42)
        self.assertEqual("hello\nworld\n42\n", self.aim_pvs_buffer.string_at())

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer.ptr)
        self.assertIsNone(self.aim_pvs_buffer.string_at())

class OnlpTest(OnlpTestMixin,
               unittest.TestCase):
    """Test interfaces in onlp/onlp.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_sw_init(None)
        libonlp.onlp_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    @unittest.skip("XXX roth -- missing implementation of onlp_platform_dump")
    def testPlatformDump(self):
        """Verify basic platform dump output."""

        flags = 0
        libonlp.onlp_platform_dump(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("System Information:", bufStr)
        self.assertIn("thermal @ 1", bufStr)

    @unittest.skip("XXX roth -- missing implementation of onlp_platform_dump")
    def testPlatformDumpFlags(self):
        """Verify platform dump flags are honored."""

        flags = 0
        libonlp.onlp_platform_dump(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("psu @ 1", bufStr)
        self.assertNotIn("PSU-1 Fan", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer.ptr)

        flags = onlp.onlp.ONLP_OID_DUMP.RECURSE
        libonlp.onlp_platform_dump(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("psu @ 1", bufStr)
        self.assertIn("PSU-1 Fan", bufStr)

        # hard to test onlp.onlp.ONLP_OID_DUMP.RECURSE,
        # since it depends on whether a specific component is inserted

    @unittest.skip("XXX roth -- missing implementation of onlp_platform_show")
    def testPlatformShow(self):
        """Verify basic platform show output."""

        flags = 0
        libonlp.onlp_platform_show(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("System Information:", bufStr)

    @unittest.skip("XXX roth -- missing implementation of onlp_platform_show")
    def testPlatformShowFlags(self):
        """Verify that onlp_platform_show honors flags."""

        flags = 0
        libonlp.onlp_platform_show(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertNotIn("PSU 1", bufStr)
        self.assertNotIn("PSU-1 Fan", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer.ptr)

        flags = onlp.onlp.ONLP_OID_SHOW.RECURSE
        libonlp.onlp_platform_show(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("PSU 1", bufStr)
        self.assertIn("PSU-1 Fan", bufStr)

class OidHdrMixin(object):

    def auditOidHdr(self, hdr):

        self.assertEqual(0, hdr.poid)
        if hdr._id == onlp.onlp.ONLP_OID_CHASSIS:
            pass
        ##elif hdr._id == 0:
        ##    self.log.warn("invalid system OID 0")
        else:
            raise AssertionError("invalid system OID")
        # root OID

        coids = [x for x in hdr.children()]
        self.assert_(coids)
        self.assertLess(len(coids), onlp.onlp.ONLP_OID_TABLE_SIZE)

        def _oidType(oid):
            return onlp.onlp.ONLP_OID_TYPE.name(oid._id>>24)

        for coid in coids:
            self.log.info("oid %d (%s): %s",
                          coid._id, _oidType(coid), coid.description)
            if _oidType(coid) is None:
                raise AssertionError("invalid oid %d" % coid._id)
            if hdr._id != coid.poid:
                self.log.warn("XXX roth -- invalid parent/child oid: %d, %d",
                              hdr._id, coid.poid)
                ##raise AssertionError("invalid parent/child oid: %d, %d"
                ##                     % (hdr._id, coid.poid,))

            # if it's a PSU, verify that it has fans
            if _oidType(coid) == "PSU":
                foids = [x for x in coid.children()]
                for foid in foids:
                    self.log.info("oid %d (%s): %s",
                                  foid._id, _oidType(foid), foid.description)
                    if _oidType(foid) in ["FAN", "THERMAL",]:
                        pass
                    else:
                        raise AssertionError("invalid child oid")
                    # parent should the the PSU or the chassis
                    if coid._id == foid.poid:
                        pass
                    elif hdr._id == foid.poid:
                        pass
                    else:
                        self.log.warn("XXX roth -- invalid parent oid: %d",
                                      foid.poid)
                        ##raise AssertionError("invalid parent OID %d"
                        ##                     % (foid.poid,))

class OidIterator(object):

    def __init__(self, log):
        self.log = log or logging.getLogger("visit")

    def visit(self, oid, cookie):
        return onlp.onlp.ONLP_STATUS.OK

    def cvisit(self):
        def _v(oid, cookie):
            try:
                return self.visit(oid, cookie)
            except:
                self.log.exception("visitor failed")
                return onlp.onlp.ONLP_STATUS.E_GENERIC
        return onlp.onlp.onlp_oid_iterate_f(_v)

    def foreach(self, oid, oidTypeFlags, cookie):
        return libonlp.onlp_oid_iterate(oid, oidTypeFlags, self.cvisit(), cookie)

class OidTest(OnlpTestMixin,
              OidHdrMixin,
              unittest.TestCase):
    """Test interfaces in onlp/oids.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_sw_init(None)
        libonlp.onlp_hw_init(0)

        self.hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(onlp.onlp.ONLP_OID_CHASSIS, ctypes.byref(self.hdr))

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testChassisOid(self):
        """Audit the system oid."""
        self.auditOidHdr(self.hdr)

    def testOidIter(self):
        """Test the oid iteration functions."""

        class V1(OidIterator):

            def __init__(self, cookie, log):
                super(V1, self).__init__(log)
                self.cookie = cookie
                self.oids = []

            def visit(self, oid, cookie):
                if cookie != self.cookie:
                    raise AssertionError("invalid cookie")
                typ = oid >> 24
                typ = onlp.onlp.ONLP_OID_TYPE.name(typ).lower()
                idx = oid & 0xffffff
                self.log.info("found oid %s-%d", typ, idx)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        oidTypeFlags = 0
        cookie = 0xdeadbeef

        # gather all OIDs

        v1 = V1(cookie, log=self.log.getChild("v1"))
        v1.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, cookie)
        self.assert_(v1.oids)
        oids = list(v1.oids)

        # filter based on OID type

        oidTypeFlags = onlp.onlp.ONLP_OID_TYPE_FLAG.PSU
        v1b = V1(cookie, log=self.log.getChild("v1b"))
        v1b.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, cookie)
        self.assert_(v1b.oids)
        self.assertLess(len(v1b.oids), len(oids))

        oidTypeFlags = 0
        v2 = V1(cookie+1, log=self.log.getChild("v2"))
        v2.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, cookie)
        self.assertEqual([], v2.oids)

        # validate early exit

        class V3(OidIterator):

            def __init__(self, log):
                super(V3, self).__init__(log)
                self.oids = []

            def visit(self, oid, cookie):
                if oid == cookie:
                    return onlp.onlp.ONLP_STATUS.E_GENERIC
                self.log.info("found oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        v3 = V3(log=self.log.getChild("v3"))
        cookie = oids[4]
        v3.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, cookie)
        self.assertEqual(4, len(v3.oids))

    def testOidHdrGet(self):
        """Test the oid_hdr_get function."""

        oid = self.hdr.coids[0]
        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))

        self.assertIn(hdr.getType(),
                      (onlp.onlp.ONLP_OID_TYPE.THERMAL,
                       onlp.onlp.ONLP_OID_TYPE.FAN,
                       onlp.onlp.ONLP_OID_TYPE.PSU,))

    def testOidInfoGet(self):
        """Test the oid_info_get function."""

        oid = self.hdr.coids[0]
        ip = onlp.onlp.onlp_oid_info_ptr(0)
        libonlp.onlp_oid_info_get(oid, ip.hnd)

        self.assertEqual(ip.contents.hdr._id, oid)
        self.assertIn(ip.contents.hdr.getType(),
                      (onlp.onlp.ONLP_OID_TYPE.THERMAL,
                       onlp.onlp.ONLP_OID_TYPE.FAN,
                       onlp.onlp.ONLP_OID_TYPE.PSU,))

    def testOidToStr(self):
        """Test the onlp_oid_to_str function."""

        oid = self.hdr.coids[0]
        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))

        oidStr = (ctypes.c_char * 32)()
        # XXX roth

        oidPtr = ctypes.cast(oidStr, ctypes.POINTER(ctypes.c_char))
        sts = libonlp.onlp_oid_to_str(oid, oidPtr)
        self.assertStatusOK(sts)

        if hdr.getType() == onlp.onlp.ONLP_OID_TYPE.THERMAL:
            self.assertIn("thermal-", oidStr.value)
        elif hdr.getType() == onlp.onlp.ONLP_OID_TYPE.FAN:
            self.assertIn("fan-", oidStr.value)
        elif hdr.getType() == onlp.onlp.ONLP_OID_TYPE.PSU:
            self.assertIn("psu-", oidStr.value)
        else:
            raise AssertionError("unkown OID type")

    def testOidToUserStr(self):
        """Test the onlp_oid_to_user_str function."""

        oid = self.hdr.coids[0]
        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))

        oidStr = (ctypes.c_char * 32)()
        # XXX roth

        oidPtr = ctypes.cast(oidStr, ctypes.POINTER(ctypes.c_char))
        sts = libonlp.onlp_oid_to_user_str(oid, oidPtr)
        self.assertStatusOK(sts)

        if hdr.getType() == onlp.onlp.ONLP_OID_TYPE.THERMAL:
            self.assertIn("Thermal ", oidStr.value)
        elif hdr.getType() == onlp.onlp.ONLP_OID_TYPE.FAN:
            self.assertIn("Fan ", oidStr.value)
        elif hdr.getType() == onlp.onlp.ONLP_OID_TYPE.PSU:
            self.assertIn("PSU ", oidStr.value)
        else:
            raise AssertionError("unkown OID type")

    def testOidHdrToJson(self):

        oid = self.hdr.coids[0]
        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_hdr_to_json(ctypes.byref(hdr), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertEqual('chassis-1', cjData['poid'])

    def testOidHdrToJsonRecursive(self):
        """Test recursion support."""

        oid = onlp.onlp.ONLP_OID_CHASSIS
        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_hdr_to_json(ctypes.byref(hdr), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIsNone(cjData['poid'])
        self.assertEqual('chassis-1', cjData['id'])

        cjh = cjson_util.cJSONHandle()
        flags = onlp.onlp.ONLP_OID_JSON_FLAG.RECURSIVE
        sts = libonlp.onlp_oid_hdr_to_json(ctypes.byref(hdr), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData2 = cjh.contents.data

        self.assertEqual(cjData, cjData2)
        # XXX roth

    def testOidTableToJson(self):

        oid = onlp.onlp.ONLP_OID_CHASSIS
        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_table_to_json(hdr.coids, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('psu-1', cjData)

    def testOidInfoToJson(self):

        oid = self.hdr.coids[0]
        ip = onlp.onlp.onlp_oid_info_ptr(0)
        libonlp.onlp_oid_info_get(oid, ip.hnd)

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_info_to_json(ip.ptr, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertEqual('chassis-1', cjData['hdr']['poid'])
        typ = ip.contents.hdr.getType()
        if typ == onlp.onlp.ONLP_OID_TYPE.THERMAL:
            self.assertIn('mcelsius', cjData)
        elif typ == onlp.onlp.ONLP_OID_TYPE.FAN:
            self.assertIn('model', cjData)
        elif typ == onlp.onlp.ONLP_OID_TYPE.PSU:
            self.assertIn('model', cjData)

    def testOidInfoToJsonRecursive(self):

        oid = onlp.onlp.ONLP_OID_CHASSIS
        ip = onlp.onlp.onlp_oid_info_ptr(0)
        libonlp.onlp_oid_info_get(oid, ip.hnd)

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_info_to_json(ip.ptr, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertEqual('chassis-1', cjData['hdr']['id'])

        cjh = cjson_util.cJSONHandle()
        flags = onlp.onlp.ONLP_OID_JSON_FLAG.RECURSIVE
        sts = libonlp.onlp_oid_info_to_json(ip.ptr, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData2 = cjh.contents.data

        self.assertEqual(cjData, cjData2)
        # XXX roth

    def testOidToJson(self):

        oid = self.hdr.coids[0]

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertEqual('chassis-1', cjData['hdr']['poid'])

    def testOidToJsonRecursive(self):

        oid = onlp.onlp.ONLP_OID_CHASSIS

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIsNone(cjData['hdr']['poid'])
        self.assertEqual('chassis-1', cjData['hdr']['id'])
        self.assert_(cjData['hdr']['coids'])

        cjh = cjson_util.cJSONHandle()
        flags = onlp.onlp.ONLP_OID_JSON_FLAG.RECURSIVE

        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData2 = cjh.contents.data

        self.assertEqual(cjData, cjData2)
        # XXX roth

    def testOidToUserJson(self):

        oid = self.hdr.coids[0]

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_user_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('Status', cjData)

    def testOidHdrGetAll(self):

        types = (onlp.onlp.ONLP_OID_TYPE_FLAG.CHASSIS
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.MODULE
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.THERMAL
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.FAN
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.PSU
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.LED
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.SFP
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.GENERIC)
        oidAll = onlp.onlp.onlp_oid_info_all()
        libonlp.onlp_oid_hdr_get_all(onlp.onlp.ONLP_OID_CHASSIS, types, 0,
                                     oidAll.hnd)

        cnt = libonlp.biglist_length(oidAll.ptr)
        self.assertGreater(cnt, 0)

        # test manual iteration
        items = [x for x in biglist.BigListIterator(oidAll.ptr)]
        self.assertEqual(cnt, len(items))

        # test API iteration
        class V(biglist.BigListVisitor):
            def __init__(self, log=None):
                super(V, self).__init__(log=log)
                self.items = []
            def visit(self, data, cookie):
                self.items.append(data)
                return onlp.onlp.ONLP_STATUS.OK

        v = V()
        v.foreach(oidAll.ptr, 0)
        self.assertEqual(items, v.items)

        hdrp = ctypes.cast(items[0], ctypes.POINTER(onlp.onlp.onlp_oid_hdr))
        typ = hdrp.contents.getType()
        typ = onlp.onlp.ONLP_OID_TYPE.name(typ)
        if typ not in ('THERMAL', 'PSU', 'FAN',):
            raise AssertionError("invalid OID type %s" % typ)

        # make sure the presence test works
        ##sts = libonlp.onlp_oid_is_present(info._id)
        ##self.assertEqual(1, sts)
        ### XXX roth -- missing

    def testOidInfoGetAll(self):

        types = (onlp.onlp.ONLP_OID_TYPE_FLAG.CHASSIS
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.MODULE
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.THERMAL
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.FAN
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.PSU
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.LED
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.SFP
                 | onlp.onlp.ONLP_OID_TYPE_FLAG.GENERIC)
        oidAll = onlp.onlp.onlp_oid_info_all()
        libonlp.onlp_oid_info_get_all(onlp.onlp.ONLP_OID_CHASSIS, types, 0,
                                      oidAll.hnd)

        cnt = libonlp.biglist_length(oidAll.ptr)
        self.assertGreater(cnt, 0)

        # test manual iteration
        items = [x for x in biglist.BigListIterator(oidAll.ptr)]
        self.assertEqual(cnt, len(items))

        # test API iteration
        class V(biglist.BigListVisitor):
            def __init__(self, log=None):
                super(V, self).__init__(log=log)
                self.items = []
            def visit(self, data, cookie):
                self.items.append(data)
                return onlp.onlp.ONLP_STATUS.OK

        v = V()
        v.foreach(oidAll.ptr, 0)
        self.assertEqual(items, v.items)

        info = ctypes.cast(items[0], ctypes.POINTER(onlp.onlp.onlp_oid_info))
        typ = info.contents.hdr.getType()
        typ = onlp.onlp.ONLP_OID_TYPE.name(typ)
        if typ not in ('THERMAL', 'PSU', 'FAN',):
            raise AssertionError("invalid OID type %s" % typ)

class AttributeTest(OnlpTestMixin,
                    unittest.TestCase):
    """Test interfaces in onlp/attribute.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_attribute_sw_init()
        libonlp.onlp_attribute_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testAttributeSupported(self):

        attr = "not-an-attribute"
        sts = libonlp.onlp_attribute_supported(onlp.onlp.ONLP_OID_CHASSIS, attr)
        self.assertEqual(0, sts)

        attr = onlp.onlp.ONLP_ATTRIBUTE_ASSET_INFO_JSON
        sts = libonlp.onlp_attribute_supported(onlp.onlp.ONLP_OID_CHASSIS, attr)
        self.assertEqual(1, sts)

    def testAttributeGet(self):

        val = ctypes.c_void_p()

        attr = "not-an-attribute"
        sts = libonlp.onlp_attribute_get(onlp.onlp.ONLP_OID_CHASSIS, attr,
                                         ctypes.byref(val))
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, sts)

        attr = onlp.onlp.ONLP_ATTRIBUTE_ASSET_INFO_JSON
        sts = libonlp.onlp_attribute_get(onlp.onlp.ONLP_OID_CHASSIS, attr,
                                         ctypes.byref(val))
        self.assertEqual(onlp.onlp.ONLP_STATUS.OK, sts)
        self.assertIsNotNone(val.value)
        libonlp.onlp_attribute_free(onlp.onlp.ONLP_OID_CHASSIS, attr, val)

    def testAttributeSet(self):

        attr = "not-an-attribute"
        sts = libonlp.onlp_attribute_set(onlp.onlp.ONLP_OID_CHASSIS, attr, None)
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, sts)

        attr = onlp.onlp.ONLP_ATTRIBUTE_ASSET_INFO_JSON
        sts = libonlp.onlp_attribute_set(onlp.onlp.ONLP_OID_CHASSIS, attr, None)
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, sts)

class StdattrsTest(OnlpTestMixin,
                   unittest.TestCase):
    """Test interfaces in onlp/stdattrs.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_attribute_sw_init()
        libonlp.onlp_attribute_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testAssetInfo(self):

        attr = onlp.onlp.ONLP_ATTRIBUTE_ASSET_INFO

        cjh = onlp.onlp.AttributeHandle(attr, klass=onlp.onlp.onlp_asset_info)

        sts = libonlp.onlp_attribute_get(onlp.onlp.ONLP_OID_CHASSIS, attr,
                                         cjh.hnd)
        self.assertEqual(onlp.onlp.ONLP_STATUS.OK, sts)
        self.assertIsNotNone(cjh.value)

        self.assertEqual(onlp.onlp.ONLP_OID_CHASSIS, cjh.contents.oid)
        if cjh.contents.manufacturer is None:
            self.log.warn("XXX roth -- missing manufacturer")
            ##raise AssertionError("missing manufacturer")

    def testAssetInfoJson(self):

        attr = onlp.onlp.ONLP_ATTRIBUTE_ASSET_INFO_JSON

        cjh = onlp.onlp.AttributeHandle(attr, klass=cjson_util.cJSON)

        sts = libonlp.onlp_attribute_get(onlp.onlp.ONLP_OID_CHASSIS, attr,
                                         cjh.hnd)
        self.assertEqual(onlp.onlp.ONLP_STATUS.OK, sts)
        self.assertIsNotNone(cjh.value)

        cjData = cjh.contents.data

        self.assertIn('Firmware Revision', cjData)

    def testOnieInfo(self):

        attr = onlp.onlp.ONLP_ATTRIBUTE_ONIE_INFO

        cjh = onlp.onlp.AttributeHandle(attr, klass=onlp.onlplib.onlp_onie_info)

        sts = libonlp.onlp_attribute_get(onlp.onlp.ONLP_OID_CHASSIS, attr,
                                         cjh.hnd)
        self.assertEqual(onlp.onlp.ONLP_STATUS.OK, sts)
        self.assertIsNotNone(cjh.value)

        self.assertIn('2017', cjh.contents.onie_version)

    def testOnieInfoJson(self):

        attr = onlp.onlp.ONLP_ATTRIBUTE_ONIE_INFO_JSON

        cjh = onlp.onlp.AttributeHandle(attr, klass=cjson_util.cJSON)

        sts = libonlp.onlp_attribute_get(onlp.onlp.ONLP_OID_CHASSIS, attr,
                                         cjh.hnd)
        self.assertEqual(onlp.onlp.ONLP_STATUS.OK, sts)
        self.assertIsNotNone(cjh.value)

        cjData = cjh.contents.data

        self.assertIn('ONIE Version', cjData)

class PlatformTest(OnlpTestMixin,
                   unittest.TestCase):
    """Test interfaces in onlp/platform.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_platform_sw_init(None)
        libonlp.onlp_platform_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testPlatformName(self):

        name = libonlp.onlp_platform_name_get()
        self.log.info("platform name is %s", name)
        self.assert_(len(name))

    def testPlatformManage(self):

        libonlp.onlp_platform_manager_start(0)
        now = time.time()
        future = now + 5.0
        while True:
            now = time.time()
            if now > future: break
            self.log.info("manage...")
            time.sleep(0.5)
        libonlp.onlp_platform_manager_stop(1)

        ##onlp_platform_manager_join();
        ### XXX roth -- missing

class ModuleTest(OnlpTestMixin,
                 OidHdrMixin,
                 unittest.TestCase):
    """Test interfaces in onlp/module.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_module_sw_init()
        libonlp.onlp_module_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testModule(self):

        class V(OidIterator):

            def __init__(self, log=None):
                super(V, self).__init__(log=log)
                self.oids = []

            def visit(self, oid, cookie):
                self.log.info("found oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        oidType = onlp.onlp.ONLP_OID_TYPE.MODULE

        v = V()
        v.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidType, 0)
        if v.oids:
            self.auditModule(v.oids[0])
        else:
            self.log.warn("no modules found")

    def auditModule(self, oid):

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_module_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(onlp.onlp.ONLP_OID_TYPE.MODULE, (hdr.oid)>>24)

class ChassisTest(OnlpTestMixin,
                  unittest.TestCase):
    """Test interfaces in onlp/chassis.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_chassis_sw_init()
        libonlp.onlp_chassis_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testChassisOid(self):

        oid = onlp.onlp.ONLP_OID_CHASSIS
        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_chassis_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(onlp.onlp.ONLP_OID_TYPE.CHASSIS, (hdr._id)>>24)

        info = onlp.onlp.onlp_chassis_info()
        libonlp.onlp_chassis_info_get(oid, ctypes.byref(info))
        self.assertEqual(onlp.onlp.ONLP_OID_TYPE.CHASSIS, (info.hdr._id)>>24)

    def testChassisOidFormat(self):

        oid = onlp.onlp.ONLP_OID_CHASSIS

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertEqual('chassis-1', cjData['hdr']['id'])

    def testChassisInfoFormat(self):

        oid = onlp.onlp.ONLP_OID_CHASSIS
        info = onlp.onlp.onlp_chassis_info()
        libonlp.onlp_chassis_info_get(oid, ctypes.byref(info))

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_info_to_json(ctypes.byref(info), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertEqual('chassis-1', cjData['hdr']['id'])

class FanTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/fan.h."""

    FAN_ZERO_VALID = False
    # RPM 0 or PCT 0 is valid/clipped, or ignored

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_fan_sw_init()
        libonlp.onlp_fan_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def auditFanOid(self, oid):
        """Test the power-on behavior of a fan."""

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_fan_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(oid, hdr._id)

        fan = onlp.onlp.onlp_fan_info()
        libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))

        self.assertEqual(oid, fan.hdr._id)
        if not fan.model:
            self.log.warn("XXX roth -- missing fan model")
            ##raise AssertionError("missing fan model")
        if not fan.serial:
            self.log.warn("XXX roth -- missing fan serial")
            ##raise AssertionError("missing fan serial")
        self.log.info("auditing fan %d: %s (S/N %s)",
                      oid & 0xFFFFFF,
                      fan.model, fan.serial)

        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_RPM:
            self.assertGreater(fan.rpm, 0)
        else:
            self.log.warn("fan does not support RPM get")

        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_PERCENTAGE:
            self.assertGreater(fan.percentage, 0)
            self.assertLessEqual(fan.percentage, 100)
        else:
            self.log.warn("fan does not support PCT get")

        self.assert_(onlp.onlp.ONLP_OID_STATUS_FLAG.PRESENT & fan.hdr.status)
        # default, fan should be present

        if fan.dir & onlp.onlp.ONLP_FAN_DIR.B2F:
            if fan.caps & onlp.onlp.ONLP_ONLP_FAN_CAPS.GET_DIR:
                pass
            else:
                self.log.warn("XXX roth -- invalid fan dir")
                ##raise AssertionError("invalid fan dir")
        elif fan.dir & onlp.onlp.ONLP_FAN_DIR.F2B:
            if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_DIR:
                pass
            else:
                self.log.warn("XXX roth -- invalid fan dir")
                ##raise AssertionError("invalid fan dir")
        else:
            self.log.warn("fan direction not supported")

        # retrieve fan status separately
        libonlp.onlp_fan_hdr_get(oid, ctypes.byref(hdr))
        self.assert_(onlp.onlp.ONLP_OID_STATUS_FLAG.PRESENT & hdr.status)

        # try to manipulate the fan speed
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.SET_RPM:
            self.auditFanRpm(oid)
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.SET_PERCENTAGE:
            self.auditFanPct(oid)
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.SET_DIR:
            self.auditFanDir(oid)

    def auditFanRpm(self, oid):
        """Try to adjust the fan RPM.

        Note that the maximum fan speed is not know ahead of time.
        Also note the mechanicals here:
        - fan spin-up takes several seconds
        - fan spin-down takes much longer than spin-up
        - actual target fan speeds are set by the driver
        - for safety reasons there may not be an 'off' setting
        """

        fan = onlp.onlp.onlp_fan_info()
        libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
        minRpm = maxRpm = curRpm = fan.rpm

        speeds = []
        pcts = []

        with self.withoutOnlpd():
            try:

                self.log.info("probing for max fan speed")
                nspeed = curRpm
                while True:
                    self.log.info("current fan rpm is %d", nspeed)
                    self.log.info("trying higher fan rpm is %d", nspeed * 2)
                    libonlp.onlp_fan_rpm_set(oid, nspeed * 2)
                    time.sleep(5.0)
                    libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
                    self.log.info("probed fan speed is %d", fan.rpm)
                    if fan.rpm > (nspeed * 125 // 100):
                        nspeed = fan.rpm
                        continue

                    self.log.info("max fan speed is %d", fan.rpm)
                    maxRpm = fan.rpm
                    break

                self.log.info("probing for min fan speed")
                nspeed = curRpm
                while True:
                    self.log.info("setting fan rpm to %d", nspeed)
                    self.log.info("trying lower fan rpm is %d", nspeed // 2)
                    libonlp.onlp_fan_rpm_set(oid, nspeed // 2)

                    time.sleep(10.0)
                    # spin-down is slower than spin-up

                    libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
                    self.log.info("probed fan speed is %d", fan.rpm)
                    if fan.rpm < (nspeed * 75 // 100):
                        nspeed = fan.rpm
                        continue

                    self.log.info("min fan speed is %d", fan.rpm)
                    minRpm = fan.rpm
                    break

                self.assertLess(minRpm, maxRpm)

                self.log.info("cycling through fan speeds")
                for nspeed in range(minRpm, maxRpm, (maxRpm-minRpm)//3):
                    self.log.info("setting fan rpm to %d", nspeed)
                    libonlp.onlp_fan_rpm_set(oid, nspeed)
                    time.sleep(5.0)
                    libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
                    speeds.append(fan.rpm)
                    pcts.append(fan.percentage)

            finally:
                libonlp.onlp_fan_rpm_set(oid, curRpm)

        # fan speeds should be monotonically increasing
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_RPM:
            self.assertEqual(speeds, sorted(speeds))
            self.assertLess(minRpm * 95 // 100, speeds[0])
            self.assertGreater(maxRpm * 105 // 100, speeds[-1])
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_PERCENTAGE:
            self.assertEqual(pcts, sorted(pcts))
            ##self.assertEqual(0, pcts[0])
            ##self.assertGreater(105, pcts[-1])

    def auditFanPct(self, oid):
        """Try to adjust the fan percentage."""

        fan = onlp.onlp.onlp_fan_info()
        libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))

        speeds = []
        pcts = []
        with self.withoutOnlpd():
            try:

                libonlp.onlp_fan_percentage_set(oid, 0)
                time.sleep(15.0)
                # initially spin down the fan

                for npct in [0, 33, 66, 100,]:
                    self.log.info("setting fan percentage to %d", npct)
                    libonlp.onlp_fan_percentage_set(oid, npct)
                    time.sleep(5.0)
                    libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
                    self.log.info("fan percentage to %d --> %drpm", npct, fan.rpm)
                    speeds.append(fan.rpm)
                    pcts.append(fan.percentage)

            finally:
                libonlp.onlp_fan_percentage_set(oid, 100)

        if not self.FAN_ZERO_VALID:
            del speeds[0]
            del pcts[0]

        # fan speeds should be monotonically increasing
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_RPM:
            self.assertEqual(speeds, sorted(speeds))
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_PERCENTAGE:
            self.assertEqual(pcts, sorted(pcts))

    def auditFanDir(self, oid):
        """Try to adjust the fan direction."""
        unittest.skip("not implemented")

    def auditFanFormat(self, oid):

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('fan-', cjData['hdr']['id'])

        info = onlp.onlp.onlp_fan_info()
        libonlp.onlp_fan_info_get(oid, ctypes.byref(info))

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_info_to_json(ctypes.byref(info), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('fan', cjData['hdr']['id'])
        self.assertIn('rpm', cjData)

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_user_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        if cjh.value is None or cjh.value == 0:
            self.log.warn("XXX roth -- onlp_oid_to_user_json unimplemented")
        else:
            cjData = cjh.contents.data
            self.assertIn('Status', cjData)

    def testFindFans(self):
        """Verify that the system has fans."""

        class V(OidIterator):

            def __init__(self, log):
                super(V, self).__init__(log)
                self.oids = []

            def visit(self, oid, cookie):
                self.log.info("found FAN oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        oidTypeFlags = onlp.onlp.ONLP_OID_TYPE_FLAG.FAN
        v = V(log=self.log.getChild("fan"))
        v.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, 0)
        self.assert_(v.oids)

        for oid in v.oids:
            hdr = onlp.onlp.onlp_oid_hdr()
            libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))
            if hdr.status & onlp.onlp.ONLP_OID_STATUS_FLAG.PRESENT:
                self.auditFanFormat(oid)
                self.auditFanOid(oid)
                return
            else:
                self.log.warn("fan %s status %d",
                              oid & 0xffffff, hdr.status)
        raise AssertionError("no fans found")

class LedTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/led.h.

    XXX roth -- need to flesh this out using a physical device.
    """

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_led_sw_init()
        libonlp.onlp_led_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testFindLeds(self):
        """Verify that the system has LEDs."""

        class V(OidIterator):

            def __init__(self, log):
                super(V, self).__init__(log)
                self.oids = []

            def visit(self, oid, cookie):
                self.log.info("found LED oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        oidTypeFlags = onlp.onlp.ONLP_OID_TYPE_FLAG.LED
        v = V(log=self.log.getChild("led"))
        v.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, 0)
        self.assert_(v.oids)

        self.auditLedFormat(v.oids[0])
        self.auditLedOid(v.oids[0])

    def auditLedFormat(self, oid):

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('led-', cjData['hdr']['id'])

        info = onlp.onlp.onlp_led_info()
        libonlp.onlp_led_info_get(oid, ctypes.byref(info))

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_info_to_json(ctypes.byref(info), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('led', cjData['hdr']['id'])
        self.assertIn('mode', cjData)

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_user_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        if cjh.value is None or cjh.value == 0:
            self.log.warn("XXX roth -- onlp_oid_to_user_json unimplemented")
        else:
            cjData = cjh.contents.data
            self.assertIn('Status', cjData)

    def auditLedOid(self, oid):

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_led_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(oid, hdr._id)

        led = onlp.onlp.onlp_led_info()
        libonlp.onlp_led_info_get(oid, ctypes.byref(led))

        self.assertEqual(oid, led.hdr._id)

        self.assert_(led.caps)
        # should support some non-empty set of capabilities

        self.log.info("auditing led %d",
                      oid & 0xFFFFFF)

        self.assert_(led.hdr.status & onlp.onlp.ONLP_OID_STATUS_FLAG.PRESENT)

        self.log.info("found LED caps [%s]", flags2str(onlp.onlp.ONLP_LED_CAPS, led.caps))

        with self.withoutOnlpd():
            if led.caps & onlp.onlp.ONLP_LED_CAPS.CHAR:
                self.auditLedChar(oid)
            self.auditLedColors(oid)
            self.auditLedBlink(oid)

    def hasColors(self, caps):
        """True if this a color LED."""

        if caps & onlp.onlp.ONLP_LED_CAPS.RED:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.RED_BLINKING:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.ORANGE:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.ORANGE_BLINKING:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.YELLOW:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.YELLOW_BLINKING:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.GREEN:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.GREEN_BLINKING:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.BLUE:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.BLUE_BLINKING:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.PURPLE:
            return True
        if caps & onlp.onlp.ONLP_LED_CAPS.PURPLE_BLINKING:
            return True

        return False

    def auditLedChar(self, oid):

        led = onlp.onlp.onlp_led_info()
        libonlp.onlp_led_info_get(oid, ctypes.byref(led))
        saveChar = led.character

        try:
            for nchar in ['0', '1', '2', '3',]:
                self.log.info("led %d: char '%s'", oid, nchar)

                sts = libonlp.onlp_led_char_set(oid, nchar)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(nchar, led.character)
        finally:
            libonlp.onlp_led_char_set(oid, saveChar)

    def auditLedColors(self, oid):

        led = onlp.onlp.onlp_led_info()
        libonlp.onlp_led_info_get(oid, ctypes.byref(led))
        saveMode = led.mode

        allModes = []
        if led.caps & onlp.onlp.ONLP_LED_CAPS.RED:
            allModes.append(onlp.onlp.ONLP_LED_MODE.RED)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.ORANGE:
            allModes.append(onlp.onlp.ONLP_LED_MODE.ORANGE)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.YELLOW:
            allModes.append(onlp.onlp.ONLP_LED_MODE.YELLOW)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.GREEN:
            allModes.append(onlp.onlp.ONLP_LED_MODE.GREEN)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.BLUE:
            allModes.append(onlp.onlp.ONLP_LED_MODE.BLUE)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.PURPLE:
            allModes.append(onlp.onlp.ONLP_LED_MODE.PURPLE)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.AUTO:
            allModes.append(onlp.onlp.ONLP_LED_MODE.AUTO)

        if not allModes:
            unittest.skip("colors not supported")
            return
        self.log.info("found %d supported colors", len(allModes))

        try:
            for ncolor in allModes:
                self.log.info("led %d: color '%s'", oid, onlp.onlp.ONLP_LED_MODE.name(ncolor))

                sts = libonlp.onlp_led_mode_set(oid, ncolor)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(ncolor, led.mode)

                self.log.info("led %d: OFF", oid)

                sts = libonlp.onlp_led_mode_set(oid, onlp.onlp.ONLP_LED_MODE.OFF)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(onlp.onlp.ONLP_LED_MODE.OFF, led.mode)

        finally:
            libonlp.onlp_led_mode_set(oid, saveMode)

    def auditLedBlink(self, oid):

        led = onlp.onlp.onlp_led_info()
        libonlp.onlp_led_info_get(oid, ctypes.byref(led))
        saveMode = led.mode

        allModes = []
        if led.caps & onlp.onlp.ONLP_LED_CAPS.RED_BLINKING:
            allModes.append(onlp.onlp.ONLP_LED_MODE.RED_BLINKING)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.ORANGE_BLINKING:
            allModes.append(onlp.onlp.ONLP_LED_MODE.ORANGE_BLINKING)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.YELLOW_BLINKING:
            allModes.append(onlp.onlp.ONLP_LED_MODE.YELLOW_BLINKING)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.GREEN_BLINKING:
            allModes.append(onlp.onlp.ONLP_LED_MODE.GREEN_BLINKING)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.BLUE_BLINKING:
            allModes.append(onlp.onlp.ONLP_LED_MODE.BLUE_BLINKING)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.PURPLE_BLINKING:
            allModes.append(onlp.onlp.ONLP_LED_MODE.PURPLE_BLINKING)
        if led.caps & onlp.onlp.ONLP_LED_CAPS.AUTO_BLINKING:
            allModes.append(onlp.onlp.ONLP_LED_MODE.AUTO_BLINKING)

        if not allModes:
            unittest.skip("blinking colors not supported")
            return
        self.log.info("found %d supported blink colors", len(allModes))

        try:
            for ncolor in allModes:
                self.log.info("led %d: blink color '%s'", oid, onlp.onlp.ONLP_LED_MODE.name(ncolor))

                sts = libonlp.onlp_led_mode_set(oid, ncolor)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(ncolor, led.mode)

                self.log.info("led %d: OFF", oid)

                sts = libonlp.onlp_led_mode_set(oid, onlp.onlp.ONLP_LED_MODE.OFF)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(onlp.onlp.ONLP_LED_MODE.OFF, led.mode)

        finally:
            libonlp.onlp_led_mode_set(oid, saveMode)

class ConfigTest(OnlpTestMixin,
                 unittest.TestCase):
    """Test interfaces in onlp/onlp_config.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testConfig(self):

        s = libonlp.onlp_config_lookup("ONLP_CONFIG_INFO_STR_MAX")
        self.assertEqual('64', s)

        s = libonlp.onlp_config_lookup("foo")
        self.assertIsNone(s)

    def testConfigShow(self):

        libonlp.onlp_config_show(self.aim_pvs_buffer.ptr)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        self.assertIn("ONLP_CONFIG_INFO_STR_MAX = 64\n", buf.string_at())

class ThermalTest(OnlpTestMixin,
                  unittest.TestCase):
    """Test interfaces in onlp/thermal.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_thermal_sw_init()
        libonlp.onlp_thermal_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testFindThermal(self):

        class V(OidIterator):

            def __init__(self, log):
                super(V, self).__init__(log)
                self.oids = []

            def visit(self, oid, cookie):
                self.log.info("found thermal oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        oidTypeFlags = onlp.onlp.ONLP_OID_TYPE_FLAG.THERMAL
        v = V(log=self.log.getChild("thermal"))
        v.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, 0)
        self.assert_(v.oids)

        self.auditThermalFormat(v.oids[0])
        self.auditThermalOid(v.oids[0])

    def auditThermalFormat(self, oid):

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('thermal-', cjData['hdr']['id'])

        info = onlp.onlp.onlp_thermal_info()
        libonlp.onlp_thermal_info_get(oid, ctypes.byref(info))

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_info_to_json(ctypes.byref(info), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('thermal', cjData['hdr']['id'])
        self.assertIn('mcelsius', cjData)

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_user_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        if cjh.value is None or cjh.value == 0:
            self.log.warn("XXX roth -- onlp_oid_to_user_json unimplemented")
        else:
            cjData = cjh.contents.data
            self.assertIn('Status', cjData)

    def auditThermalOid(self, oid):

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_thermal_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(oid, hdr._id)

        thm = onlp.onlp.onlp_thermal_info()
        libonlp.onlp_thermal_info_get(oid, ctypes.byref(thm))

        self.assertEqual(oid, thm.hdr._id)

        self.assert_(thm.caps)
        # should support some non-empty set of capabilities

        if thm.caps == onlp.onlp.ONLP_THERMAL_CAPS_ALL:
            self.log.info("found THM caps [ALL]")
        else:
            self.log.info("found THM caps [%s]", flags2str(onlp.onlp.ONLP_THERMAL_CAPS, thm.caps))

        self.assert_(thm.caps & onlp.onlp.ONLP_THERMAL_CAPS.GET_TEMPERATURE)
        # sensor should at least report temperature

        self.log.info("auditing thermal %d",
                      oid & 0xFFFFFF)

        self.assert_(thm.hdr.status & onlp.onlp.ONLP_OID_STATUS_FLAG.PRESENT)
        # sensor should be present

        self.assertGreater(thm.mcelcius, 20000)
        self.assertLess(thm.mcelcius, 35000)
        # temperature should be non-crazy

class PsuTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/psu.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_psu_sw_init()
        libonlp.onlp_psu_hw_init(0)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testFindPsu(self):

        class V(OidIterator):

            def __init__(self, log):
                super(V, self).__init__(log)
                self.oids = []

            def visit(self, oid, cookie):
                self.log.info("found psu %d", oid & 0xffffff)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        oidTypeFlags = onlp.onlp.ONLP_OID_TYPE_FLAG.PSU
        v = V(log=self.log.getChild("psu"))
        v.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidTypeFlags, 0)
        self.assert_(v.oids)

        # find a PSU that is plugged in and present
        found = False
        for oid in v.oids:
            hdr = onlp.onlp.onlp_oid_hdr()
            libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))
            if (hdr.status & onlp.onlp.ONLP_OID_STATUS_FLAG.PRESENT
                and not hdr.status & onlp.onlp.ONLP_OID_STATUS_FLAG.UNPLUGGED):
                found = True
                self.auditPsuFormat(oid)
                self.auditPsuOid(oid)
        self.assert_(found)

    def auditPsuFormat(self, oid):

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('psu-', cjData['hdr']['id'])

        info = onlp.onlp.onlp_psu_info()
        libonlp.onlp_psu_info_get(oid, ctypes.byref(info))

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_info_to_json(ctypes.byref(info), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('psu', cjData['hdr']['id'])
        self.assertIn('mvout', cjData)

    def auditPsuOid(self, oid):

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(oid, hdr._id)

        psu = onlp.onlp.onlp_psu_info()
        sts = libonlp.onlp_psu_info_get(oid, ctypes.byref(psu))
        self.assertStatusOK(sts)

        self.assertEqual(oid, psu.hdr._id)

        self.assertLess(-1, psu.type)
        self.log.info("found PSU type %s",
                      onlp.onlp.ONLP_PSU_TYPE.name(psu.type))

        self.log.info("found PSU caps [%s]",
                      flags2str(onlp.onlp.ONLP_PSU_CAPS, psu.caps))
        vcaps = (psu.caps
                 & (onlp.onlp.ONLP_PSU_CAPS.GET_VOUT
                    | onlp.onlp.ONLP_PSU_CAPS.GET_VIN
                    | onlp.onlp.ONLP_PSU_CAPS.GET_IOUT
                    | onlp.onlp.ONLP_PSU_CAPS.GET_IIN
                    | onlp.onlp.ONLP_PSU_CAPS.GET_POUT
                    | onlp.onlp.ONLP_PSU_CAPS.GET_PIN))
        if not vcaps:
            self.log.warn("XXX roth -- missing caps")
            ##raise AssertionError("invalid/mssing caps")
        # should support some non-empty set of capabilities

        self.log.info("auditing psu %d",
                      oid & 0xFFFFFF)

        self.assert_(psu.hdr.status & onlp.onlp.ONLP_OID_STATUS_FLAG.PRESENT)
        # sensor should be present

        # hm, maybe 12v or 5v or some such
        if (psu.type == onlp.onlp.ONLP_PSU_TYPE.AC
            and (psu.caps & onlp.onlp.ONLP_PSU_CAPS.GET_VOUT)):
            self.log.info("PSU is reading %.1fV AC",
                          1.0 * psu.mvout / 1000)
            self.assertGreater(psu.mvout, 5000)
            self.assertLess(psu.mvout, 250000)

        if (psu.type == onlp.onlp.ONLP_PSU_TYPE.DC12
            and (psu.caps & onlp.onlp.ONLP_PSU_CAPS.GET_VOUT)):
            self.log.info("PSU is reading %.1fV DC",
                          1.0 * psu.mvout / 1000)
            self.assertGreater(psu.mvout, 11000)
            self.assertLess(psu.mvout, 13000)
        if (psu.type == onlp.onlp.ONLP_PSU_TYPE.DC48
            and (psu.caps & onlp.onlp.ONLP_PSU_CAPS.GET_VOUT)):
            self.log.info("PSU is reading %.1fV DC",
                          1.0 * psu.mvout / 1000)
            self.assertGreater(psu.mvout, 47000)
            self.assertLess(psu.mvout, 49000)
        # output voltage should be non-crazy

class Eeprom(ctypes.Structure):
    _fields_ = [('eeprom', ctypes.c_ubyte * 256,),]

class SfpTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/sfp.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_sfp_sw_init()
        libonlp.onlp_sfp_hw_init(0)

        self.bitmap = aim.aim_bitmap256()

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

        libonlp.onlp_sfp_sw_denit()

        ##libonlp.onlp_sfp_hw_denit()
        ### XXX roth -- missing

    def bitmap2list(self, bitmap=None):
        outBits = []
        bitmap = bitmap or self.bitmap
        for pos in range(256):
            outBits.append(aim.aim_bitmap_get(bitmap.hdr, pos))
        return outBits

    def testBitmap(self):
        """Verify that our aim_bitmap implementation is sound."""

        refBits = []
        for pos in range(256):
            val = random.randint(0, 1)
            refBits.append(val)
            aim.aim_bitmap_mod(self.bitmap.hdr, pos, val)

        for i in range(1000):
            pos = random.randint(0, 255)
            val = refBits[pos] ^ 1
            refBits[pos] = val
            aim.aim_bitmap_mod(self.bitmap.hdr, pos, val)

        self.assertEqual(refBits, self.bitmap2list())

        refBits = [0] * 256
        libonlp.onlp_sfp_bitmap_t_init(ctypes.byref(self.bitmap))
        self.assertEqual(refBits, self.bitmap2list())

    def testValid(self):
        """Test for valid SFP ports."""

        libonlp.onlp_sfp_bitmap_t_init(ctypes.byref(self.bitmap))
        sts = libonlp.onlp_sfp_bitmap_get(ctypes.byref(self.bitmap))
        self.assertStatusOK(sts)
        refBits = [0] * 256

        ports = [x[0] for x in enumerate(self.bitmap2list()) if x[1]]
        self.log.info("found %d SFP ports", len(ports))
        self.assert_(ports)

        self.assertEqual(0, ports[0])
        self.assertEqual(len(ports)-1, ports[-1])
        # make sure the ports are contiguous, starting from 1

        # make sure the per-port valid bits are correct
        for i in range(256):
            sts = libonlp.onlp_sfp_port_valid(i)
            if i < len(ports):
                self.assertEqual(onlp.onlp.ONLP_STATUS.OK, sts)
            else:
                self.assertEqual(onlp.onlp.ONLP_STATUS.E_PARAM, sts)

        # see if any of them are present
        # XXX this test requires at least one of the SFPs to be present.
        bm = aim.aim_bitmap256()
        sts = libonlp.onlp_sfp_presence_bitmap_get(ctypes.byref(bm))
        self.assertStatusOK(sts)
        present = [x[0] for x in enumerate(self.bitmap2list(bm)) if x[1]]
        self.log.info("found %d SFPs", len(present))
        self.assert_(present)

        presentSet = set(present)
        portSet = set(ports)

        for port in presentSet:
            if port not in portSet:
                raise AssertionError("invalid SFP %d not valid"
                                     % (port,))

        for i in range(256):
            valid = libonlp.onlp_sfp_is_present(i)
            if i in presentSet:
                self.assertEqual(1, valid)
            elif i in portSet:
                self.assertEqual(0, valid)
            else:
                self.assertGreater(0, valid)

        # test the rx_los bitmap
        # (tough to be more detailed since it depends on connectivity)
        sts = libonlp.onlp_sfp_rx_los_bitmap_get(ctypes.byref(bm))
        if sts != onlp.onlp.ONLP_STATUS.E_UNSUPPORTED:
            self.assertStatusOK(sts)
            rxLos = [x[0] for x in enumerate(self.bitmap2list(bm)) if x[1]]

            # any port exhibiting rx_los should actually be a port
            for i in rxLos:
                self.assertIn(i, portSet)

            # any missing SFP should *NOT* be exhibiting rx_los
            rxLosSet = set(rxLos)
            for i in portSet:
                if not i in presentSet:
                    self.assertNotIn(i, rxLosSet)

        port = present[0]
        self.log.info("auditing SFP %d", port)
        self.auditSfpFormat(port)
        self.auditSfpEeprom(port)

    def auditSfpEeprom(self, port):

        self.auditControl(port)

        info = onlp.onlp.onlp_sfp_info()
        sts = libonlp.onlp_sfp_info_get(port, ctypes.byref(info))
        self.assertStatusOK(sts)

        # try to read in the data manually
        for i in range(128):
            b = libonlp.onlp_sfp_dev_readb(port, 0x50, i)
            if b != info.bytes.a0[i]:
                raise AssertionError("eeprom mismatch at 0x50.%d" % i)

        monType = info.bytes.a0[92] & 0x40
        # See e.g. https://www.optcore.net/wp-content/uploads/2017/04/SFF_8472.pdf

        self.auditEeprom(info.bytes.a0)

        if monType:

            # if DOM is supported, the DOM data should already be here
            self.auditDomInfo(info.dom)

            # try to reconstruct using raw reads
            domData = ctypes.c_ubyte * 256
            for i in range(128):
                domData[i] = libonlp.onlp_sfp_dev_readb(port, 0x51, i)

            domInfo = sff.sff.sff_dom_info()
            sts = sff_dom_info_get(ctypes.byref(domData), info.bytes.a0, domData)
            self.assertStatusOK(sts)

            self.auditDomInfo(domInfo)

            # make sure this is the same dom info (but ignore the actual metrics)
            self.assertEqual(info.dom.spec, domInfo.spec)
            self.assertEqual(info.dom.fields, domInfo.fields)
            self.assertEqual(info.dom.extcal, domInfo.extcal)
            self.assertEqual(info.dom.nchannels, domInfo.nchannels)

        else:
            self.log.info("this SFP does not support DOM")
            self.assertEqual(sff.sff.SFF_DOM_SPEC.UNSUPPORTED, info.dom.spec)

    def auditSfpFormat(self, port):

        oid = (port+1) | (onlp.onlp.ONLP_OID_TYPE.SFP<<24)

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data
        self.assertIn('sfp-', cjData['hdr']['id'])

        info = onlp.onlp.onlp_sfp_info()
        libonlp.onlp_sfp_info_get(oid, ctypes.byref(info))

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_info_to_json(ctypes.byref(info), cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData = cjh.contents.data

        self.assertIn('sfp', cjData['hdr']['id'])
        self.assertIn('type', cjData)

        # get info at the oid level

        ip = onlp.onlp.onlp_oid_info_ptr(0)
        libonlp.onlp_oid_info_get(oid, ip.hnd)

        cjh = cjson_util.cJSONHandle()
        flags = 0

        sts = libonlp.onlp_oid_info_to_json(ip.ptr, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData2 = cjh.contents.data

        self.assertIn('sfp', cjData['hdr']['id'])
        self.assertIn('type', cjData)

        self.assertEqual(cjData, cjData2)

        # try recursive output

        cjh = cjson_util.cJSONHandle()
        flags = onlp.onlp.ONLP_OID_JSON_FLAG.RECURSIVE

        sts = libonlp.onlp_oid_info_to_json(ip.ptr, cjh.hnd, flags)
        self.assertStatusOK(sts)

        cjData3 = cjh.contents.data

        self.assertIn('sfp', cjData['hdr']['id'])
        self.assertIn('type', cjData)

        self.assertEqual(cjData2, cjData3)
        # recursive output should be identical

        cjh = cjson_util.cJSONHandle()
        flags = 0
        sts = libonlp.onlp_oid_to_user_json(oid, cjh.hnd, flags)
        self.assertStatusOK(sts)

        if cjh.value is None or cjh.value == 0:
            self.log.warn("XXX roth -- onlp_oid_to_user_json unimplemented")
        else:
            cjData = cjh.contents.data
            self.assertIn('Status', cjData)

    def auditEeprom(self, eeprom):
        """Audit that the entries for this SFP are valid."""

        sffEeprom = sff.sff.sff_eeprom()
        sts = libonlp.sff_eeprom_parse(ctypes.byref(sffEeprom), eeprom)
        self.assertStatusOK(sts)

        self.assertEqual(1, sffEeprom.identified)

        # XXX info strings include space padding
        vendor = sffEeprom.info.vendor.strip()
        self.assert_(vendor)
        model = sffEeprom.info.model.strip()
        self.assert_(model)
        serial = sffEeprom.info.serial.strip()
        self.assert_(serial)

        self.log.info("found SFP: %s %s (S/N %s)",
                      vendor, model, serial)

        self.log.info("%s (%s %s)",
                      sffEeprom.info.module_type_name,
                      sffEeprom.info.media_type_name,
                      sffEeprom.info.sfp_type_name)

        sffType = libonlp.sff_sfp_type_get(eeprom)
        self.assertEqual(sffType, sffEeprom.info.sfp_type)

        moduleType = libonlp.sff_module_type_get(eeprom)
        self.assertEqual(moduleType, sffEeprom.info.module_type)

        mediaType = libonlp.sff_media_type_get(sffEeprom.info.module_type)
        self.assertEqual(mediaType, sffEeprom.info.media_type)

        caps = ctypes.c_uint32()
        sts = libonlp.sff_module_caps_get(sffEeprom.info.module_type, ctypes.byref(caps))
        self.assertStatusOK(sts)
        self.assert_(caps)
        capsStr = flags2str(sff.sff.SFF_MODULE_CAPS, caps.value)
        self.log.info("module caps [%s]", capsStr)

        libonlp.sff_info_show(ctypes.byref(sffEeprom.info), self.aim_pvs_buffer.ptr)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        self.assertIn("Vendor:", buf.string_at())

        # cons up a new info structure
        # XXX includes space padding

        info = sff.sff.sff_info()
        sts = libonlp.sff_info_init(ctypes.byref(info),
                                    sffEeprom.info.module_type,
                                    sffEeprom.info.vendor,
                                    sffEeprom.info.model,
                                    sffEeprom.info.serial,
                                    sffEeprom.info.length)
        self.assertStatusOK(sts)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer.ptr)
        libonlp.sff_info_show(ctypes.byref(info), self.aim_pvs_buffer.ptr)
        buf2 = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        self.assertEqual(buf2.string_at(), buf.string_at())

        # test parsing from a file

        try:
            fno, p = tempfile.mkstemp(prefix="sfp-", suffix=".data")
            with os.fdopen(fno, "w") as fd:
                for i in range(256):
                    fd.write("%c" % sffEeprom.eeprom[i])

            sffEeprom2 = sff.sff.sff_eeprom()
            sts = libonlp.sff_eeprom_parse_file(ctypes.byref(sffEeprom2), p);
            self.assertStatusOK(sts)
            self.assertEqual(1, sffEeprom2.identified)

        finally:
            os.unlink(p)

        # test valid vs. invalid

        sts = libonlp.sff_eeprom_validate(ctypes.byref(sffEeprom), 1)
        self.assertGreater(sts, 0)

        libonlp.sff_eeprom_invalidate(ctypes.byref(sffEeprom))

        sts = libonlp.sff_eeprom_validate(ctypes.byref(sffEeprom), 1)
        self.assertEqual(sts, 0)

    def auditDomInfo(self, domData):
        self.assertNotEqual(sff.sff.SFF_DOM_SPEC.UNSUPPORTED, domData.spec)
        self.log.info("found DOM type %s",
                      sff.sff.SFF_DOM_SPEC.name(domData.spec))

        self.assertGreater(domData.nchannels, 0)

        self.assertGreater(domData.temp, 23*256)
        # temperature in 1/256 Celsius

    def auditControl(self, port):

        flags = ctypes.c_uint32()
        sts = libonlp.onlp_sfp_control_flags_get(port, ctypes.byref(flags))
        self.assertStatusOK(sts)

        for i in range(32):
            fl = (1<<i)
            name = onlp.onlp.ONLP_SFP_CONTROL_FLAG.name(fl)
            if name is None: break

            ctlEnum = getattr(onlp.onlp.ONLP_SFP_CONTROL, name)

            ctl = ctypes.c_int()
            sts = libonlp.onlp_sfp_control_get(port, ctlEnum, ctypes.byref(ctl))

            if flags.value & fl:
                self.log.info("found control flag %s", name)
                self.assertEqual(1, ctl.value)
            else:
                self.assertEqual(0, ctl.value)

        # let's try resetting

        sts = libonlp.onlp_sfp_control_set(port, onlp.onlp.ONLP_SFP_CONTROL.RESET, 1)
        if sts != onlp.onlp.ONLP_STATUS.E_UNSUPPORTED:
            self.assertStatusOK(sts)

            time.sleep(1)

            ctl = ctypes.c_int()
            sts = libonlp.onlp_sfp_control_get(port, onlp.onlp.ONLP_SFP_CONTROL.RESET_STATE, ctypes.byref(ctl))
            self.assertStatusOK(sts)
            self.assertEqual(1, ctl.value)

            sts = libonlp.onlp_sfp_control_set(port, onlp.onlp.ONLP_SFP_CONTROL.RESET, 0)
            self.assertStatusOK(sts)

            time.sleep(1)

            ctl = ctypes.c_int()
            sts = libonlp.onlp_sfp_control_get(port, onlp.onlp.ONLP_SFP_CONTROL.RESET_STATE, ctypes.byref(ctl))
            self.assertStatusOK(sts)
            self.assertEqual(0, ctl.value)

        else:
            self.log.warn("RESET not supported by this SFP")

class GenericTest(OnlpTestMixin,
                  unittest.TestCase):
    """Test interfaces in onlp/generic.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testGeneric(self):

        class V(OidIterator):

            def __init__(self, log=None):
                super(V, self).__init__(log=log)
                self.oids = []

            def visit(self, oid, cookie):
                self.log.info("found oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        oidType = onlp.onlp.ONLP_OID_TYPE.GENERIC

        v = V()
        v.foreach(onlp.onlp.ONLP_OID_CHASSIS, oidType, 0)
        if v.oids:
            self.auditModule(v.oids[0])
        else:
            self.log.warn("no generic components found")

    def auditGeneric(self, oid):

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_generic_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(onlp.onlp.ONLP_OID_TYPE.GENERIC, (hdr.oid)>>24)

if __name__ == "__main__":
    logging.basicConfig()
    unittest.main()

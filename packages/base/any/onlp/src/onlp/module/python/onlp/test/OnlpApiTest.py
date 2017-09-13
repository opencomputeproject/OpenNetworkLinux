"""OnlpApiTest.py

Test the API bindings.
"""

import ctypes
import unittest
import logging
import re
import time

import onlp.onlp
onlp.onlp.onlp_init()

libonlp = onlp.onlp.libonlp

class OnlpTestMixin(object):

    def setUp(self):

        self.log = logging.getLogger("onlp")
        self.log.setLevel(logging.DEBUG)

        self.aim_pvs_buffer_p = libonlp.aim_pvs_buffer_create()
        self.aim_pvs_buffer = self.aim_pvs_buffer_p.contents

    def tearDown(self):

        libonlp.aim_pvs_destroy(self.aim_pvs_buffer_p)

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

        nullString = onlp.onlp.aim_char_p(None)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        self.assertEqual(nullString, buf)

        libonlp.aim_printf(self.aim_pvs_buffer_p, "hello\n")
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        self.assertEqual("hello\n", buf.string_at())

        libonlp.aim_printf(self.aim_pvs_buffer_p, "world\n")
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        self.assertEqual("hello\nworld\n", buf.string_at())

        libonlp.aim_printf(self.aim_pvs_buffer_p, "%d\n", 42)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        self.assertEqual("hello\nworld\n42\n", buf.string_at())

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer_p)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        self.assertEqual(nullString, buf)

class OnlpTest(OnlpTestMixin,
               unittest.TestCase):
    """Test interfaces in onlp/onlp.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testPlatformDump(self):
        """Verify basic platform dump output."""

        flags = 0
        libonlp.onlp_platform_dump(self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIn("System Information:", bufStr)
        self.assertIn("thermal @ 1", bufStr)

    def testPlatformDumpFlags(self):
        """Verify platform dump flags are honored."""

        flags = 0
        libonlp.onlp_platform_dump(self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIn("psu @ 1", bufStr)
        self.assertNotIn("PSU-1 Fan", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer_p)

        flags = onlp.onlp.ONLP_OID_DUMP_F_RECURSE
        libonlp.onlp_platform_dump(self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIn("psu @ 1", bufStr)
        self.assertIn("PSU-1 Fan", bufStr)

        # hard to test onlp.onlp.ONLP_OID_DUMP_F_RECURSE,
        # since it depends on whether a specific component is inserted

    def testPlatformShow(self):
        """Verify basic platform show output."""

        flags = 0
        libonlp.onlp_platform_show(self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIn("System Information:", bufStr)

    def testPlatformShowFlags(self):
        """Verify that onlp_platform_show honors flags."""

        flags = 0
        libonlp.onlp_platform_show(self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertNotIn("PSU 1", bufStr)
        self.assertNotIn("PSU-1 Fan", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer_p)

        flags = onlp.onlp.ONLP_OID_SHOW_F_RECURSE
        libonlp.onlp_platform_show(self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIn("PSU 1", bufStr)
        self.assertIn("PSU-1 Fan", bufStr)

class SysHdrMixin(object):

    def auditOidHdr(self, hdr):

        self.assertEqual(0, hdr.poid)
        if hdr._id == onlp.onlp.ONLP_OID_SYS:
            pass
        elif hdr._id == 0:
            self.log.warn("invalid system OID 0")
        else:
            raise AssertionError("invalid system OID")
        # root OID

        coids = [x for x in hdr.children()]
        self.assert_(coids)
        self.assert_(len(coids) < onlp.onlp.ONLP_OID_TABLE_SIZE)

        def _oidType(oid):
            if oid.isSystem():
                return "sys"
            if oid.isThermal():
                return "thm"
            if oid.isFan():
                return "fan"
            if oid.isPsu():
                return "psu"
            if oid.isLed():
                return "led"
            if oid.isModule():
                return "mod"
            if oid.isRtc():
                return "rtc"
            return "unk"

        for coid in coids:
            self.log.info("oid %d (%s): %s",
                          coid._id, _oidType(coid), coid.description)
            if _oidType(coid) == "unk":
                raise AssertionError("invalid oid")
            self.assertEqual(hdr._id, coid.poid)

            # if it's a PSU, verify that it has fans
            if _oidType(coid) == "psu":
                foids = [x for x in coid.children()]
                for foid in foids:
                    self.log.info("oid %d (%s): %s",
                                  foid._id, _oidType(foid), foid.description)
                    self.assertEqual("fan", _oidType(foid))
                    # parent should the the PSU or the chassis
                    if coid._id == foid.poid:
                        pass
                    elif hdr._id == foid.poid:
                        pass
                    else:
                        raise AssertionError("invalid parent OID")

class SysTest(OnlpTestMixin,
              SysHdrMixin,
              unittest.TestCase):
    """Test interfaces in onlp/sys.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_sys_init()
        self.sys_info = onlp.onlp.onlp_sys_info()

        libonlp.onlp_sys_info_get(ctypes.byref(self.sys_info))
        self.sys_info.initialized = True

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testNoop(self):
        pass

    def testOnieInfo(self):
        """Verify the ONIE fields."""

        product_re = re.compile("(.*)-(.*)-(.*)")
        m = product_re.match(self.sys_info.onie_info.product_name)
        self.assertIsNotNone(m)

        vendor_re = re.compile("[A-Z][a-z]*[a-zA-Z0-9_. -]")
        m = vendor_re.match(self.sys_info.onie_info.vendor)
        self.assertIsNotNone(m)

        self.assertIn('.', self.sys_info.onie_info.onie_version)

        # see if there are any vendor extensions
        # if there are any, make sure the are well-formed
        for vx in self.sys_info.onie_info.vx_list:
            sz = vx.size
            self.assert_(sz <= 256)

    def testPlatformInfo(self):
        """Verify the platform info fields."""
        # XXX VM platforms have null for both
        pass

    def testSysHeaderOnie(self):
        """Test the sys_hdr data that is in the sys_info."""
        self.auditOidHdr(self.sys_info.hdr)

    def testSysHeader(self):
        """Test the sys_hdr data available via sys_hdr_get."""

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_sys_hdr_get(ctypes.byref(hdr))
        self.auditOidHdr(hdr)

    def testManage(self):
        """Verify we can start/stop platform management."""

        code = libonlp.onlp_sys_platform_manage_start(0)
        self.assert_(code >= 0)

        for i in range(10):
            libonlp.onlp_sys_platform_manage_now()
            time.sleep(0.25)

        code = libonlp.onlp_sys_platform_manage_stop(0)
        self.assert_(code >= 0)

        time.sleep(2.0)

        code = libonlp.onlp_sys_platform_manage_join(0)
        self.assert_(code >= 0)

    def testSysDump(self):
        """Test the SYS OID debug dump."""

        oid = onlp.onlp.ONLP_OID_SYS
        flags = 0
        libonlp.onlp_sys_dump(oid, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIn("System Information", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer_p)

        # this is not the system OID

        oid = self.sys_info.hdr.coids[0]
        libonlp.onlp_sys_dump(oid, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIsNone(bufStr)

    def testSysShow(self):
        """Test the OID status."""

        oid = onlp.onlp.ONLP_OID_SYS
        flags = 0
        libonlp.onlp_sys_show(oid, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIn("System Information", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer_p)

        # this is not the system OID

        oid = self.sys_info.hdr.coids[0]
        libonlp.onlp_sys_show(oid, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        bufStr = buf.string_at()
        self.assertIsNone(bufStr)

    def testSysIoctl(self):
        """Test the IOCTL interface."""

        # no such ioctl

        code = libonlp.onlp_sys_ioctl(9999)
        self.assertEqual(onlp.onlp.ONLP_STATUS_E_UNSUPPORTED, code)

class OidTest(OnlpTestMixin,
              SysHdrMixin,
              unittest.TestCase):
    """Test interfaces in onlp/oids.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        self.hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_oid_hdr_get(onlp.onlp.ONLP_OID_SYS, ctypes.byref(self.hdr))

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testSystemOid(self):
        """Audit the system oid."""
        self.auditOidHdr(self.hdr)

    def testOidIter(self):
        """Test the oid iteration functions."""

        class OidIterator(object):

            def __init__(self, log):
                self.log = log or logging.getLogger("visit")

            def visit(self, oid, cookie):
                return 0

            def cvisit(self):
                def _v(oid, cookie):
                    try:
                        return self.visit(oid, cookie)
                    except:
                        self.log.exception("visitor failed")
                        return -1
                return onlp.onlp.onlp_oid_iterate_f(_v)

        class V1(OidIterator):

            def __init__(self, cookie, log):
                super(V1, self).__init__(log)
                self.cookie = cookie
                self.oids = []

            def visit(self, oid, cookie):
                if cookie != self.cookie:
                    raise AssertionError("invalid cookie")
                self.log.info("found oid %d", oid)
                self.oids.append(oid)
                return 0

        oidType = 0
        cookie = 0xdeadbeef

        # gather all OIDs

        v1 = V1(cookie, log=self.log.getChild("v1"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS, oidType, v1.cvisit(), cookie)
        self.assert_(v1.oids)
        oids = list(v1.oids)

        # validate error recovery

        v2 = V1(cookie+1, log=self.log.getChild("v2"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS, oidType, v2.cvisit(), cookie)
        self.assertEqual([], v2.oids)

        # validate early exit

        class V3(OidIterator):

            def __init__(self, log):
                super(V3, self).__init__(log)
                self.oids = []

            def visit(self, oid, cookie):
                if oid == cookie:
                    return -1
                self.log.info("found oid %d", oid)
                self.oids.append(oid)
                return 0

        v3 = V3(log=self.log.getChild("v3"))
        cookie = oids[4]
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS, oidType, v3.cvisit(), cookie)
        self.assertEqual(4, len(v3.oids))

    def testOidDump(self):
        oid = self.hdr.coids[0]
        flags = 0
        libonlp.onlp_oid_dump(oid, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        self.assertIn("Description:", buf.string_at())

    def testOidTableDump(self):
        tbl = self.hdr.coids
        flags = 0
        libonlp.onlp_oid_table_dump(tbl, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        lines = buf.string_at().splitlines(False)
        lines = [x for x in lines if 'Description' in x]
        self.assert_(len(lines) > 1)

    def testOidShow(self):
        oid = self.hdr.coids[0]
        flags = 0
        libonlp.onlp_oid_show(oid, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        self.assertIn("Description:", buf.string_at())

    def testOidTableShow(self):
        tbl = self.hdr.coids
        flags = 0
        libonlp.onlp_oid_table_show(tbl, self.aim_pvs_buffer_p, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer_p)
        lines = buf.string_at().splitlines(False)
        lines = [x for x in lines if 'Description' in x]
        self.assert_(len(lines) > 1)

if __name__ == "__main__":
    logging.basicConfig()
    unittest.main()

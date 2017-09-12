"""OnlpApiTest.py

Test the API bindings.
"""

import ctypes
import unittest
import logging
import re

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

class SysTest(OnlpTestMixin,
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

    def auditOidHdr(self, hdr):

        self.assertEqual(0, hdr.poid)
        self.assertEqual(0, hdr._id)
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

    def testSysHeaderOnie(self):
        """Test the sys_hdr data that is in the sys_info."""
        self.auditOidHdr(self.sys_info.hdr)

        # test the iteration of the oid table

    def testSysHeader(self):
        """Test the sys_hdr data available via sys_hdr_get."""
        pass

if __name__ == "__main__":
    logging.basicConfig()
    unittest.main()

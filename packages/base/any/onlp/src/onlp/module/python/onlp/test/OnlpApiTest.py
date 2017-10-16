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

import onlp.onlp
import onlp.sff

libonlp = onlp.onlp.libonlp

import onlp.onlp.aim_weakref

def isVirtual():
    with open("/etc/onl/platform") as fd:
        buf = fd.read()
    return "bigswitch" in buf

def skipIfVirtual(reason="this test only works with a physical device"):
    return unittest.skipIf(isVirtual(), reason)

class aim_pvs_buffer(onlp.onlp.aim_weakref.AimReference):

    def __init__(self):
        ptr = libonlp.aim_pvs_buffer_create()
        super(aim_pvs_buffer, self).__init__(ptr)

    @classmethod
    def deleteReference(self, ptr):
        libonlp.aim_pvs_destroy(ptr)

    def string_at(self):
        buf = libonlp.aim_pvs_buffer_get(self.ptr)
        return buf.string_at()

class OnlpTestMixin(object):

    def setUp(self):

        self.log = logging.getLogger("onlp")
        self.log.setLevel(logging.DEBUG)

        self.aim_pvs_buffer = aim_pvs_buffer()

        onlp.onlp.aim_weakref.logger = self.log.getChild("weakref")

    def tearDown(self):
        pass

    def assertStatusOK(self, sts):
        if sts == onlp.onlp.ONLP_STATUS.OK:
            return
        raise AssertionError("invalid ONLP status %s" % onlp.onlp.ONLP_STATUS.name(sts))

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

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testPlatformDump(self):
        """Verify basic platform dump output."""

        flags = 0
        libonlp.onlp_platform_dump(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("System Information:", bufStr)
        self.assertIn("thermal @ 1", bufStr)

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

    def testPlatformShow(self):
        """Verify basic platform show output."""

        flags = 0
        libonlp.onlp_platform_show(self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("System Information:", bufStr)

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
        self.assertLess(len(coids), onlp.onlp.ONLP_OID_TABLE_SIZE)

        def _oidType(oid):
            return onlp.onlp.ONLP_OID_TYPE.name(oid._id>>24)

        for coid in coids:
            self.log.info("oid %d (%s): %s",
                          coid._id, _oidType(coid), coid.description)
            if _oidType(coid) is None:
                raise AssertionError("invalid oid")
            self.assertEqual(hdr._id, coid.poid)

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

        product_re = re.compile("[a-z]*[a-zA-Z0-9_. -]")
        m = product_re.match(self.sys_info.onie_info.product_name)
        if m is None:
            raise AssertionError("invalid product name %s"
                                 % repr(self.sys_info.onie_info.product_name))

        vendor_re = re.compile("[A-Z][a-z]*[a-zA-Z0-9_. -]")
        m = vendor_re.match(self.sys_info.onie_info.vendor)
        if m is None:
            raise AssertionError("invalid vendor %s"
                                 % repr(self.sys_info.onie_info.vendor))

        self.assertIn('.', self.sys_info.onie_info.onie_version)

        # see if there are any vendor extensions
        # if there are any, make sure the are well-formed
        for vx in self.sys_info.onie_info.vx_list:
            sz = vx.size
            self.assertLessEqual(sz, 256)

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

        try:

            subprocess.check_call(('service', 'onlpd', 'stop',))

            code = libonlp.onlp_sys_platform_manage_init()
            self.assertGreaterEqual(code, 0)

            code = libonlp.onlp_sys_platform_manage_start(0)
            self.assertGreaterEqual(code, 0)

            for i in range(10):
                libonlp.onlp_sys_platform_manage_now()
                time.sleep(0.25)

            code = libonlp.onlp_sys_platform_manage_stop(0)
            self.assertGreaterEqual(code, 0)

            time.sleep(2.0)

            code = libonlp.onlp_sys_platform_manage_join(0)
            self.assertGreaterEqual(code, 0)

        finally:
            subprocess.check_call(('service', 'onlpd', 'start',))

    def testSysDump(self):
        """Test the SYS OID debug dump."""

        oid = onlp.onlp.ONLP_OID_SYS
        flags = 0
        libonlp.onlp_sys_dump(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("System Information", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer.ptr)

        # this is not the system OID

        oid = self.sys_info.hdr.coids[0]
        libonlp.onlp_sys_dump(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIsNone(bufStr)

    def testSysShow(self):
        """Test the OID status."""

        oid = onlp.onlp.ONLP_OID_SYS
        flags = 0
        libonlp.onlp_sys_show(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("System Information", bufStr)

        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer.ptr)

        # this is not the system OID

        oid = self.sys_info.hdr.coids[0]
        libonlp.onlp_sys_show(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIsNone(bufStr)

    def testSysIoctl(self):
        """Test the IOCTL interface."""

        # no such ioctl

        code = libonlp.onlp_sys_ioctl(9999)
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, code)

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
                return onlp.onlp.ONLP_STATUS.OK

        oidType = 0
        cookie = 0xdeadbeef

        # gather all OIDs

        v1 = V1(cookie, log=self.log.getChild("v1"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS, oidType, v1.cvisit(), cookie)
        self.assert_(v1.oids)
        oids = list(v1.oids)

        # filter based on OID type

        oidType = onlp.onlp.ONLP_OID_TYPE.PSU
        v1b = V1(cookie, log=self.log.getChild("v1b"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS, oidType, v1b.cvisit(), cookie)
        self.assert_(v1b.oids)
        self.assertLess(len(v1b.oids), len(oids))

        # validate error recovery

        oidType = 0
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
                    return onlp.onlp.ONLP_STATUS.E_GENERIC
                self.log.info("found oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        v3 = V3(log=self.log.getChild("v3"))
        cookie = oids[4]
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS, oidType, v3.cvisit(), cookie)
        self.assertEqual(4, len(v3.oids))

    def testOidDump(self):
        oid = self.hdr.coids[0]
        flags = 0
        libonlp.onlp_oid_dump(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        self.assertIn("Description:", buf.string_at())

    def testOidTableDump(self):
        tbl = self.hdr.coids
        flags = 0
        libonlp.onlp_oid_table_dump(tbl, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        lines = buf.string_at().splitlines(False)
        lines = [x for x in lines if 'Description' in x]
        self.assertGreater(len(lines), 1)

    def testOidShow(self):
        oid = self.hdr.coids[0]
        flags = 0
        libonlp.onlp_oid_show(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        self.assertIn("Description:", buf.string_at())

    def testOidTableShow(self):
        tbl = self.hdr.coids
        flags = 0
        libonlp.onlp_oid_table_show(tbl, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        lines = buf.string_at().splitlines(False)
        lines = [x for x in lines if 'Description' in x]
        self.assertGreater(len(lines), 1)

class FanTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/fan.h."""

    FAN_MODE_VALID = False
    # 'fan mode' configuration is not implemented

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_sys_init()
        libonlp.onlp_fan_init()

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
        self.assert_(fan.model)
        self.assert_(fan.serial)
        self.log.info("auditing fan %d: %s (S/N %s)",
                      oid & 0xFFFFFF,
                      fan.model, fan.serial)

        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.B2F:
            pass
        elif fan.caps & onlp.onlp.ONLP_FAN_CAPS.F2B:
            pass
        else:
            raise AssertionError("invalid fan caps")

        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_RPM:
            self.assertGreater(fan.rpm, 0)
        else:
            self.log.warn("fan does not support RPM get")

        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_PERCENTAGE:
            self.assertGreater(fan.percentage, 0)
            self.assertLessEqual(fan.percentage, 100)
        else:
            self.log.warn("fan does not support PCT get")

        if self.FAN_MODE_VALID:
            self.assertNotEqual(onlp.onlp.ONLP_FAN_MODE.OFF, fan.mode)
            # default, fan should be running

        self.assert_(onlp.onlp.ONLP_FAN_STATUS.PRESENT & fan.status)
        # default, fan should be present

        if fan.status & onlp.onlp.ONLP_FAN_STATUS.B2F:
            self.assert_(onlp.onlp.ONLP_ONLP_FAN_CAPS.B2F)
        elif fan.status & onlp.onlp.ONLP_FAN_STATUS.F2B:
            self.assert_(onlp.onlp.ONLP_FAN_CAPS.F2B)
        else:
            self.log.warn("fan direction not supported")

        # retrieve fan status separately
        sts = ctypes.c_uint()
        libonlp.onlp_fan_status_get(oid, ctypes.byref(sts))
        self.assert_(onlp.onlp.ONLP_FAN_STATUS.PRESENT & sts.value)

        # try to manipulate the fan speed
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.SET_RPM:
            self.auditFanRpm(oid)
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.SET_PERCENTAGE:
            self.auditFanPct(oid)
        if (self.FAN_MODE_VALID
            and (fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_RPM
                 or fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_PERCENTAGE)):
            self.auditFanMode(oid)
        if (fan.caps & onlp.onlp.ONLP_FAN_CAPS.F2B
            and fan.caps & onlp.onlp.ONLP_FAN_CAPS.B2F):
            self.auditFanDir(oid)

        flags = 0
        libonlp.onlp_fan_dump(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("Fan", bufStr)

        libonlp.onlp_fan_show(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("Fan", bufStr)

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
        if self.FAN_MODE_VALID:
            self.assertEqual(fan.mode, onlp.onlp.ONLP_FAN_MODE.MAX)
        minRpm = maxRpm = curRpm = fan.rpm

        speeds = []
        pcts = []
        try:
            subprocess.check_call(('service', 'onlpd', 'stop',))

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
            libonlp.onlp_fan_mode_set(oid, onlp.onlp.ONLP_FAN_MODE.MAX)
            subprocess.check_call(('service', 'onlpd', 'start',))

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
        if self.FAN_MODE_VALID:
            self.assertEqual(fan.mode, onlp.onlp.ONLP_FAN_MODE.MAX)

        speeds = []
        pcts = []
        try:
            subprocess.check_call(('service', 'onlpd', 'stop',))

            libonlp.onlp_fan_percentage_set(oid, 0)
            time.sleep(10.0)
            # initially spin down the fan

            for npct in [0, 33, 66, 100,]:
                self.log.info("setting fan percentage to %d", npct)
                libonlp.onlp_fan_percentage_set(oid, npct)
                time.sleep(5.0)
                libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
                speeds.append(fan.rpm)
                pcts.append(fan.percentage)
        finally:
            libonlp.onlp_fan_percentage_set(oid, 100)
            libonlp.onlp_fan_mode_set(oid, onlp.onlp.ONLP_FAN_MODE.MAX)
            subprocess.check_call(('service', 'onlpd', 'start',))

        # fan speeds should be monotonically increasing
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_RPM:
            self.assertEqual(speeds, sorted(speeds))
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_PERCENTAGE:
            self.assertEqual(pcts, sorted(pcts))

    def auditFanDir(self, oid):
        """Try to adjust the fan direction."""
        unittest.skip("not implemented")

    def auditFanMode(self, oid):
        """Try to adjust the fan speed using the mode specifier."""

        fan = onlp.onlp.onlp_fan_info()
        libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
        self.assertEqual(fan.mode, onlp.onlp.ONLP_FAN_MODE.MAX)

        speeds = []
        pcts = []
        try:
            subprocess.check_call(('service', 'onlpd', 'stop',))
            for nmode in [onlp.onlp.ONLP_FAN_MODE.OFF,
                          onlp.onlp.ONLP_FAN_MODE.SLOW,
                          onlp.onlp.ONLP_FAN_MODE.NORMAL,
                          onlp.onlp.ONLP_FAN_MODE.FAST,
                          onlp.onlp.ONLP_FAN_MODE.MAX,]:
                self.log.info("setting fan mode to %s", onlp.onlp.ONLP_FAN_MODE.name(nmode))
                libonlp.onlp_fan_mode_set(oid, nmode)
                time.sleep(2.0)
                libonlp.onlp_fan_info_get(oid, ctypes.byref(fan))
                speeds.append(fan.rpm)
                pcts.append(fan.percentage)
        finally:
            libonlp.onlp_fan_mode_set(oid, onlp.onlp.ONLP_FAN_MODE.MAX)
            subprocess.check_call(('service', 'onlpd', 'start',))

        # fan speeds should be monotonically increasing
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_RPM:
            self.assertEqual(speeds, sorted(speeds))
            self.assertEqual(0, speeds[0])
            self.assertGreater(105*maxRpm//100, speeds[-1])
        if fan.caps & onlp.onlp.ONLP_FAN_CAPS.GET_PERCENTAGE:
            self.assertEqual(pcts, sorted(pcts))
            self.assertEqual(0, pcts[0])
            self.assertGreater(105, pcts[-1])

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

        v = V(log=self.log.getChild("fan"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS,
                                 onlp.onlp.ONLP_OID_TYPE.FAN,
                                 v.cvisit(), 0)
        self.assert_(v.oids)

        self.auditFanOid(v.oids[0])

class LedTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/led.h.

    XXX roth -- need to flesh this out using a physical device.
    """

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_sys_init()
        libonlp.onlp_led_init()

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

        v = V(log=self.log.getChild("led"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS,
                                 onlp.onlp.ONLP_OID_TYPE.LED,
                                 v.cvisit(), 0)
        self.assert_(v.oids)

        self.auditLedOid(v.oids[0])

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

        self.assert_(led.status & onlp.onlp.ONLP_LED_STATUS.PRESENT)

        # retrieve led status separately
        sts = ctypes.c_uint()
        libonlp.onlp_led_status_get(oid, ctypes.byref(sts))
        self.assert_(onlp.onlp.ONLP_LED_STATUS.PRESENT & sts.value)

        try:
            subprocess.check_call(('service', 'onlpd', 'stop',))

            if led.caps & onlp.onlp.ONLP_LED_CAPS.CHAR:
                self.auditLedChar(oid)
            if (led.caps & onlp.onlp.ONLP_LED_CAPS.ON_OFF
                and not self.hasColors(led.caps)):
                self.auditLedOnOff(oid)
            self.auditLedColors(oid)
            self.auditLedBlink(oid)

        finally:
            subprocess.check_call(('service', 'onlpd', 'start',))


        flags = 0
        libonlp.onlp_led_dump(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("led @", bufStr)

        libonlp.onlp_led_show(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("led @", bufStr)

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
        saveChar = led.char

        try:
            for nchar in ['0', '1', '2', '3',]:
                self.log.info("led %d: char '%s'", oid, nchar)

                sts = libonlp.onlp_led_char_set(oid, nchar)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(nchar, led.char)
        finally:
            libonlp.onlp_led_char_set(oid, saveChar)

    def auditLedOnOff(self, oid):

        led = onlp.onlp.onlp_led_info()
        libonlp.onlp_led_info_get(oid, ctypes.byref(led))
        saveMode = led.mode

        if saveMode == onlp.onlp.ONLP_LED_MODE.OFF:
            pass
        elif saveMode == onlp.onlp.ONLP_LED_MODE.ON:
            pass
        else:
            self.log.warn("invalid LED on/off mode %s",
                          onlp.onlp.ONLP_LED_MODE.name(saveMode))

        try:
            for i in range(4):
                self.log.info("led %d: on", oid)

                sts = libonlp.onlp_led_set(oid, 1)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(onlp.onlp.ONLP_LED_MODE.ON, led.mode)

                sts = libonlp.onlp_led_get(oid, 0)
                self.assertStatusOK(sts)

                time.sleep(1.0)

                libonlp.onlp_led_info_get(oid, ctypes.byref(led))
                self.assertEqual(onlp.onlp.ONLP_LED_MODE.OFF, led.mode)

        finally:
            if saveMode == onlp.onlp.ONLP_LED_MODE.OFF:
                libonlp.onlp_led_set(oid, 0)
            else:
                libonlp.onlp_led_set(oid, 1)

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

        libonlp.onlp_thermal_init()

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

        v = V(log=self.log.getChild("thermal"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS,
                                 onlp.onlp.ONLP_OID_TYPE.THERMAL,
                                 v.cvisit(), 0)
        self.assert_(v.oids)

        self.auditThermalOid(v.oids[0])

    def auditThermalOid(self, oid):

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_thermal_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(oid, hdr._id)

        thm = onlp.onlp.onlp_thermal_info()
        libonlp.onlp_thermal_info_get(oid, ctypes.byref(thm))

        self.assertEqual(oid, thm.hdr._id)

        self.assert_(thm.caps)
        # should support some non-empty set of capabilities

        self.assert_(thm.caps & onlp.onlp.ONLP_THERMAL_CAPS.GET_TEMPERATURE)
        # sensor should at least report temperature

        self.log.info("auditing thermal %d",
                      oid & 0xFFFFFF)

        self.assert_(thm.status & onlp.onlp.ONLP_THERMAL_STATUS.PRESENT)
        # sensor should be present

        self.assertGreater(thm.mcelcius, 20000)
        self.assertLess(thm.mcelcius, 35000)
        # temperature should be non-crazy

        # retrieve thermal status separately
        sts = ctypes.c_uint()
        libonlp.onlp_thermal_status_get(oid, ctypes.byref(sts))
        self.assert_(onlp.onlp.ONLP_THERMAL_STATUS.PRESENT & sts.value)

        # test ioctl
        code = libonlp.onlp_thermal_ioctl(9999)
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, code)

        flags = 0
        libonlp.onlp_thermal_dump(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("thermal @", bufStr)

        libonlp.onlp_thermal_show(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("thermal @", bufStr)

class PsuTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/psu.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_psu_init()

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

    def testFindPsu(self):

        class V(OidIterator):

            def __init__(self, log):
                super(V, self).__init__(log)
                self.oids = []

            def visit(self, oid, cookie):
                self.log.info("found psu oid %d", oid)
                self.oids.append(oid)
                return onlp.onlp.ONLP_STATUS.OK

        v = V(log=self.log.getChild("psu"))
        libonlp.onlp_oid_iterate(onlp.onlp.ONLP_OID_SYS,
                                 onlp.onlp.ONLP_OID_TYPE.PSU,
                                 v.cvisit(), 0)
        self.assert_(v.oids)

        self.auditPsuOid(v.oids[0])

    def auditPsuOid(self, oid):

        hdr = onlp.onlp.onlp_oid_hdr()
        libonlp.onlp_psu_hdr_get(oid, ctypes.byref(hdr))
        self.assertEqual(oid, hdr._id)

        psu = onlp.onlp.onlp_psu_info()
        libonlp.onlp_psu_info_get(oid, ctypes.byref(psu))

        self.assertEqual(oid, psu.hdr._id)

        self.assert_(psu.caps
                     & (onlp.onlp.ONLP_PSU_CAPS.AC
                        | onlp.onlp.ONLP_PSU_CAPS.DC12
                        | onlp.onlp.ONLP_PSU_CAPS.DC48))
        # should support some non-empty set of capabilities

        self.log.info("auditing psu %d",
                      oid & 0xFFFFFF)

        self.assert_(psu.status & onlp.onlp.ONLP_PSU_STATUS.PRESENT)
        # sensor should be present

        if (psu.caps
            & onlp.onlp.ONLP_PSU_CAPS.AC
            & onlp.onlp.ONLP_PSU_CAPS.VOUT):
            self.assertGreater(psu.mvout, 100000)
            self.assertLess(psu.mvout, 125000)
        if (psu.caps
            & onlp.onlp.ONLP_PSU_CAPS.DC12
            & onlp.onlp.ONLP_PSU_CAPS.VOUT):
            self.assertGreater(psu.mvout, 11000)
            self.assertLess(psu.mvout, 13000)
        if (psu.caps
            & onlp.onlp.ONLP_PSU_CAPS.DC48
            & onlp.onlp.ONLP_PSU_CAPS.VOUT):
            self.assertGreater(psu.mvout, 47000)
            self.assertLess(psu.mvout, 49000)
        # output voltage should be non-crazy

        # retrieve psu status separately
        sts = ctypes.c_uint()
        libonlp.onlp_psu_status_get(oid, ctypes.byref(sts))
        self.assert_(onlp.onlp.ONLP_PSU_STATUS.PRESENT & sts.value)

        # test ioctl
        code = libonlp.onlp_psu_ioctl(9999)
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, code)

        flags = 0
        libonlp.onlp_psu_dump(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("psu @", bufStr)

        libonlp.onlp_psu_show(oid, self.aim_pvs_buffer.ptr, flags)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        bufStr = buf.string_at()
        self.assertIn("psu @", bufStr)

class Eeprom(ctypes.Structure):
    _fields_ = [('eeprom', ctypes.c_ubyte * 256,),]

class SfpTest(OnlpTestMixin,
              unittest.TestCase):
    """Test interfaces in onlp/sfp.h."""

    def setUp(self):
        OnlpTestMixin.setUp(self)

        libonlp.onlp_sfp_init()

        self.bitmap = onlp.onlp.aim_bitmap256()

    def tearDown(self):
        OnlpTestMixin.tearDown(self)

        libonlp.onlp_sfp_denit()

    def bitmap2list(self, bitmap=None):
        outBits = []
        bitmap = bitmap or self.bitmap
        for pos in range(256):
            outBits.append(onlp.onlp.aim_bitmap_get(bitmap.hdr, pos))
        return outBits

    def testBitmap(self):
        """Verify that our aim_bitmap implementation is sound."""

        refBits = []
        for pos in range(256):
            val = random.randint(0, 1)
            refBits.append(val)
            onlp.onlp.aim_bitmap_mod(self.bitmap.hdr, pos, val)

        for i in range(1000):
            pos = random.randint(0, 255)
            val = refBits[pos] ^ 1
            refBits[pos] = val
            onlp.onlp.aim_bitmap_mod(self.bitmap.hdr, pos, val)

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
            valid = libonlp.onlp_sfp_port_valid(i)
            if i < len(ports):
                self.assertEqual(1, valid)
            else:
                self.assertEqual(0, valid)

        # see if any of them are present
        # XXX this test requires at least one of the SFPs to be present.
        bm = onlp.onlp.aim_bitmap256()
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

        port = ports[0]

        self.auditIoctl(port)
        self.auditControl(port)

        eeprom = ctypes.POINTER(ctypes.c_ubyte)()
        sts = libonlp.onlp_sfp_eeprom_read(port, ctypes.byref(eeprom))
        self.assertStatusOK(sts)

        try:

            # try to read in the data manually
            for i in range(128):
                b = libonlp.onlp_sfp_dev_readb(port, 0x50, i)
                if b != eeprom[i]:
                    raise AssertionError("eeprom mismatch at 0x50.%d" % i)

            monType = eeprom[92] & 0x40
            # See e.g. https://www.optcore.net/wp-content/uploads/2017/04/SFF_8472.pdf

            self.auditEeprom(eeprom)

        finally:
            ptr = onlp.onlp.aim_void_p(ctypes.cast(eeprom, ctypes.c_void_p).value)
            del ptr

        if monType:

            domData = ctypes.POINTER(ctypes.c_ubyte)()
            sts = libonlp.onlp_sfp_dom_read(port, ctypes.byref(domData))
            self.assertStatusOK(sts)

            try:
                self.auditDom(domData)
            finally:
                ptr = onlp.onlp.aim_void_p(ctypes.cast(domData, ctypes.c_void_p).value)
                del ptr

    def auditEeprom(self, eeprom):
        """Audit that the entries for this SFP are valid."""

        sffEeprom = onlp.sff.sff_eeprom()
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
        cl = []
        for i in range(32):
            fl = 1<<i
            if caps.value & fl:
               cl.append(onlp.sff.SFF_MODULE_CAPS.name(fl))
        self.log.info("module caps %s", "+".join(cl))

        libonlp.sff_info_show(ctypes.byref(sffEeprom.info), self.aim_pvs_buffer.ptr)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        self.assertIn("Vendor:", buf.string_at())

        # cons up a new info structure
        # XXX includes space padding

        info = onlp.sff.sff_info()
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

            sffEeprom2 = onlp.sff.sff_eeprom()
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

    def auditDom(self, domData):
        unittest.skip("not implemented")

    def testDump(self):
        unittest.skip("this is a really slow command")
        return
        libonlp.aim_pvs_buffer_reset(self.aim_pvs_buffer.ptr)
        libonlp.onlp_sfp_dump(self.aim_pvs_buffer.ptr)
        buf = libonlp.aim_pvs_buffer_get(self.aim_pvs_buffer.ptr)
        self.assertIn("Presence Bitmap", buf.string_at())

    def auditIoctl(self, port):

        sts = libonlp.onlp_sfp_ioctl(port, 999)
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, sts)

        sts = libonlp.onlp_sfp_ioctl(127, 999)
        self.assertEqual(onlp.onlp.ONLP_STATUS.E_UNSUPPORTED, sts)

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

if __name__ == "__main__":
    logging.basicConfig()
    unittest.main()

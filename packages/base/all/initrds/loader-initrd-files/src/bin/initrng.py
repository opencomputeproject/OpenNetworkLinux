#!/usr/bin/env python

"""
Distributed under MIT License.
See LICENSE file for full license text.
"""

import fcntl
import struct
import sys
import os
import hashlib
import logging
import argparse

RNDADDENTROPY = 0x40085203 # from linux/random.h

def sha512sum(digest, fileName, blockSize = 16 * 1024):
    """
    Take SHA512 digest from file given by it's name contents.

    File opened in os.O_NONBLOCK mode to support FIFOs and
    special files like /dev/kmsg. Read data from files in
    blockSize (default 16kB) chunks to keep memory usage at
    minimum when used with large files.

    Return True on success and False otherwise.
    """

    def read():
        try:
            return fp.read(blockSize)
        except IOError:
            return b''
    #end def

    if digest is None:
        return False

    try:
        fp = open(fileName, 'rb')
    except IOError:
        return False

    flags = fcntl.fcntl(fp, fcntl.F_GETFL) | os.O_NONBLOCK
    if fcntl.fcntl(fp, fcntl.F_SETFL, flags):
        fp.close()
        return False

    for block in iter(read, b''):
        digest.update(block)

    fp.close()
    return True

def sha512(digest, fileName):
    """
    Computes sha512sum() optionally logging status.

    For internal use only.

    Return 1 on success and 0 on failure.
    """

    rc = sha512sum(digest, fileName)
    logging.debug("SHA512 (%s) = %s", fileName, 'ok' if rc else 'fail')
    return int(rc)

def add_entropy(digest, fileName = "/dev/urandom"):
    """
    Adds digest.digest_size * 8 bits of entropy using ioctl(RNDADDENTROPY, ...)
    to increase entropy count if fileName is a character special file node and
    CAP_SYS_ADMIN is available (otherwise IOError is raised by fnctl.fnctl()).

    Writes entropy to fileName if IOError exception is raised by fnctl.fnctl()
    or fileName is a regular file. Update entropy pool without incrementing
    entropy count if fileName is special character device like "/dev/urandom".
    """

    try:
        fp = open(fileName, 'ab')
    except IOError as e:
        return None, str(e)

    size = digest.digest_size
    digest = digest.digest()

    method = None
    err = None

    do_ioctl = not os.path.isfile(fileName)

    try:
        if do_ioctl:
            # from random(4):
            #
            #   struct rand_pool_info {
            #           int    entropy_count;
            #           int    buf_size;
            #           __u32  buf[0];
            #   };
            #
            fmt = "ii{:d}s".format(size)
            rand_pool_info = struct.pack(fmt, size * 8, size, digest)
            fcntl.ioctl(fp, RNDADDENTROPY, rand_pool_info)
        else:
            raise IOError()
    except IOError:
        try:
            fp.write(digest)
        except IOError as e:
            err = str(e)
        else:
            method = "write"
    else:
        method = "ioctl"

    return method, err

def init():
    prog_name = os.path.splitext(os.path.basename(__file__))[0]
    if not prog_name:
        prog_name = "initrng"

    parser = argparse.ArgumentParser(description = 'Linux RNG early init')

    # loglevel
    loglevels = {
        'crit' : 'CRITICAL',
        'err'  : 'ERROR',
        'warn' : 'WARNING',
        'info' : 'INFO',
        'debug': 'DEBUG',
    }
    parser.add_argument('-l', '--loglevel', default = 'info',
                        choices = list(loglevels.keys()),
                        help = 'set program loging severity (level)')

    # entropy_files
    dflt_entropy_files = [
        "/proc/timer_list",
        "/proc/buddyinfo",
        "/proc/interrupts",
        "/proc/softirqs",
    ]

    sched_debug = "/proc/sched_debug"
    if os.path.isfile(sched_debug):
        dflt_entropy_files.append(sched_debug)
    else:
        dflt_entropy_files.append("/proc/schedstat")

    parser.add_argument('-e', '--entropy-file', default = [],
                        action = 'append', dest = 'entropy_files', type = str,
                        help = 'files to use as source of entropy ({:s})'.format(', '.join(dflt_entropy_files)))

    # repeat
    dflt_repeat = 8
    parser.add_argument('-r', '--repeat', default = dflt_repeat,
                        action = 'store', type = int,
                        help = 'repeat entropy updates # times (default {:d})'.format(dflt_repeat))

    # output
    dflt_output = "/dev/urandom"
    parser.add_argument('-o', '--output', default = dflt_output,
                        action = 'store', type = str,
                        help = 'file to output entropy (default "{:s}")'.format(dflt_output))

    args = parser.parse_args()

    logging.basicConfig(format = "{:s}: %(message)s".format(prog_name),
                        level = getattr(logging, loglevels[args.loglevel]))

    # adjust entropy_files
    if not args.entropy_files:
        args.entropy_files = dflt_entropy_files
        logging.debug("using default list of files as entropy source")

    # adjust repeat
    repeat = args.repeat

    if repeat <= 0:
        repeat = 1
    elif repeat > 65536:
        repeat = 65536

    if repeat != args.repeat:
        logging.debug("adjust repeat count from %d to %d", args.repeat, repeat)
        args.repeat = repeat

    # truncate entropy file
    output = args.output
    try:
        with open(output, 'wb'):
            pass
    except IOError:
        logging.debug("entropy file '%s' isn't writable", output)
        args = None
    else:
        logging.debug("entropy file '%s' is writable", output)

    return args

if __name__ == '__main__':
    args = init()

    if args is None:
        logging.debug("fail to adjust/validate args")
        sys.exit(1)

    logging.info("Linux Random Number Generator (RNG) early init")

    entropy_files = args.entropy_files
    repeat = args.repeat

    for x in range(1, repeat + 1):
        digest_sha512 = hashlib.sha512()

        i = 0
        for f in entropy_files:
            i += sha512(digest_sha512, f)
        if not i:
            logging.debug("digest error for all entropy files, step %d", x)
            continue

        method, err = add_entropy(digest_sha512, args.output)
        if err:
            logging.debug("error seeding Linux RNG, step %d: %s", x, err)
        else:
            logging.debug("seeded Linux RNG, method '%s', step %d", method, x)

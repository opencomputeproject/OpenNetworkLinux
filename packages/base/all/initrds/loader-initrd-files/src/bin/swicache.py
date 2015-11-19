#!/usr/bin/python
import argparse
import os
import sys
import hashlib
import shutil
import logging

logging.basicConfig()
logger = logging.getLogger("swicache")
logger.setLevel(logging.INFO)

def filehash(fname, blocksize=1024*1024):
   h = hashlib.sha1()
   with open(fname,'rb') as f:
       block = 0
       while block != b'':
           block = f.read(blocksize)
           h.update(block)
   return h.hexdigest()

def write_hash(fname, digest):
    open(fname, "w").write(digest)

def read_hash(fname):
    return open(fname).read()

ap = argparse.ArgumentParser(description="SWI Cacher")
ap.add_argument("src")
ap.add_argument("dst")
ap.add_argument("--force", action='store_true')

ops = ap.parse_args()

# Generate hash of the source file
logger.info("Generating hash for %s..." % ops.src)
src_hash = filehash(ops.src)
logger.info("Generated hash for %s: %s" % (ops.src, src_hash))

dst_hash_file = "%s.md5sum" % ops.dst

if not ops.force:
    if os.path.exists(ops.dst) and os.path.exists(dst_hash_file):
        # Destination exists and the hash file exists.
        dst_hash = read_hash(dst_hash_file)
        if dst_hash == src_hash:
           # Src and destination are the same.
           logger.info("Cache file is up to date.")
           sys.exit(0)

#
# Either force==True, a destination file is missing, or the
# current file is out of date.
#
logger.info("Updating %s --> %s, %s" % (ops.src, ops.dst, src_hash))
if not os.path.isdir(os.path.dirname(ops.dst)):
   os.makedirs(os.path.dirname(ops.dst))
shutil.copyfile(ops.src, ops.dst)
write_hash(dst_hash_file, src_hash);
logger.info("Syncing...")
os.system("sync")
logger.info("Done.")




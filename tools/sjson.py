#!/usr/bin/python2
############################################################
#
# Simple JSON Generator
#
############################################################
import argparse
import yaml
import json
import os
import sys


def setkeypath(d, kvt):
    """Set a key tree to a value in the given dict.

    The key tree is specified as k1.k2..kN-1.kN"""

    (key, value) = kvt
    ktree = key.split('.')
    for k in ktree:
        if k == ktree[-1]:
            d[k] = value
            return
        elif k not in d:
            d[k] = {}

        d = d[k]


ap=argparse.ArgumentParser(description="Simple JSON Generator.")

ap.add_argument("--in",             metavar='FILENAME',          help="Load json source data.", dest='_in')
ap.add_argument("--kj",  nargs=2,   metavar=('KEY', 'FILE|STR'), help="Add json data.")
ap.add_argument("--ky",  nargs=2,   metavar=('KEY', 'FILE|STR'), help="Add yaml jdata.")
ap.add_argument("--kv",  nargs=2,   metavar=('KEY', 'VALUE'),    help="Add key/value pair.")
ap.add_argument("--kl",  nargs='+', metavar=('KEY', 'ENTRY'),    help="Add key/list pair.")
ap.add_argument("--out",            metavar='FILENAME',          help="Write output to the given file. The default is stdout")
ap.add_argument("--indent", nargs=1,                             help="Json output indentation value. Default is 2", default=2)
ap.add_argument("--no-nl", action='store_true',                  help="No newline at the end of the output.")
ap.add_argument("--inout",          metavar='FILENAME',          help="Modify. Equivalent to --in FILENAME --out FILENAME")
ops = ap.parse_args();

if ops.inout:
    ops._in = ops.inout
    ops.out = ops.inout

g_data={}

if ops._in:
    try:
        g_data = yaml.load(open(ops._in))
    except:
        g_data = json.load(open(ops._in))

if ops.kj:
    (k, j) = ops.kj
    if os.path.exists(j):
        v = json.load(open(j))
    else:
        v = json.loads(j)
    setkeypath(g_data, (k, v))

if ops.ky:
    (k, y) = ops.ky
    if os.path.exists(y):
        v = yaml.load(open(y))
    else:
        v = yaml.load(y)
    setkeypath(g_data, (k, v))

if ops.kv:
    setkeypath(g_data, ops.kv)

if ops.kl:
    k = ops.kl.pop(0)
    setkeypath(g_data, (k, ops.kl))

out=sys.stdout
if ops.out and ops.out not in ['-', 'stdout']:
    print ops.out
    out = open(ops.out, "w")

json.dump(g_data, out, indent=ops.indent)
if not ops.no_nl:
    out.write('\n')

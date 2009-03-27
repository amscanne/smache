#!/usr/bin/env python

import sys
import native

#
# The Smache class.
#
# Currently, it only supports retrieving files and putting files.  Backends are
# added and tweaked exclusively through their respectively backend classes.
#

class Smache:
    def __init__(self):
        self.sm = native.smache_create()

    def __del__(self):
        native.smache_destroy(self.sm)

    def debug(self, state):
        native.smache_setdebug(self.sm, int(state))

    def getfile(self, hash, filename):
        nativehash = native.smache_create_hash()
        native.smache_parsehash(nativehash, hash)
        native.smache_getfile(self.sm, nativehash, filename)
        native.smache_delete_hash(nativehash)
        sys.stderr.write("extracted %s -> %s\n" % (hash, filename))

    def addfile(self, filename):
        nativehash = native.smache_create_hash()
        native.smache_putfile(self.sm, nativehash, filename, native.SMACHE_FIXED, native.SMACHE_NONE)
        hash = native.smache_create_hashstr(nativehash)
        hashcopy = hash[:]
        native.smache_delete_hashstr(hash)
        sys.stderr.write("added %s <- %s\n" % (hashcopy, filename))
        return hashcopy

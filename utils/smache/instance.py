#!/usr/bin/env python

import native

class Smache:
    def __init__(self):
        self.sm = native.smache_create()

    def __del__(self):
        native.smache_destroy(self.sm)

    def getfile(self, hash, filename):
        nativehash = native.smache_create_hash()
        native.smache_parsehash(nativehash, hash)
        native.smache_getfile(self.sm, nativehash, filename)
        native.smache_delete_hash(nativehash)
        print "extracted %s -> %s" % (hash, filename)

    def addfile(self, filename):
        nativehash = native.smache_create_hash()
        native.smache_putfile(self.sm, nativehash, filename, native.SMACHE_FIXED, native.SMACHE_NONE)
        hash = native.smache_create_hashstr(nativehash)
        hashcopy = hash[:]
        native.smache_delete_hashstr(hash)
        print "added %s <- %s" % (hashcopy, filename)
        return hashcopy

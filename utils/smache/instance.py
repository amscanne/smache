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
    def __init__(self, config):
        self.sm = native.smache_create()
        self.nh = native.smache_create_hash()

        # Set the debug appropriately.
        self.setdebug(config.smache.get("debug", False))
        self.setprogress(config.smache.get("progress", False))
        self.setcompression(config.smache.get("compression", False))
        self.setblockalgo(config.smache.get("blockalgo", False))
        self.setblocksize(int(config.smache.get("blocksize", 512)))

        # Create all the necessary backends.
        for (btype, options) in config.backends:
            backend = btype(self.sm, options)

    def __del__(self):
        native.smache_destroy(self.sm)
        native.smache_delete_hash(self.nh)

    #
    # Options.
    #
    def setdebug(self, state):
        native.smache_setdebug(self.sm, int(state))
    def setprogress(self, progress):
        native.smache_setprogress(self.sm, int(progress))
    def setblockalgo(self, block):
        native.smache_setblockalgorithm(self.sm, int(block))
    def setblocksize(self, block):
        self.blocksize = block
    def setcompression(self, compression):
        native.smache_setcompressiontype(self.sm, int(compression))

    #
    # Utility functions.  In python, we currently only support
    # getting/putting complete files.  This may change eventually,
    # but for now it keeps the semantics very simple.
    #

    def getfile(self, hash, filename):
        native.smache_parsehash(self.nh, hash)
        native.smache_getfile(self.sm, self.nh, filename)
        sys.stderr.write("extracted %s -> %s\n" % (hash, filename))

    def addfile(self, filename):
        native.smache_putfile(self.sm, self.nh, filename, self.blocksize)
        hash = native.smache_temp_hashstr(self.nh)
        hashcopy = hash[:]
        sys.stderr.write("added %s <- %s\n" % (hashcopy, filename))
        return hashcopy

    def length(self, hash):
        native.smache_parsehash(self.nh, hash)
        return native.smache_info_length(self.sm, self.nh)

    def totallength(self, hash):
        native.smache_parsehash(self.nh, hash)
        return native.smache_info_totallength(self.sm, self.nh)

    def references(self, hash):
        native.smache_parsehash(self.nh, hash)
        return native.smache_info_references(self.sm, self.nh)

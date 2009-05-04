#!/usr/bin/env python

import sys
import instance
import os
import stat
import bsddb
import binascii

class Stats:
    def __init__(self, instance, index, config):
        self.instance = instance
        self.index    = index
        self.config   = config

        self.hashes     = {}
        self.metahashes = {}
        self.filesizes  = 0
        self.dbsizes    = 0

        self.buildstats() 

    def buildstats(self):
        #
        # Build the sum of all files.
        #
        hashes = self.index.hashes()
        for h in hashes:
            self.filesizes += self.instance.totallength(h)

        #
        # Compute the total of all db files.
        #
        for (btype, options) in self.config.backends:
            if options.has_key("filename"):
                self.dbsizes += os.path.getsize(options["filename"])
                self.builddbstats(options["filename"])

    def builddbstats(self, filename):
        db = bsddb.btopen(filename)
        hashes = db.keys()
        for h in hashes:
            h  = binascii.hexlify(h)
            tl = self.instance.totallength(h)
            l  = self.instance.length(h)
            r  = self.instance.references(h)
            if tl == l:
                target = self.hashes
            else:
                target = self.metahashes

            if target.has_key(h):
                target[h][1] = max(r, target[h][2])
            else:
                target[h] = (l, r)

    def keycount(self):
        return len(self.hashes)

    def keyoverhead(self):
        return 4 * (len(self.hashes) + len(self.metahashes))

    def hashoverhead(self):
        return sum(map(lambda x: x[0], self.metahashes.values()))

    def totalsize(self):
        return self.dbsizes

    def datasize(self):
        return sum(map(lambda x: x[0], self.hashes.values()))

    def origsize(self):
        return self.filesizes

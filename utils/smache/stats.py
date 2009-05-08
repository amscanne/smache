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
        db = bsddb.hashopen(filename)
        try:
            h = db.first()[0]
        except:
            return

        while True:
            ah  = binascii.hexlify(h)
            tl = self.instance.totallength(ah)
            l  = self.instance.length(ah)
            r  = self.instance.references(ah)
            mh = self.instance.metahash(ah)

            if mh:
                target = self.metahashes
            else:
                target = self.hashes

            if target.has_key(ah):
                target[ah][2] = max(r, target[ah][2])
            else:
                # Store index as (data length, compressed length, reference count)
                target[ah] = (l, len(db[h]), r)

            try:
                h = db.next()[0]
            except:
                break

    def keycount(self):
        return len(self.hashes)

    def keyoverhead(self):
        return 4 * (len(self.hashes) + len(self.metahashes))

    def hashoverhead(self):
        return sum(map(lambda x: x[0], self.metahashes.values()))

    def totalsize(self):
        return self.dbsizes

    def compressed_datasize(self):
        return sum(map(lambda x: x[1], self.hashes.values()))

    def uncompressed_datasize(self):
        return sum(map(lambda x: x[0], self.hashes.values()))

    def percent_compressed(self):
        return float(sum(map(lambda x: x[1] < x[0], self.hashes.values()))) / (len(self.hashes) or 1)

    def total_compression_ratio(self):
        return float(self.compressed_datasize()) / (self.uncompressed_datasize() or 1)

    def getdatahashes(self):
        return self.hashes.values()

    def getmetahashes(self):
        return self.metahashes.values()

    def origsize(self):
        return self.filesizes

    def dump(self):
        totalsize = self.totalsize()
        origsize  = self.origsize()
        datasizec = self.compressed_datasize()
        datasizeu = self.uncompressed_datasize()
        keycount  = self.keycount()
        keyover   = self.keyoverhead()
        hashover  = self.hashoverhead()
        otherover = (totalsize - datasizec) - (keyover + hashover)

        print "Original size:             %10d" % origsize
        print "SMACHE size:               %10d" % totalsize
        print " Keys:                     %10d" % keycount
        print " Data size (compressed):   %10d" % datasizec
        print " Data size (uncompressed): %10d" % datasizeu
        print " Key overhead:             %10d" % keyover
        print " Hash overhead:            %10d" % hashover
        print " Other overhead:           %10d" % otherover


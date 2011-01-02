#!/usr/bin/env python

import sys
import time
import threading

class ClusterManager(threading.Thread):
    r      = None
    w      = None
    n      = None
    seeds  = []
    stores = []
    smache = None

    def __init__(self, smache, r, w, n, seeds, stores):
        threading.Thread.__init__(self)
        self.daemon = True
        self.r = r
        self.w = w
        self.n = n
        self.seeds  = seeds  
        self.stores = stores
        self.smache = smache

    def buckets(self):
        rval = []
        for s in self.stores:
            for b in s.buckets.all():
                rval.append(b) 
        return rval

    def hosts(self):
        return []

    def run(self):
        sys.stderr.write("Cluster manager running...\n")
        sys.stderr.write(" r = %d, w = %d, n = %d\n" % (self.r, self.w, self.n))
        sys.stderr.write(" seeds = %s\n" % str(self.seeds))
        sys.stderr.write(" stores = %s\n" % str(self.stores))
        while True:
            time.sleep(1.0)

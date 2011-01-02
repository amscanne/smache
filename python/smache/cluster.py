#!/usr/bin/env python

import sys
import time
import threading

from log import log

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
        log("Cluster manager running...")
        log(" r = %d, w = %d, n = %d" % (self.r, self.w, self.n))
        log(" seeds = %s" % str(self.seeds))
        log(" stores = %s" % str(self.stores))
        while True:
            time.sleep(1.0)

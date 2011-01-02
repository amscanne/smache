#!/usr/bin/env python

import sys
import time
import threading
import time

from log import log

class Host:
    address   = None
    port      = None
    timestamp = None

    def __init__(self, address, port):
        self.address = address
        self.port    = port

    def update(self, buckets):

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

    def save(self, o):
        pass

    def load(self, i):
        pass

    def listAll(self):
        pass

    def listData(self):
        pass

    def listMeta(self):
        pass

    def listIndices(self):
        pass

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

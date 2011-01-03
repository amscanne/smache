#!/usr/bin/env python

import sys
import time
import threading
import time

import buckets
from log import log

class Host:
    address   = None
    port      = None
    timestamp = time.time()

    def __init__(self, url):
        (self.address, self.port) = url.split(":", 1)

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "%s:%d" % (self.address, self.port)

class ClusterManager(threading.Thread):
    r      = None
    w      = None
    n      = None
    hosts  = []
    stores = []
    smache = None

    def __init__(self, smache, r, w, n, seeds, stores):
        threading.Thread.__init__(self)
        self.daemon = True
        self.r = r
        self.w = w
        self.n = n
        self.hsots = map(Host, seeds)
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

    def run(self):
        log("Cluster manager running...")
        log(" r = %d, w = %d, n = %d" % (self.r, self.w, self.n))
        log(" hosts = %s" % str(self.hosts))
        log(" stores = %s" % str(self.stores))
        while True:
            time.sleep(1.0)

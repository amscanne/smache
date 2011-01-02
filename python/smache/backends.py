#!/usr/bin/env python

import os
import random

import native
from chunks import *
from exceptions import *
from constants import *

class BucketFile:
    filename = None
    buckets = []

    def __init__(self, directory):
        self.filename = os.path.join(directory, 'buckets')
        self.load()
        if len(self.buckets) == 0:
            self.resize(DEFAULT_BUCKETS)

    def load(self):
        f = open(self.filename, 'w+')
        for l in f.readlines():
            buckets.append(l.rstrip())
        f.close()

    def save(self):
        f = open(self.filename, 'w+')
        for b in self.buckets:
            f.write(b + "\n")
        f.close()

    def resize(self, n):
        while len(self.buckets) > n: 
            del self.buckets[random.randint(0, len(self.buckets)-1)]
        while len(self.buckets) < n: 
            self.buckets.append(str(Hash()))
        self.save()

    def add(self, bucket):
        self.buckets.append(bucket)
        self.save() 

    def remove(self, bucket):
        self.buckets.remove(bucket)
        self.save()

    def all(self):
        return self.buckets[:]        

class Store(native.Store):
    directory = None
    buckets = None

    def __init__(self, directory):
        native.Store.__init__(self, directory)
        self.directory = directory
        self.buckets = BucketFile(directory)

    def get(self, h):
        return native.Store.get(self, h)

    def head(self, h):
        return native.Store.head(self, h)

    def add(self, c):
        return native.Store.add(self, h)

    def remove(self, h):
        return native.Store.remove(self, h)

    def adjrefs(self, h, d):
        return native.Store.adjrefs(self, h, d)

    def fetchAll(self):
        return map(Hash, native.Store.fetchAll(self))

    def countAll(self):
        return native.Store.countAll(self)

    def fetchData(self):
        return map(Hash, native.Store.fetchData(self))

    def countData(self):
        return native.Store.countData(self)

    def fetchIndex(self):
        return map(Hash, native.Store.fetchIndex(self))

    def countIndex(self):
        return native.Store.countIndex(self)

    def fetchMeta(self):
        return map(Hash, native.Store.fetchMeta(self))

    def countMeta(self):
        return native.Store.countMeta(self)

    def __str__(self):
        return "%s[buckets=%d]" % (self.directory, len(self.buckets.all()))

class Remote(native.Remote):
    directory = None
    buckets = None

    def __init__(self, directory):
        native.Store.__init__(self, directory)

    def __init__(self, address):
        native.Remote(self)
        self.address = address
        self.buckets = BucketFile(directory)

    #
    # The following are over-riden and called from the
    # native side in order to provide an implementation
    # for the real calls.  This means that the C-code 
    # will call out to python land to get work done.
    # This is done for the case of remote objects, where
    # the convenience of python massively outweights
    # any performance trade-offs.
    #

    def _get(self, h):
        return self.get(Hash(h))

    def _head(self, h):
        return self.head(Hash(h))

    def _add(self, c):
        return self.add(Chunk(c))

    def _remove(self, h):
        return self.remove(Hash(h))

    def _adjrefs(self, h, d):
        return self.adjrefs(Hash(h), d)

    def _fetchAll(self):
        return self.fetchAll()

    def _countAll(self):
        return self.countAll()

    def _fetchData(self):
        return self.fetchData()

    def _countData(self):
        return self.countData()

    def _fetchIndex(self):
        return self.fetchIndex()

    def _countIndex(self):
        return self.countIndex()

    def _fetchMeta(self):
        return self.fetchMeta()

    def _countMeta(self):
        return self.countMeta()

    def get(self, h):
        return Chunk()

    def head(self, h):
        return Chunk()

    def add(self, c):
        return False

    def remove(self, h):
        return False

    def adjrefs(self, h, d):
        return False

    def fetchAll(self):
        return []

    def countAll(self):
        return 0

    def fetchData(self):
        return []

    def countData(self):
        return 0

    def fetchIndex(self):
        return []

    def countIndex(self):
        return 0

    def fetchMeta(self):
        return []

    def countMeta(self):
        return 0

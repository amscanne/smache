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
        return native.Store.add(self, c)

    def remove(self, h):
        return native.Store.remove(self, h)

    def adjrefs(self, h, d):
        return native.Store.adjrefs(self, h, d)

    def fetchAll(self, s, c):
        return map(Hash, native.Store.fetchAll(self, s, c))

    def countAll(self):
        return native.Store.countAll(self)

    def fetchData(self, s, c):
        return map(Hash, native.Store.fetchData(self, s, c))

    def countData(self):
        return native.Store.countData(self)

    def fetchIndex(self, s, c):
        return map(Hash, native.Store.fetchIndex(self, s, c))

    def countIndex(self):
        return native.Store.countIndex(self)

    def fetchMeta(self, s, c):
        return map(Hash, native.Store.fetchMeta(self, s, c))

    def countMeta(self):
        return native.Store.countMeta(self)

    def __str__(self):
        return "%s[buckets=%d]" % (self.directory, len(self.buckets.all()))

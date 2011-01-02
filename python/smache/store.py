#!/usr/bin/env python

import os
import random

import native
from chunks import *
from buckets import *
from exceptions import *
from constants import *

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

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "%s[buckets=%d]" % (self.directory, self.buckets.size())

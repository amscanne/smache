#!/usr/bin/env python

import os
import random

import native
from chunks import *
from exceptions import *
from constants import *


class BucketMap:
    buckets = []

    def add(self, bucket):
        self.buckets.append(bucket)

    def remove(self, bucket):
        self.buckets.remove(bucket)

    def all(self):
        return self.buckets[:]        

class Remote(native.Remote):
    buckets = None

    def __init__(self, address):
        native.Remote(self)
        self.address = address
        self.buckets = BucketMap()

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

    def _fetchAll(self, s, c):
        return self.fetchAll(s, c)

    def _countAll(self):
        return self.countAll()

    def _fetchData(self, s, c):
        return self.fetchData(s, c)

    def _countData(self):
        return self.countData()

    def _fetchIndex(self, s, c):
        return self.fetchIndex(s, c)

    def _countIndex(self):
        return self.countIndex()

    def _fetchMeta(self, s, c):
        return self.fetchMeta(s, c)

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

    def fetchAll(self, s, c):
        return []

    def countAll(self):
        return 0

    def fetchData(self, s, c):
        return []

    def countData(self):
        return 0

    def fetchIndex(self, s, c):
        return []

    def countIndex(self):
        return 0

    def fetchMeta(self, s, c):
        return []

    def countMeta(self):
        return 0

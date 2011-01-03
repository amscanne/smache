#!/usr/bin/env python

import os
import random

from constants import *

class Buckets:
    buckets = []

    def __init__(self):
        if len(self.buckets) == 0:
            self.resize(DEFAULT_BUCKETS)

    def size(self):
        return len(self.buckets)

    def resize(self, n):
        while len(self.buckets) > n: 
            del self.buckets[random.randint(0, len(self.buckets)-1)]
        while len(self.buckets) < n: 
            self.buckets.append(str(Hash()))

    def shrink(self):
        return self.resize(os.max(self.size()-1, 0))

    def grow(self):
        return self.resize(self.size()+1)

    def add(self, bucket):
        self.buckets.append(bucket)

    def remove(self, bucket):
        self.buckets.remove(bucket)

    def add(self, bucket):
        self.buckets.append(bucket)

    def remove(self, bucket):
        self.buckets.remove(bucket)

    def all(self):
        return self.buckets[:]        

class BucketFile(Buckets):
    filename = None

    def __init__(self, directory):
        self.filename = os.path.join(directory, 'buckets')
        self.load()

    def load(self):
        f = open(self.filename, 'w+')
        for l in f.readlines():
            Buckets.add(self, l.rstrip())
        f.close()

    def save(self):
        f = open(self.filename, 'w+')
        for b in self.buckets:
            f.write(b + "\n")
        f.close()

    def resize(self, n):
        Buckets.resize(self, n)
        self.save()

    def add(self, bucket):
        Buckets.add(self, bucket)
        self.save() 

    def remove(self, bucket):
        Buckets.remove(self, bucket)
        self.save()

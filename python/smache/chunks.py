#!/usr/bin/env python

import native
from exceptions import *

class Hash(native.Hash):

    def __init__(self, h=None):
        if h:
            this = h
            try: self.this.append(this)
            except: self.this = this
        else:
            native.Hash.__init__(self)

    def __repr__(self):
        return str(self)

    def __str__(self):
        return self.tostr()

class Chunk(native.Chunk):

    # refs()
    # adjrefs()
    # length()
    # compress()
    # uncompress()

    def __init__(self, c=None):
        if c:
            this = c
            try: self.this.append(this)
            except: self.this = this
        else:
            native.Chunk.__init__(self)

    def set(self, data, length=None):
        if length:
            native.Chunk.set(self, data, length)
        else:
            native.Chunk.set(self, data, len(data))

    def hash(self):
        return Hash(native.Chunk.hash(self))

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "%s[refs=%d,length=%d]" % (self.hash(), self.refs(), self.length())

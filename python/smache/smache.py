#!/usr/bin/env python

import native

from chunks import *
from exceptions import *

class Smache(native.Smache):
    compression = False

    def __init__(self, r, w, n, cas, compression):
        native.Smache.__init__(self, r, w, n, cas)
        self.compression = compression        

    def info(self, h):
        return Chunk(self.dinfo(h))

    def create(self, fd, offset, length):
        return Hash(self.dcreate(fd, offset, length))
    
    def append(self):
        return Hash(self.dappend(fd, length))
    
    def read(self):
        return self.dread(h, fd, offset, length)
    
    def write(self):
        return self.dwrite(h, fd, offset, length)
    
    def remove(self, h):
        return Hash(self.dremove(h, offset, length))
    
    def truncate(self, h, length):
        return Hash(self.dtruncate(h, length))
    
    def map(self, n, h):
        return self.imap(n, h)

    def lookup(self, n):
        return Hash(self.ilookup(n))

    def remap(self, n, h):
        return self.iremap(n, h)

    def unmap(self, n):
        return self.iunmap(n)

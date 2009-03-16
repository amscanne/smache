#!/usr/bin/env python

import native

class BerkeleyDB:
    def __init__(self, smache, filename):
        self.bdb = native.smache_berkeleydb_backend(filename)
        native.smache_add_backend(smache.sm, self.bdb)

#!/usr/bin/env python

import native

#
# The only backend currently supported is BerkeleyDB.
#

class BerkeleyDB:
    def __init__(self, smache, filename):
        self.bdb = native.smache_berkeleydb_backend(filename)
        native.smache_add_backend(smache.sm, self.bdb)

    def debug(self, state):
        native.smache_backend_setdebug(self.bdb, int(state))


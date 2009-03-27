#!/usr/bin/env python

import native

class UnknownBackend(Exception):
    def __init__(self, type):
        self.type = type

    def __str__(self):
        return self.type

class Backend:
    def __init__(self, smache, options):
        raise UnknownBackend()

    def debug(self, state):
        native.smache_backend_setdebug(self.bdb, int(state))

#
# The only backend currently supported is BerkeleyDB.
#

class BerkeleyDB(Backend):
    def __init__(self, smache, options):
        self.bdb = native.smache_berkeleydb_backend(options["filename"])
        native.smache_add_backend(smache.sm, self.bdb)

#
# This gets the class based on a string.
#

def getclass(s):
    if s == "berkeleydb":
        return BerkeleyDB
    else:
        raise UnknownBackend(s)

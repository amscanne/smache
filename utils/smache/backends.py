#!/usr/bin/env python

import native


#
# The only backend currently supported is BerkeleyDB.
#

class BerkeleyDB:
    def __init__(self, smache, options):
        self.bdb = native.smache_berkeleydb_backend(options["filename"])
        self.setdebug(options.get("debug", False))
        native.smache_add_backend(smache, self.bdb)

    def setdebug(self, state):
        native.smache_backend_setdebug(self.bdb, int(state))

#
# This gets the class based on a string.
#

def getclass(s):
    if s == "berkeleydb":
        return BerkeleyDB
    else:
        return None

def getallclasses():
    return ["berkeleydb"]

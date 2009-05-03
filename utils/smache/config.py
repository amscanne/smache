#!/usr/bin/env python

import sys
import files
import native
import backends
import instance
import ConfigParser

#
# The different types of block and compression algorithms possible.
#
block_algos = {
    "fixed":native.SMACHE_FIXED,
    "rabin":native.SMACHE_RABIN,
}

compression_algos = {
    "none":native.SMACHE_NONE,
    "lzo":native.SMACHE_LZO
}

#
# Keyword mappers.  When you see a special value, these are used to
# convert the given value to a different one.
#
keyword_mappers = {
    "block":lambda x: block_algos[x],
    "compression":lambda x: compression_algos[x]
}

#
# The actual config parser.
#
class Config:
    def __init__(self, filename=None):
        self.filename = filename

        #
        # Store options for the main instance and backends.
        #
        self.smache   = {}
        self.backends = []

        #
        # Load the configuration.
        #
        if self.filename:
            self.load(self.filename)
      
    def load(self, filename):
        #
        # Parse the file.
        #
        cp = ConfigParser.ConfigParser()
        cp.read(filename)

        globalfound = False

        for s in cp.sections():
            options = dict()
            options.update(map(lambda (k, v): (k, keyword_mappers.has_key(k) and keyword_mappers[k](v) or v), cp.items(s)))

            # NOTE: Special section is "smache".
            if s == "smache":
                self.smache = options

            # All other sections are for backends.
            else:
                if not(backends.getclass(s)):
                    sys.stderr.write("error: Unknown backend type '%s'.  Require one of: %s\n" % \
                                     (s, ", ".join(backends.getallclassses())))

                btype = backends.getclass(s)
                self.backends.append((btype, options))

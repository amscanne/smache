#!/usr/bin/env python

import files
import backends
import instance
import ConfigParser

class ConfigAppliedException:
    def __str__(self):
        return "Configuration applied twice."

class Config:
    def __init__(self, filename=None):
        self.filename = filename

        # Set the defaults.
        self.index    = None
        self.debug    = False
        self.backends = []
        self.instance = None
        self.applied  = False

        # Load the configuration.
        if self.filename:
            self.load()
      
        # Dump the configuration (if debug is on).
        if self.debug:
            self.dump()

        # Apply the configuration.
        self.apply()

    def load(self):
        cp = ConfigParser.ConfigParser()
        cp.read(self.filename)

        for s in cp.sections():
            # NOTE: Special section is "global".
            if s == "global":
                self.debug = cp.getint(s, "debug")
                self.index = files.Index(cp.get(s, "index"))

            # All other sections are for backends.
            else:
                btype = backends.getclass(cp.get(s, "type"))
                options = dict()
                options.update(cp.items(s))
                del options["type"]
                self.backends.append((s, btype, options))

    def dump(self):
        print "[global]"
        print "	index=%s" % str(self.index)
        print "	debug=%s" % str(self.debug)
        for (name, btype, options) in self.backends:
            print "[%s]" % name
            print "	type=%s" % str(btype).split(".")[-1]
            for (k,v) in options.items():
                print "	%s=%s" % (k,v)

    def apply(self):
        # Don't allow two apply statements.
        if self.applied:
            raise ConfigAppliedException()
        self.applied = True

        # Create an instance.
        self.instance = instance.Smache()
        self.instance.debug(self.debug)

        # Create all the necessary backends.
        for (name, btype, options) in self.backends:
            backend = btype(self.instance, options)
            backend.debug(options.get("debug", False))

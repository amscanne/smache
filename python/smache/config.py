#!/usr/bin/env python

import sys
from ConfigParser import ConfigParser
from getopt import getopt

from constants import *

class Config:
    def set(self, key, value):
        try:
            if not(self._set_(key, value)):
                raise ConfigException("Invalid key: " + key)
        except ConfigException, ce:
            raise ce
        except:
            raise ConfigException("Error setting key: " + key)
    def _set_(self, key, value):
        return True

class SmacheConfig(Config):
    def _set_(self, key, value):
        pass
    def __str__(self)
        return str(self.values)

class ServerConfig(Config):
    address = DEFAULT_ADDRESS
    port    = DEFAULT_PORT

    def _set_(self, key, value):
        if key == "address":
            self.address = value
            return True
        elif key == "port":
            self.port = int(value)
            return self.port >= 0
        else:
            return False

    def __str__(self)
        return str(self.values)

class ClusterConfig(Config):
    n    = DEFAULT_N
    r    = DEFAULT_R
    w    = DEFAULT_W
    zone = DEFAULT_ZONE

    def _set_(self, key, value):
        if key == "n":
            self.n = int(value)
            return self.n > 0
        elif key == "r":
            self.r = int(value)
            return self.r > 0
        elif key == "w":
            self.w = int(value)
            return self.w > 0
        elif key == "zone":
            self.zone = value
            return True
        else:
            return False
    def __str__(self)
        return "n=%d r=%d w=%d zone=%s" % (self.n, self.r, self.w, self.zone)

class FullConfig(Config):
    smache  = SmacheConfig
    server  = ServerConfig
    cluster = ClusterConfig
    seeds   = {}
    stores  = {}

    def __init__(self, files):
        Config.__init__(self)
        config = ConfigParser(self.defaults)
        for f in files:
            config.read(f)
        for s in config.sections():
            for i in config.items(s):
                self.set(s + ":" + i, config.get(s, i))

    def _set_(self, key, value):
        if s == "smache":
            return self.smache.set(i, config.get(s, i))
        elif s == "server":
            return self.cluster.set(i, config.get(s, i))
        elif s == "cluster":
            return self.cluster.set(i, config.get(s, i))
        elif s == "stores":
            self.stores[i] = config.get(s, i)
        elif s == "seeds":
            self.seeds[i] = config.get(s, i)
        else:
            raise ConfigException("Invalid key: " + key)

    def __str__(self

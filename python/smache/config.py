#!/usr/bin/env python

import sys
from ConfigParser import ConfigParser
from getopt import getopt

from constants import *

class ConfigBase:
    def set(self, key, value):
        try:
            if not(self._set_(key, value)):
                raise ConfigException("invalid key: " + key)
        except ConfigException, ce:
            raise ce
        except:
            raise ConfigException("error setting key: " + key)

    def _set_(self, key, value):
        return True

class SmacheConfig(ConfigBase):
    algo        = DEFAULT_ALGO
    blocksize   = DEFAULT_BLOCKSIZE
    compression = DEFAULT_COMPRESSION
    
    def _set_(self, key, value):
        if key == "algo":
            self.algo = algo
            return True
        elif key == "blocksize":
            self.blocksize = int(value)
            return self.blocksize > 0
        elif key == "compression":
            self.compression = value.tolower() == "true"
            return True
        else:
            return False

    def _str_(self):
        return "algo=%s blocksize=%d compression=%s" % (self.algo, self.blocksize, str(self.compression))

class ServerConfig(ConfigBase):
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

    def _str_(self):
        return "address=%s port=%d" % (self.address, self.port)

class ClusterConfig(ConfigBase):
    n    = DEFAULT_N
    r    = DEFAULT_R
    w    = DEFAULT_W

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
        else:
            return False

    def __str__(self):
        return "n=%d r=%d w=%d" % (self.n, self.r, self.w)

class Config:
    smache  = SmacheConfig
    server  = ServerConfig
    cluster = ClusterConfig
    seeds   = {}
    stores  = {}

    def __init__(self, files):
        config = ConfigParser({})
        for f in files:
            config.read(f)

            # Parse each of the sections.
            for s in config.sections():
                for i in config.items(s):
                    self.set(s, i[0], i[1])

    def set(self, s, key, value):
        if s == "smache":
            return self.smache.set(i, config.get(s, i))
        elif s == "server":
            return self.cluster.set(i, config.get(s, i))
        elif s == "cluster":
            return self.cluster.set(i, config.get(s, i))
        elif s == "stores":
            self.stores[key] = value
        elif s == "seeds":
            self.seeds[key] = value
        else:
            raise ConfigException("invalid key: " + key)

    def __str__(self):
        return "smache[%s]\nserver[%s]\ncluster[%s]\nseeds%s\nstores%s" % \
         (str(self.smache), str(self.server), str(self.cluster), str(self.seeds), str(self.stores))

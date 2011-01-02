#!/usr/bin/env python

import sys

from chunks import *
from store import *
from remote import *
from smache import *
from server import *
from cluster import *
from exceptions import *

from config import Config

from native import setLogLevel, getLogLevel

def go(config):
    # Print an initial welcome.
    sys.stderr.write("Starting...\n")

    # Create the CAS splitter based on algo and blocksize.
    if config.smache.algo == "rabin":
        cas = native.RabinChunker(config.smache.blocksize)
    elif config.smache.algo == "fixed":
        cas = native.FixedChunker(config.smache.blocksize)
    else:
        raise ConfigException("invalid algorithm, only rabin and fixed are possible")

    # Create the instance with the splitter.
    r = config.cluster.r
    w = config.cluster.w
    n = config.cluster.n
    compression = config.smache.compression
    instance = smache.Smache(r, w, n, cas, compression)

    # Create the stores.
    stores = []
    for s in config.stores.items():
        store = Store(s[1])
        sys.stderr.write(" store %s -> %s\n" % (s[0], str(store)))
        stores.append(store)

    # Create the cluster manager (detects hosts, etc.).
    cluster = ClusterManager(instance, r, w, n, config.seeds, stores)
    cluster.start()

    # Run the server.
    server = Server(instance, cluster, config.server.address, config.server.port)
    server.run()

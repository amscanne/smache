#!/usr/bin/env python

import instance
import stats
import sys
import os
import stat

#
# This class defines a simple index of files.
#
# This index is stored as a simple list, as follows:
# <hash> <permissions> <file path>
#

class Index:
    def __init__(self, filename):
        self.filename = filename
        self.load()

    def load(self):
        self.index = dict()
        try:
            f = open(self.filename, "r")
            for l in f.readlines():
                try:
                    (hash, perms, filename) = l.split(' ', 2)
                except:
                    continue
                filename = filename.rstrip()
                self.index[filename] = (hash, int(perms))
            f.close()
        except:
            pass

    def save(self):
        f = open(self.filename, "w")
        for k in self.index.keys():
            (hash, perms) = self.index[k]
            f.write("%s %d %s\n" % (hash, perms, k))
            f.flush()
        f.close()

    def list(self):
        return self.index.keys()

    def hashes(self):
        return map(lambda x: x[0], self.index.values())

    def extract(self, sm, files):
        for file in files:
            self.extractone(sm, file)

    def extractone(self, sm, file):
        (dirname, filename) = os.path.split(file)
        if dirname and not(os.path.exists(dirname)):
            os.makedirs(dirname, mode=0777)
        (hash, perms) = self.index[file]
        sm.getfile(hash, file)
        os.chmod(file, perms)

    def add(self, sm, files):
        for file in files:
            self.addone(sm, file)

    def addone(self, sm, file):
        if os.path.isdir(file):
            for root, dirs, files in os.walk(file):
                self.add(sm, map(lambda x: os.path.join(root, x), files))
        else:
            hash     = sm.addfile(file)
            statinfo = os.stat(file)
            perms    = statinfo[stat.ST_MODE]
            self.index[file] = (hash, perms)

    def __str__(self):
        return self.filename

#
# A FileStore is a special class of Smache instance.
# It basically allows for adding and removing of files
# from the index (using the instance as the storage).
#
class FileStore:
    def __init__(self, config):
        if not(config.smache.has_key("index")):
            sys.stderr.write("error: No index defined in the configuration file.\n")
            sys.exit(1)

        self.config   = config
        self.index    = Index(config.smache["index"])
        self.instance = instance.Smache(config)

    def list(self):
        return self.index.list()

    def getstats(self):
        return stats.Stats(self.instance, self.index, self.config)

    def save(self):
        return self.index.save()

    def add(self, files):
        return self.index.add(self.instance, files)

    def extract(self, files):
        return self.index.extract(self.instance, files)

#!/usr/bin/env python

import instance
import stats
import sys
import os
import re
import stat
import shelve

#
# This class defines a simple index of files.
#
# This index is stored as a simple list, as follows:
# <hash> <permissions> <file path>
#

class Index:
    def __init__(self, filename):
        self.filename = filename
        self.index    = shelve.open(filename)

    def list(self):
        return self.index.keys()

    def contains(self, name):
        return self.index.has_key(name)
    
    def lookup(self, name):
        return self.index[name][0]

    def hashes(self):
        return map(lambda x: x[0], self.index.values())

    def extractlist(self, sm, files):
        count = 0
        for file in files:
            count += self.extractone(sm, file)
        return count

    def length(self, sm, file):
        hash = self.index[file][0]
        return sm.totallength(hash)

    def mode(self, file):
        (hash, mode, uid, gid, atime, mtime, ctime) = self.index[file]
        return mode

    def atime(self, file):
        (hash, mode, uid, gid, atime, mtime, ctime) = self.index[file]
        return atime

    def ctime(self, file):
        (hash, mode, uid, gid, atime, mtime, ctime) = self.index[file]
        return ctime

    def mtime(self, file):
        (hash, mode, uid, gid, atime, mtime, ctime) = self.index[file]
        return mtime

    def uid(self, file):
        (hash, mode, uid, gid, atime, mtime, ctime) = self.index[file]
        return uid

    def gid(self, file):
        (hash, mode, uid, gid, atime, mtime, ctime) = self.index[file]
        return gid

    def extractone(self, sm, file):
        (dirname, filename) = os.path.split(file)
        if dirname and not(os.path.exists(dirname)):
            os.makedirs(dirname, mode=0777)
        (hash, mode, uid, gid, atime, mtime, ctime) = self.index[file]
        sm.getfile(hash, file)
        os.chmod(file, mode)
        os.chown(file, uid, gid)
        os.utime(file, (atime, mtime))
        return 1

    def addlist(self, sm, files, overwrite=True):
        count = 0
        for file in files:
            count += self.addone(sm, file, overwrite=overwrite)
        return count

    def addone(self, sm, file, overwrite=True):
        file  = os.path.normpath(file)
        if os.path.isdir(file):
            count = 0
            for root, dirs, files in os.walk(file):
                count += self.addlist(sm, map(lambda x: os.path.join(root, x), files), overwrite=overwrite)
            return count
        elif self.contains(file) and not(overwrite):
            return 0
        else:
            hash     = sm.addfile(file)
            statinfo = os.stat(file)
            self.index[file] = (hash, \
                statinfo[stat.ST_MODE], \
                statinfo[stat.ST_UID], \
                statinfo[stat.ST_GID], \
                statinfo[stat.ST_ATIME], \
                statinfo[stat.ST_MTIME], \
                statinfo[stat.ST_CTIME])
            return 1

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

    def getstats(self):
        return stats.Stats(self.instance, self.index, self.config)

    def length(self, file):
        return self.index.length(self.instance, file)

    def add(self, files, overwrite=True):
        if type(files) == list:
            return self.index.addlist(self.instance, files, overwrite=overwrite)
        else:
            return self.index.addone(self.instance, files, overwrite=overwrite)

    def extract(self, files):
        if type(files) == list:
            return self.index.extractlist(self.instance, files)
        else:
            return self.index.extractone(self.instance, files)

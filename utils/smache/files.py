#!/usr/bin/env python

import instance
import stats
import sys
import os
import re
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
                m = re.match("([^#]*)#.*", l)
                if m:
                    l = m.group(1)
                try:
                    (hash, mode, uid, gid, atime, mtime, ctime, filename) = l.split(' ', 8)
                except:
                    continue
                filename = filename.rstrip()
                self.index[filename] = (hash, int(mode, 8), int(uid), int(gid), int(atime), int(mtime), int(ctime))
            f.close()
        except:
            pass

    def save(self):
        f = open(self.filename, "w")
        f.write("# hash mode uid gid atime mtime ctime filename\n")
        for k in self.index.keys():
            (hash, mode, uid, gid, atime, mtime, ctime) = self.index[k]
            f.write("%s %o %d %d %d %d %d %s\n" % (hash, mode, uid, gid, atime, mtime, ctime, k))
            f.flush()
        f.close()

    def list(self):
        return self.index.keys()

    def contains(self, name):
        return self.index.has_key(name)
    
    def lookup(self, name):
        return self.index[name][0]

    def hashes(self):
        return map(lambda x: x[0], self.index.values())

    def extractlist(self, sm, files):
        for file in files:
            self.extractone(sm, file)

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
        os.chmod(file, perms)
        os.chown(file, uid)
        os.chgrp(file, gid)
        os.utime(file, (atime, mtime))

    def addlist(self, sm, files, overwrite=True):
        for file in files:
            self.addone(sm, file, overwrite=overwrite)

    def addone(self, sm, file, overwrite=True):
        file = os.path.normpath(file)
        if os.path.isdir(file):
            for root, dirs, files in os.walk(file):
                self.addlist(sm, map(lambda x: os.path.join(root, x), files), overwrite=overwrite)
        elif self.contains(file) and not(overwrite):
            return
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

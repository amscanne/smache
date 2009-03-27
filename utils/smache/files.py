#!/usr/bin/env python

import instance
import os
import stat

class NoIndexException:
    def __str__(self):
        return "No index defined in configuration file."

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
            hash = sm.addfile(file)
            statinfo = os.stat(file)
            perms = statinfo[stat.ST_MODE]
            self.index[file] = (hash, perms)

    def __str__(self):
        return self.filename

class FileStore:
    def __init__(self, config):
        self.config = config
        if not(self.config.index):
            raise NoIndexException()

    def list(self):
        return self.config.index.list()

    def save(self):
        return self.config.index.save()

    def add(self, files):
        return self.config.index.add(self.config.instance, files)

    def extract(self, files):
        return self.config.index.extract(self.config.instance, files)

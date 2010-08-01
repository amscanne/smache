#!/usr/bin/python

try:
    import _find_fuse_parts
except ImportError:
    pass
import fuse
from fuse import Fuse
from time import time

fuse.fuse_python_api = (0, 2)

import stat
import os
import errno
import sys

#
# This file is based on the fuse example file system.
# NOTE: We don't support mutable files currently, only
# exporting read-only file systems.
#

class BaseStat(fuse.Stat):
    def __init__(self):
        fuse.Stat.__init__(self)
        self.st_atime = int(time())
        self.st_mtime = self.st_atime
        self.st_ctime = self.st_atime
        self.st_nlink = 1
        self.st_size  = 4096
        # self.st_ino   = 0
        # self.st_dev   = 0

    def __str__(self):
        return "Mode: %o Ino: %d Dev: %d Nlink: %d Uid: %d Gid: %d Size: %d Atime: %d Mtime: %d Ctime: %d" % \
            (int(self.st_mode), int(self.st_ino), int(self.st_dev), int(self.st_nlink), \
             int(self.st_uid), int(self.st_gid), int(self.st_size), \
             int(self.st_atime), int(self.st_mtime), int(self.st_ctime))

class FS(Fuse):

    def __init__(self, store):
        Fuse.__init__(self)
        self.store         = store
        self.files         = self.normedfiles()
        self.dirs          = self.normeddirs()

    def getattr(self, path):
        st   = BaseStat()
        path = os.path.normpath(path)

        while len(path) > 0 and path[0] == '/':
            path = path[1:]

        #
        # Take care of the case of a directory.
        #
        if not(path) or path in self.dirs:
            st.st_nlink = 2 + len(self.listdir(path))
            st.st_mode  = stat.S_IFDIR | 0755
            return st

        #
        # Figure out if it's a file, and the path in the index.
        #
        elif path in self.files:
            if self.store.index.contains(path):
                realname = path
            elif self.store.index.contains('/' + path):
                realname = '/' + path
            else:
                return -errno.ENOENT

        #
        # Otherwise, it doesn't exist.
        #
        else:
            return -errno.ENOENT

        #
        # Take all the stat settings from the index.
        #
        st.st_mode  = self.store.index.mode(realname)
        st.st_size  = self.store.length(realname)
        st.st_uid   = self.store.index.uid(realname)
        st.st_gid   = self.store.index.gid(realname)
        st.st_atime = self.store.index.atime(realname)
        st.st_mtime = self.store.index.mtime(realname)
        st.st_ctime = self.store.index.ctime(realname)

        print str(st)
        return st

    def normedfiles(self):
        #
        # Build the list of all files.
        #
        files = self.store.index.list()
        for i in range(len(files)):
            files[i] = os.path.normpath(files[i])
            if files[i][0] == '/':
                files[i] == files[i][1:]
        return files

    def normeddirs(self):
        #
        # Build the list of all directories.
        #
        dirs = []
        for file in self.files:
            dirname = os.path.dirname(file)
            dirs.append(dirname) 
        return dirs

    #
    # Built up a list of matching files.
    #
    def readdir(self, path, offset):
        for f in self.listdir(path):
            yield fuse.Direntry(f)

    def listdir(self, path):
        path  = os.path.normpath(path)
        while len(path) > 0 and path[0] == '/':
            path = path[1:]

        files = ['.', '..']
        for f in self.files:
            (head, tail) = os.path.split(f)
            if not(tail in files) and head == path:
                files.append(tail)
        for f in self.dirs:
            (head, tail) = os.path.split(f)
            if not(tail in files) and head == path:
                files.append(tail)

        return files
        
    def open(self, path, flags):
        path = os.path.normpath(path)
        while len(path) > 0 and path[0] == '/':
            path = path[1:]

        if not(path in self.dirs) and not(path in self.files):
            return -errno.ENOENT

        accmode = os.O_RDONLY | os.O_WRONLY | os.O_RDWR
        if (flags & accmode) != os.O_RDONLY:
            return -errno.EACCESS

        return 0

    def read(self, path, size, offset):
        path = os.path.normpath(path)
        while len(path) > 0 and path[0] == '/':
            path = path[1:]

        err = self.open(path, os.O_RDONLY)
        if err:
            return err

        if self.store.index.contains(path):
            realname = path
        elif self.store.index.contains('/' + path):
            realname = '/' + path
        else:
            return -errno.ENOENT

        hash = self.store.index.lookup(realname)
        buff = self.store.instance.get(hash, offset, size)
        return buff

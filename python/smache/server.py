#!/usr/bin/env python

import sys
import time
import re
import BaseHTTPServer

from log import log
from smache import *
from cluster import *

#
# This file serves as the main entry point into the service itself.
# It provides a very basic protocol through which you can manipulate
# data, config, etc. all over HTTP.
#
# The other side of this service is the Remote, which should be
# accessed via client.py, not directly.
#

class PathHandler:
    handler = None

    def __init__(self, handler):
        self.handler = handler

    def head(self, path):
        self.handler.error()

    def get(self, path):
        self.handler.notfound()

    def put(self, path):
        self.handler.error()

    def post(self, path):
        self.handler.error()

    def delete(self, path):
        self.handler.error()

class ClusterInfoPath(PathHandler):
    cluster = None

    def __init__(self, handler, cluster):
        PathHandler.__init__(self, handler)
        self.cluster = cluster

    # Not provided:
    # head: error
    # delete: error
    # put: error

    # Dump all the cluster information.
    def get(self, path):
        self.handler.OK()
        self.cluster.save(self.handler.wfile)

    # Allow a host to update cluster information.
    def post(self, path):
        self.handler.OK()
        self.cluster.load(self.handler.wfile)

class ChunkPath(ClusterInfoPath):
    smache = None

    def __init__(self, handler, cluster, smache):
        ClusterInfoPath.__init__(self, handler, cluster)
        self.smache = smache

    def range(self):
        r = self.handler.headers['Content-Range']
        m = re.match("bytes (.*)-(.*)/(.*)")
        if m:
            return (int(m.group(1)), int(m.group(2)), int(m.group(3)))
        else:
            return None

    # Grab a listing of all hashes.
    def get(self, path):
        if path == '':
            self.handler.OK()
            self.cluster.listAll(self.handler.wfile)
        elif path == 'data':
            self.handler.OK()
            self.cluster.listData(self.handler.wfile)
        elif path == 'meta':
            self.handler.OK()
            self.cluster.listMeta(self.handler.wfile)
        else:
            l = self.handler.headers['Content-Length']
            r = self.handler.headers['Content-Range']
            h = Hash(path)
            c = self.smache.info(h)
            if c:
                headers = { 'Content-Range' : 'bytes %ld-%ld/%ld' % (r, r+l, c.length()),
                            'Content-Length' : l,
                            'References' : c.references(),
                            'Type' : c.blocktype() }
                self.handler.OK(headers)
                self.smache.read(h, self.handler.wfile.fileno(), r, l)

    # Grab the meta information for a hash.
    def head(self, path):
        if path == '' or path == 'data' or path == 'meta':
            self.handler.error()
        else:
            h = Hash(path)
            c = self.smache.info(h)
            if c:
                headers = { 'Content-Length' : c.length(),
                            'References' : c.references(),
                            'Type' : c.blocktype() }
                self.handler.OK(headers)

    # Create a new path or append to an existing one.
    def put(self, path):
        if path == '':
            l = self.handler.headers['Content-Length']
            h = self.smache.create(self.handler.rfile.fileno(), l)
            if h:
                c = self.smache.info(h)
                headers = { 'Content-Length': c.length(), 
                            'References' : c.references(),
                            'Type' : c.blocktype() }
                self.handler.OK(headers)
                self.handler.wfile.write(str(h))
            else:
                self.handler.error()
        else:
            l = self.handler.headers['Content-Length']
            h = Hash(path)
            h = self.smache.append(h, self.handler.rfile.fileno(), l)
            if h:
                c = self.smache.info(h)
                headers = { 'Content-Length': c.length(), 
                            'References' : c.references(),
                            'Type' : c.blocktype() }
                self.handler.OK(headers)
                self.handler.wfile.write(str(h))
            else:
                self.handler.error()

    # Edit a piece of existing data.
    def post(self, path):
        if path == '':
            self.handler.error()
        else:
            l = self.handler.headers['Content-Length']
            r = self.handler.headers['Content-Range']
            h = Hash(path)
            h = self.smache.write(h, self.handler.rfile.fileno(), r, l)
            if h:
                c = self.smache.info(h)
                headers = { 'Content-Length': c.length(), 
                            'References' : c.references(),
                            'Type' : c.blocktype() }
                self.handler.OK(headers)
                self.handler.wfile.write(str(h))
            else:
                self.handler.error()

    # Delete a part.
    def delete(self, path):
        if path == '':
            self.handler.error()
        else:
            r = self.handler.headers['Content-Range']
            m = re.match("bytes (.*)-(.*)/(.*)")
            h = Hash(path)
            h = self.smache.remove(h, r, l)
            if h:
                c = self.smache.info(h)
                headers = { 'Content-Length': c.length(), 
                            'References' : c.references(),
                            'Type' : c.blocktype() }
                self.handler.OK(headers)
                self.handler.wfile.write(str(h))
            else:
                self.handler.error()

class IndexPath(PathHandler):
    smache = None

    def __init__(self, handler, smache):
        PathHandler.__init__(self, handler)
        self.smache = smache

    # Not provider:
    # head: error

    # Lookup an index key (using redirect for convenience).
    def get(self, path):
        if path == '':
            self.handler.OK()
            self.cluster.listIndices(self.handler.wfile)
        else:
            h = self.smache.lookup(path)
            if h:
                self.handler.redirect('/chunks/%s' % str(h))
            else:
                self.handler.error()

    # Put a new index key (will fail if already there).
    def put(self, path):
        if path == '':
            self.handler.error()
        else:
            h = Hash(self.rfile.read())
            if self.smache.map(path, h):
                self.handler.OK()
            else:
                self.handler.error()

    # Remap an existing key.
    def post(self, path):
        if path == '':
            self.handler.error()
        else:
            h = Hash(self.rfile.read())
            if self.smache.remap(path, h):
                self.handler.OK()
            else:
                self.handler.error()

    # Remove an existing key.
    def delete(self, path):
        if path == '':
            self.handler.error()
        else:
            if self.smache.unmap(path):
                self.handler.OK()
            else:
                self.handler.error()

class Server:

    def __init__(self, smache, cluster):
        self.HttpHandler.smache = smache
        self.HttpHandler.cluster = cluster

    class HttpHandler(BaseHTTPServer.BaseHTTPRequestHandler):
        smache = None
        cluster = None

        def forpath(self):
            # Check for a cluster info request.
            if self.path == '/':
                h = ClusterInfoPath(self, self.cluster)
                log(" path %s -> cluster(%s)" % (self.path, self.path[1:]))
                return (h, self.path[1:])

            # Check for a chunk or index request.
            m = re.match("/(chunks|index)/(.*)", self.path)
            if not(m):
                # The default path handler will error.
                log(" path %s -> error(%s)" % (self.path, self.path[1:]))
                return (PathHandler(self), self.path[1:])
            if m.group(1) == "chunks":
                log(" path %s -> chunks(%s)" % (self.path, m.group(2)))
                return (ChunkPath(self, self.cluster, self.smache), m.group(2))
            elif m.group(1) == "index":
                log(" path %s -> index(%s)" % (self.path, m.group(2)))
                return (IndexPath(self, self.smache), m.group(2))

        def OK(self, headers = {}):
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            log("Handled OK")
            for (k, v) in headers.items():
                self.send_header(k, v)
            self.end_headers()

        def redirect(self, path):
            self.send_response(301)
            self.send_header("Location", path)
            self.end_headers()

        def unauth(self):
            self.send_response(401)
            self.end_headers()

        def notfound(self):
            self.send_response(404)
            self.end_headers()

        def error(self, msg = None):
            self.send_response(500)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            if msg:
                self.wfile.write(msg)

        def do_GET(self):
            (h, p) = self.forpath()
            h.get(p)

        def do_HEAD(self):
            (h, p) = self.forpath()
            h.head(p)

        def do_PUT(self):
            (h, p) = self.forpath()
            h.put(p)

        def do_POST(self):
            (h, p) = self.forpath()
            h.post(p)

        def do_DELETE(self):
            (h, p) = self.forpath()
            h.delete(p)

    def run(self, address, port):
        clz = self.HttpHandler
        log("Server running...")
        log(" address = %s" % str(address))
        log(" port = %d" % port)
        httpd = BaseHTTPServer.HTTPServer((address, port), clz)
        httpd.serve_forever()

#!/usr/bin/env python

import sys
import time
import re
import BaseHTTPServer

from smache import *
from cluster import *

#
# This file serves as the main entry point into the service itself.
# It provides a very basic protocol through which you can manipulate
# data, config, etc. all over HTTP.
# The other side of this service is the Remote, which lives in remote.py.
#

class Server:
    address = None
    port = None

    class HttpHandler(BaseHTTPServer.BaseHTTPRequestHandler):
        smache = None
        cluster = None

        def do_GET(self):
            if self.path == "/":
                self.send_response(200)
                self.send_header("Content-type", "text/plain")
                self.end_headers()

            elif self.path == "/buckets":
                self.send_response(200)
                self.send_header("Content-type", "text/plain")
                self.end_headers()
                for b in self.cluster.buckets():
                    self.wfile.write("%s\n" % b)

            elif self.path == "/cluster":
                self.send_response(200)
                self.send_header("Content-type", "text/plain")
                self.end_headers()
                for h in self.cluster.hosts():
                    self.wfile.write("%s %s\n" % (h.address, h.timestamp, buckethash(h.buckets)))

                # Grab our own name and timestamp.
                addr = self.connection.getsockname()
                self.wfile.write("%s:%d\n" % (addr[0], addr[1]))

            elif re.match("/data/(.*)", self.path):
                pass

            elif re.match("/index/(.*)", self.path):
                pass

            else:
                self.send_response(500)
                self.end_headers()

        def do_HEAD(self):
            if re.match("/data/(.*)", self.path):
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()
                self.wfile.write("<html></html>")
            elif re.match("/index/(.*)", self.path):
            else:
                self.send_response(500)
                self.end_headers()

        def do_PUT(self):
            if re.match("/data/(.*)", self.path):
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()
            elif re.match("/index/(.*)", self.path):
            else:
                self.send_response(500)
                self.end_headers()

        def do_POST(self):
            if re.match("/data/(.*)", self.path):
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()
                self.wfile.write("<html></html>")
            elif re.match("/index/(.*)", self.path):
            else:
                self.send_response(500)
                self.end_headers()

        def do_DELETE(self):
            if re.match("/data/(.*)", self.path):
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()
                self.wfile.write("<html></html>")
            elif re.match("/index/(.*)", self.path):
            else:
                self.send_response(500)
                self.end_headers()

    def __init__(self, smache, cluster, address, port):
        self.address = address
        self.port = port
        self.HttpHandler.smache = smache
        self.HttpHandler.cluster = cluster

    def run(self):
        clz = self.HttpHandler
        sys.stderr.write("Server running...\n")
        sys.stderr.write(" address = %s\n" % str(self.address))
        sys.stderr.write(" port = %d\n" % self.port)
        httpd = BaseHTTPServer.HTTPServer((self.address, self.port), clz)
        httpd.serve_forever()

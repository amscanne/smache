#!/usr/bin/make -f

LIBRARY=libsmache.so
OBJECTS=berkeleydb.o memcached.o protocol.o smache.o misc.o files.o hash.o compress.o cut.o
CFLAGS:=-Wall -g -fPIC -I../include -I/opt/local/include
LIBFLAGS:=-lc -llzo2 -lmhash -ldb
LDFLAGS:=-g -L/opt/local/lib -L/opt/local/lib/db47 $(LIBFLAGS)
CC:=gcc

all: $(LIBRARY)

install:
	mkdir -p /usr/lib
	install -m 0755 -o 0 -g 0 $(LIBRARY) /usr/lib

clean:
	@rm -f $(OBJECTS) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(CC) -o $@ -shared $(LDFLAGS) $^

%.o: %.c $(wildcard ../include/smache/*)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: all install clean

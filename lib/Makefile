#!/usr/bin/make -f

LIBRARY=libsmache.so
OBJECTS=berkeleydb.o memcached.o protocol.o smache.o misc.o files.o hash.o compress.o
CFLAGS:=-Wall -fPIC -I../include -I/opt/local/include
LIBTOOLFLAGS:=-lc -lmhash -ldb -L/opt/local/lib
CC:=gcc

all: $(LIBRARY)

install:
	install -m 0755 -o 0 -g 0 $(LIBRARY) /usr/lib

clean:
	@rm -f $(OBJECTS) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(CC) -o $@ -shared $(LIBTOOLFLAGS) $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all install clean

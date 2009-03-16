#!/usr/bin/make -f

LIBRARY=libsmache.so
OBJECTS=berkeleydb.o memcached.o protocol.o smache.o
CFLAGS:=-Wall -fPIC -I../include -I/opt/local/include
LIBTOOLFLAGS:=-lc -lmhash -L/opt/local/lib
CC:=gcc

all: $(LIBRARY)

install:
	@echo "Not implemented."

clean:
	@rm -f $(OBJECTS) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	# libtool -o $@ -dynamic $(LIBTOOLFLAGS) $^
	$(CC) -o $@ -shared $(LIBTOOLFLAGS) $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all install clean

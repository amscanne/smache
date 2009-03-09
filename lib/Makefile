#!/usr/bin/make -f

LIBRARY=libsmache.so
OBJECTS=berkeleydb.o memcached.o protocol.o smache.o
LDFLAGS:=-shared -fPIC
CFLAGS:=-Wall -I../include
CC:=gcc
LD:=gcc

all: $(LIBRARY)

install:
	@echo "Not implemented."

clean:
	@rm -f $(OBJECTS) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all install clean

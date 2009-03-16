#!/usr/bin/make -f

BINARIES=smachezip smache
OBJECTS=smachezip.o smache.o server.o client.o
CFLAGS:=-Wall -I../include
LDFLAGS:=-lsmache -lc -L../lib
CC:=gcc
LD:=gcc

all: $(BINARIES)

install:
	@echo "Not implemented."

clean:
	@rm -f $(OBJECTS) $(BINARIES)

smachezip: smachezip.o
	$(LD) -o $@ $(LDFLAGS) $^

smache: smache.o
	$(LD) -o $@ $(LDFLAGS) $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all install clean

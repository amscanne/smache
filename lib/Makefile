#!/usr/bin/make -f

LIBRARY=libsmache.so
OBJECTS=store.o smache.o chunk.o cas.o log.o worker.o
CPPFLAGS:=-Wall -g -fPIC -I../include -I/opt/local/include -D_FILE_OFFSET_BITS=64
LIBFLAGS:=-lc -llzo2 -lmhash -lpthread
LDFLAGS:=-g -L/opt/local/lib -L/opt/local/lib/db44 $(LIBFLAGS)
CPP:=g++

all: $(LIBRARY) $(TESTS)

install:
	@mkdir -p /usr/lib
	@install -m 0755 -o 0 -g 0 $(LIBRARY) /usr/lib

dist:
	@false # FIXME, not yet implemented.

clean:
	@rm -f $(OBJECTS) $(LIBRARY) $(TESTS)

$(LIBRARY): $(OBJECTS)
	$(CPP) -o $@ -shared $(LDFLAGS) $^

%.o: %.cc $(wildcard ../include/smache/*.hh)
	$(CPP) $(CPPFLAGS) -o $@ -c $<

.PHONY: all install clean

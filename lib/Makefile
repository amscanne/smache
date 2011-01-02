#!/usr/bin/make -f

LIBRARY=libsmache.so
OBJECTS=store.o smache.o chunk.o cas.o log.o worker.o
CPPFLAGS:=$(CFLAGS) -Wall -fPIC -I../include -D_FILE_OFFSET_BITS=64
LINKFLAGS:=$(LDFLAGS) -lc -llzo2 -lmhash -lpthread
CPP:=g++
DESTDIR:=/

all: $(LIBRARY) $(TESTS)

install:
	@mkdir -p $(DESTDIR)/usr/lib
	@install -m 0755 -o 0 -g 0 $(LIBRARY) $(DESTDIR)/usr/lib

clean:
	@rm -f $(OBJECTS) $(LIBRARY) $(TESTS)

$(LIBRARY): $(OBJECTS)
	$(CPP) -o $@ -shared $(LINKFLAGS) $^

%.o: %.cc $(wildcard ../include/smache/*.hh)
	$(CPP) $(CPPFLAGS) -o $@ -c $<

.PHONY: all install clean

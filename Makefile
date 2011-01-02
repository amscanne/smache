#!/usr/bin/make -f

all:
	(cd lib && $(MAKE))
	(cd python && $(MAKE))

install: 
	(cd lib && $(MAKE) install)
	(cd python && $(MAKE) install)

dist:
	(cd lib && $(MAKE) dist)
	(cd python && $(MAKE) dist)

tests: all
	(cd tests && $(MAKE) test)
debugs: all
	(cd tests && $(MAKE) debug)

clean:
	(cd lib && $(MAKE) clean)
	(cd python && $(MAKE) clean)
	(cd tests && $(MAKE) clean)

.PHONY: all dist install tests debugs clean

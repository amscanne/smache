#!/usr/bin/make -f

all:
	(cd lib && make)
	(cd python && make)

install: 
	(cd lib && make install)
	(cd python && make install)

dist:
	(cd lib && make dist)
	(cd python && make dist)

tests: all
	(cd tests && make test)
debugs: all
	(cd tests && make debug)

clean:
	(cd lib && make clean)
	(cd python && make clean)
	(cd tests && make clean)

.PHONY: all dist install tests debugs clean

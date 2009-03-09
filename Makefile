#!/usr/bin/make -f

all: lib utils

install: 
	(cd lib && make install)
	(cd utils && make install)
	(cd include && make install)

clean:
	(cd lib && make clean)
	(cd utils && make clean)

lib:
	(cd lib && make)

utils:
	(cd utils && make)

.PHONY: all install clean lib utils

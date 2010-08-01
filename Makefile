#!/usr/bin/make -f

all: lib utils sanity

install: 
	(cd lib && make install)
	(cd utils && make install)
	(cd include && make install)

clean:
	(cd lib && make clean)
	(cd utils && make clean)
	rm sanity.*

lib:
	(cd lib && make)

utils:
	(cd utils && make)

sanity:
	bash run-sanity

.PHONY: all install clean lib utils sanity

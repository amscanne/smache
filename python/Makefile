#!/usr/bin/make -f

all: build

swig.i: swig.hh
	@touch $@

build: swig.i $(wildcard ../include/smache/*)
	@python setup.py build && cp native.py smache/ && python setup.py build

dist: build
	@python setup.py bdist

clean:
	@rm -rf native.py smache/native.py swig_wrap.*
	@rm -rf smache/*.pyc
	@rm -rf build

install:
	@python setup.py install

.PHONY: all build dist install clean

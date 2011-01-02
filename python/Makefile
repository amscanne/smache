#!/usr/bin/make -f

DESTDIR:=/

all: build

swig.i: swig.hh
	@touch $@

build: swig.i $(wildcard ../include/smache/*)
	@python setup.py build && cp native.py smache/ && python setup.py build

clean:
	@rm -rf native.py smache/native.py swig_wrap.*
	@rm -rf smache/*.pyc
	@rm -rf build

install: build
	@mkdir -p $(DESTDIR)
	@python setup.py install --prefix $(DESTDIR)/usr

.PHONY: all build install clean

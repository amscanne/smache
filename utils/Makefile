#!/usr/bin/make -f

all: build

build:
	python setup.py build
	python setup.py build

clean:
	rm -rf smache/native.py
	rm -rf smache/*.pyc
	rm -rf build

install:
	python setup.py install

.PHONY: all build install clean

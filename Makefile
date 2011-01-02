#!/usr/bin/make -f

DESTDIR:=/

all:
	(cd lib && $(MAKE))
	(cd python && $(MAKE))

install: init/smached config
	(cd lib && $(MAKE) install DESTDIR=$(DESTDIR))
	(cd python && $(MAKE) install DESTDIR=$(DESTDIR))
	@mkdir -p $(DESTDIR)/etc/init.d
	@install -m 0755 -o 0 -g 0 init/smached $(DESTDIR)/etc/init.d
	@install -m 0755 -o 0 -g 0 config $(DESTDIR)/etc/smached

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
	@rm -rf debian/debhelper.log debian/files debian/substvars

deb:
	@dpkg-buildpackage -b

.PHONY: all dist install tests debugs clean deb

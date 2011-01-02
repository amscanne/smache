#!/usr/bin/make -f

TESTS:=basic/remote basic/store
RUN:=$(TESTS:=.run) 
DBG:=$(TESTS:=.dbg)
VLG:=$(TESTS:=.vlg)

all: test

test: $(RUN)
debug: $(DBG)
valgrind: $(VLG)

%.run: %
	@./run $*
%.debug: %
	@./debug $*
%.grind: %
	@./grind $* 2>&1 | less

clean:
	@rm -rf scratch

.PHONY: all clean

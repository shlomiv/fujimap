#!/usr/bin/make -f
# Waf Makefile wrapper
WAF_HOME=/media/HD/fujimap

all:
	@/media/HD/fujimap/waf build

all-debug:
	@/media/HD/fujimap/waf -v build

all-progress:
	@/media/HD/fujimap/waf -p build

install:
	/media/HD/fujimap/waf install

uninstall:
	/media/HD/fujimap/waf uninstall

clean:
	@/media/HD/fujimap/waf clean

distclean:
	@/media/HD/fujimap/waf distclean
	@-rm -rf build
	@-rm -f Makefile

check:
	@/media/HD/fujimap/waf check

dist:
	@/media/HD/fujimap/waf dist

.PHONY: clean dist distclean check uninstall install all


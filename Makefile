#!/usr/bin/make -f
# Waf Makefile wrapper
WAF_HOME=/media/HD/fujimap-0.1.3

all:
	@/media/HD/fujimap-0.1.3/waf build

all-debug:
	@/media/HD/fujimap-0.1.3/waf -v build

all-progress:
	@/media/HD/fujimap-0.1.3/waf -p build

install:
	/media/HD/fujimap-0.1.3/waf install

uninstall:
	/media/HD/fujimap-0.1.3/waf uninstall

clean:
	@/media/HD/fujimap-0.1.3/waf clean

distclean:
	@/media/HD/fujimap-0.1.3/waf distclean
	@-rm -rf build
	@-rm -f Makefile

check:
	@/media/HD/fujimap-0.1.3/waf check

dist:
	@/media/HD/fujimap-0.1.3/waf dist

.PHONY: clean dist distclean check uninstall install all


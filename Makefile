all:
	cd src && $(MAKE) all

clean:
	cd src && $(MAKE) clean
	rm -Rf release

install:
	cd src && $(MAKE) install

test:
	cd src && $(MAKE) test

leak:
	cd src && $(MAKE) leak

coverage:
	cd src && $(MAKE) coverage

release: clean
	mkdir -p release/lib
	cd src && $(MAKE) PREFIX=$(shell pwd)/release FORCE_RPATH='@ORIGIN/../lib' install
	rm -f release/bin/test_log_proxy
	ldd release/bin/log_proxy
	cp -f `ldd release/bin/log_proxy |grep libglib |awk -F '=> ' '{print $$2;}' |awk '{print $$1;}'` `ldd release/bin/log_proxy |grep libpcre |awk -F '=> ' '{print $$2;}' |awk '{print $$1;}'` release/lib/

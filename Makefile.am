ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src
dist_doc_DATA = examples \
                README \
                doc/wasora.texi \
                doc/html \
                doc/wasora.xml

EXTRA_DIST = ChangeLog-0.2 ChangeLog-0.3 \
             locateruntest.sh \
             src/version.sh \
             doc

# TESTS = examples/test-fibonacci.sh \
#         examples/test-pi.sh \
#         examples/test-interp1d.sh \
#         examples/test-histogram.sh \
#         examples/test-logistic.sh \
#         examples/test-lorenz.sh \
#         examples/test-mathace.sh \
#         examples/test-fsm.sh \
#         examples/test-io.sh \
#         examples/test-roessler.sh

TESTS = tests/fibonacci.sh \
        tests/pi.sh

all-local:
	cp -r src/wasora$(EXEEXT) .
	rm -f src/version.$(OBJEXT) src/version.h src/version.lo src/.lib/version.$(OBJEXT)
	@echo
	@echo Happy engineering hacking with wasora!
	@echo

clean-local:
	rm -f wasora$(EXEEXT) src/.libs/*
	rm -f `ls examples/*.dat | grep -v binding | grep -v histogram-samples`

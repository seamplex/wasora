SUBDIRS = src doc
# DIST_SUBDIRS = src doc

ACLOCAL_AMFLAGS = -I wasora/m4
dist_doc_DATA = AUTHORS ChangeLog README TODO \
                doc/wasora.texi doc/wasora.info doc/logo.svg doc/wasora.xml doc/reference.md

dist_man_MANS = doc/wasora.1

EXTRA_DIST = examples doc locateruntest.sh \
             src/version.sh



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
        tests/pi.sh \
        tests/interp1d.sh \
        tests/lorenz.sh

all-local:
	cp -r src/wasora$(EXEEXT) .
# 	rm -f src/version.$(OBJEXT) src/version.h src/version.lo src/.lib/version.$(OBJEXT)
	@echo
	@echo Happy engineering hacking with wasora!
	@echo

clean-local:
	rm -f wasora$(EXEEXT) src/.libs/*
	rm -f `ls examples/*.dat | grep -v binding | grep -v histogram-samples`

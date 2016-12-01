ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src doc
dist_doc_DATA = examples \
                README \
                doc/wasora.texi \
                doc/html \
                doc/katepart

EXTRA_DIST = ChangeLog-0.2 ChangeLog-0.3 \
             locateruntest.sh \
             src/version.sh \
             doc

TESTS = examples/test-fibonacci.sh \
        examples/test-pi.sh \
        examples/test-interp1d.sh \
        examples/test-histogram.sh \
        examples/test-logistic.sh \
        examples/test-lorenz.sh \
        examples/test-mathace.sh \
        examples/test-fsm.sh \
        examples/test-io.sh \
        examples/test-roessler.sh


all-local:
	cp -r src/wasora$(EXEEXT) .
	rm -f src/version.$(OBJEXT) src/version.h src/version.lo src/.lib/version.$(OBJEXT)
	@echo
	@echo Happy engineering hacking with wasora!
	@echo

clean-local:
	rm -f wasora$(EXEEXT) src/.libs/*
	rm -f `ls examples/*.dat | grep -v binding | grep -v histogram-samples`

check-local:
	@echo
	@echo If you want to manually repeat the tests, type
	@echo
	@echo "./wasora$(EXEEXT) examples/fibonacci.was"
	@echo "./wasora$(EXEEXT) examples/pi.was"
# 	@echo "./wasora$(EXEEXT) examples/read-shm.was"
	@echo "./wasora$(EXEEXT) examples/logistic.was | gnuplot -p -e \"plot '<cat' pt 0\""
	@echo "./wasora$(EXEEXT) examples/mathace.was | gnuplot -p -e \"plot '<cat' pt 0\""
	@echo "./wasora$(EXEEXT) examples/lorenz.was | gnuplot -p -e \"splot '<cat' u 2:3:4 w l\""
	@echo "./wasora$(EXEEXT) examples/roessler-sender.was & ./wasora$(EXEEXT) examples/roessler-receiver.was | gnuplot -p -e \"splot '<cat' u 2:3:4 w l\""
	@echo

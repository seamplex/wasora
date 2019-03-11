#!/bin/sh
# 
# Execute this script to clean the directory and leave it as
# a fresh mercurial branch
#
if test -e Makefile; then
  make clean
fi
rm -f *~ .*~ src/*~
rm -rf src/.libs
rm -f src/version.h src/version-vcs.h src/version-conf.h
rm -f README INSTALL PLUGINS
rm -f version.m4
rm -f doc/README.pdf doc/README.html doc/INSTALL.pdf doc/INSTALL.html doc/PLUGINS.pdf doc/PLUGINS.html
rm -f aclocal.m4 configure config.log config.status config.guess config.sub compile depcomp install-sh missing configure.scan autoscan.log test-driver
rm -f Makefile Makefile.in src/Makefile src/Makefile.in
rm -rf autom4te.cache 

cd examples
./clean.sh
cd ..

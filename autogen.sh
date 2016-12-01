#!/bin/sh
# 
# Execute this script to generate a configure script
#
# This file is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.
#

# check for needed tools (cannot put this into an m4 macro
# because we do not even know if we have m4 available)
for i in git m4 autoconf xargs; do
 if [ -z "`which $i`" ]; then
  echo "error: $i not installed"
  exit 1
 fi
done

m4 m4/bootstrap.m4 - << EOF | sh
WASORA_CHECK_VCS
WASORA_AUTOCLEAN
WASORA_VERSION_VCS
WASORA_README_INSTALL
EOF

echo "calling autoreconf... "
autoreconf -i 2>&1
echo "done"

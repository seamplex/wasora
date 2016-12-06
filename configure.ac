dnl Process this file with autoconf to produce a configure script.
dnl
dnl This file is free software; as a special exception the author gives
dnl unlimited permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
dnl implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

m4_include([version.m4])
AC_INIT([wasora],wasoraversion,[jeremy@seamplex.com])

AC_CONFIG_SRCDIR([src/wasora.c])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

WASORA_OPT_FLAGS_C

WASORA_CHECK_BASIC_HEADERS
WASORA_CHECK_BASIC_LIBS
WASORA_CHECK_IDA
WASORA_CHECK_READLINE
WASORA_CHECK_GSL

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile]) 

WASORA_HOST_VERSION_H

AC_OUTPUT
echo
AS_BOX([Configuration summary])
WASORA_RESUME_LIBS
AS_IF([ test $ida -eq 0 -o $readline -eq 0 ], [
  WASORA_FINAL_WARN([0],[0])
],[
  WASORA_FINAL_WARN([1],[0])
])

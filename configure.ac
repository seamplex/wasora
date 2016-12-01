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

AC_CANONICAL_HOST

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

AC_ARG_ENABLE([mesh],
  [AS_HELP_STRING([--enable-mesh], [compile with wasora mesh framework @<:@default=no@:>@])],
  [AC_DEFINE([WASORA_MESH])])

# tested host_oses are:
#  linux-gnu
#  mingw32msvc (cross compiling from gnu/linux)
#  cygwin
#  mingw32
AS_CASE([${host_os}],
  [linux*],  [os_family="linux"],  dnl I know the OS is not linux but gnu, but this refers to the way the kernel works
  [mingw*],  [os_family="mingw"],
  [cygwin*], [os_family="cygwin"],
  [os_family="unknown"]
)
AM_CONDITIONAL([MINGW],  [test "x${os_family}" == "xmingw"])
AM_CONDITIONAL([CYGWIN], [test "x${os_family}" == "xcygwin"])

AS_IF([test "x${os_family}" == "xmingw" -a $readline -ne 0 ],
LIBS="$LIBS -lreadline.dll"
)

AS_IF([test "x${os_family}" == "xmingw"],
AS_MKDIR_P([src/.libs])
)

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

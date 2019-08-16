dnl This file is part of wasora and/or one of its plugins
dnl GPL v3+ (c) 2019 ramiro vignolo
dnl <https://github.com/seamplex/wasora>
dnl
AC_DEFUN([WASORA_CHECK_CVODES],[

# the default is read from the macro argument, but the help string
# does not expand variables so it always states that it is "check"
cvodes_default=m4_default([$1],[check])

AC_ARG_WITH([cvodes],
  [AS_HELP_STRING([--with-cvodes],
    [support systems of ODEs@<:@default=check@:>@])],
  [],
  [with_cvodes=${cvodes_default}])

AS_IF([test "x$with_cvodes" != xno],[
   AC_CHECK_HEADERS([sundials/sundials_types.h cvodes/cvodes.h], [],
    [AS_IF([test "x$with_cvodes" != xcheck],
       [AC_MSG_FAILURE([--with-cvodes was given, but test for cvodes headers failed])],
       [AC_MSG_WARN([sundials cvodes headers not found])])
    ])
   AC_CHECK_LIB([sundials_cvodes], [CVodeInit],,
    [AS_IF([test "x$with_cvodes" != xcheck],
       [AC_MSG_FAILURE([--with-cvodes was given, but test for cvodes libray failed])],
       [AC_MSG_WARN([sundials cvodes library (libsundials-cvodes) not found])])
    ])
   AC_CHECK_HEADER([nvector/nvector_serial.h], [],
    [AS_IF([test "x$with_cvodes" != xcheck],
       [AC_MSG_FAILURE([--with-cvodes was given, but test for sundials nvecserial headers failed])],
       [AC_MSG_WARN([sundials cvodes headers not found])])
    ])
   AC_CHECK_LIB([sundials_nvecserial], [N_VNew_Serial],,
    [AS_IF([test "x$with_cvodes" != xcheck],
       [AC_MSG_FAILURE([--with-cvodes was given, but test for sundials nvecserial libray failed])],
       [AC_MSG_WARN([sundials nvecserial library (libsundials-nvecserial) not found])])
    ])
  ])

# check if we have everything
AS_IF([test "x$ac_cv_lib_sundials_cvodes_CVodeInit" = xyes -a "x$ac_cv_header_sundials_sundials_types_h" = xyes -a "x$ac_cv_lib_sundials_nvecserial_N_VNew_Serial" = xyes -a "x$ac_cv_header_nvector_nvector_serial_h" = xyes ],
  [
   cvodes=1
   #AS_IF([test "x`which sundials-config`" != x],
           #[ida_include=`sundials-config -lc -mida -ts | grep I | awk '{print [$]1}' | cut -c3-`
            #ida_version=`cat ${ida_include}/sundials/sundials_config.h | grep VERSION | awk '{print [$]3}' | sed s/\"//g`],
         #[ida_version=unknown])
   cvodes_version=unknown
   AC_DEFINE(HAVE_CVODES)
  ],[
   cvodes=0
  ])
])


AC_DEFUN([WASORA_RESUME_CVODES],[
if [[ $cvodes -eq 1 ]]; then
  cvodes_message="yes, version ${cvodes_version}"
  cvodes_not=""
else
  cvodes_message="no"
  cvodes_not=" NOT"
fi
echo "  CVODES library (optional): ${cvodes_message}"
echo "    Systems of ODEs will${cvodes_not} be solved"
echo
])

dnl This file is part of wasora and/or one of its plugins
dnl GPL v3+ (c) 2009-2018 jeremy theler
dnl <https://bitbucket.org/seamplex/wasora>
dnl
AC_DEFUN([WASORA_CHECK_PETSC],[

petsc_required=m4_default([$1],[optional])

# petsc
AC_ARG_VAR(PETSC_DIR, [location of PETSc installation])
AC_ARG_VAR(PETSC_ARCH, [PETSc architecture])

# asumimos que la encontramos
petsc_found=1

AC_MSG_CHECKING([for PETSc dir])
AS_IF([ test -z "$PETSC_DIR" ], [
    AC_MSG_RESULT([empty, trying /usr/lib/petsc])
    export PETSC_DIR=/usr/lib/petsc
  ])
AS_IF([ test ! -d "$PETSC_DIR" ], [
    AC_MSG_RESULT([no])
    AS_IF([ test "x${petsc_required}" = "xrequired" ], [
      AC_MSG_ERROR([PETSc not found; PETSC_DIR=$PETSC_DIR does not exist.
Do you have a working PETSc installation?])
    ])
    petsc_found=0
  ], [ test ! -d "$PETSC_DIR/include" ], [
    AC_MSG_RESULT([broken])
    AS_IF([ test "x${petsc_required}" = "xrequired" ], [
      AC_MSG_ERROR([PETSc include dir $PETSC_DIR/include not found; check PETSC_DIR])
    ])
    petsc_found=0
  ], [ test ! -f "$PETSC_DIR/include/petscversion.h" ], [
    AC_MSG_RESULT([broken])
    AS_IF([ test "x${petsc_required}" = "xrequired" ], [
      AC_MSG_ERROR([PETSc header file $PETSC_DIR/include/petscversion.h not found; check PETSC_DIR])
    ])
    petsc_found=0
  ],[
    AC_MSG_RESULT([$PETSC_DIR])
  ])


AC_MSG_CHECKING([for PETSc arch])
AS_IF([ test ! -e "${PETSC_DIR}/${PETSC_ARCH}/lib/petsc/conf/petscvariables" ], [
    AC_MSG_RESULT([broken])
    AS_IF([ test "x${petsc_required}" = "xrequired" ], [
      AC_MSG_ERROR([cannot find ${PETSC_DIR}/${PETSC_ARCH}/lib/petsc/conf/petscvariables.
Do you have a working PETSc > 3.6.0 installation?])
    ])
    petsc_found=0
  ],[
    AC_MSG_RESULT([$PETSC_ARCH])
  ])

AS_IF([ test ${petsc_found} -eq 1 ], [
  AC_DEFINE(HAVE_PETSC)
])
])


AC_DEFUN([WASORA_RESUME_PETSC],[
  AS_IF([ test ${petsc_found} -eq 1 ], [
    message="yes, ${PETSC_DIR}/${PETSC_ARCH}"
    not=""
  ],[
    message="no"
    not=" NOT"
  ])
  echo "        PETSc (${petsc_required}): ${message}"
  echo "    systems of equations represented by sparse matrices will${not} be solved"
  echo
])


AC_DEFUN([WASORA_CHECK_SLEPC],[
slepc_required=m4_default([$1],[optional])

# asumimos que la encontramos
slepc_found=1

# slepc
AC_ARG_VAR(SLEPC_DIR, [location of SLEPc installation])

AC_MSG_CHECKING([for SLEPC_DIR environment variable])
AC_ARG_VAR(SLEPC_DIR, [location of SLEPc installation])

AC_MSG_CHECKING([for SLEPc dir])
AS_IF([ test -z "$SLEPC_DIR" ], [
    AC_MSG_RESULT([empty, trying /usr/lib/slepc])
    export SLEPC_DIR=/usr/lib/petsc
  ])
AS_IF([ test ! -d "$SLEPC_DIR" ], [
    AC_MSG_RESULT([no])
    AS_IF([ test "x${slepc_required}" = "xrequired" ], [
      AC_MSG_ERROR([SLEPc not found; SLEPC_DIR=$SLEPC_DIR does not exist])
    ])
    slepc_found=0
  ], [ test ! -d "$SLEPC_DIR/include" ], [
    AC_MSG_RESULT([broken])
    AS_IF([ test "x${slepc_required}" = "xrequired" ], [
      AC_MSG_ERROR([SLEPc include dir $SLEPC_DIR/include not found; check SLEPC_DIR])
    ])
    slepc_found=0
  ], [ test ! -f "$SLEPC_DIR/include/slepcversion.h" ], [
    AC_MSG_RESULT([broken])
    AS_IF([ test "x${slepc_required}" = "xrequired" ], [
      AC_MSG_ERROR([SLEPc header file $SLEPC_DIR/include/slepcversion.h not found; check SLEPC_DIR])
    ])
    slepc_found=0
  ],[
    AC_MSG_RESULT([$SLEPC_DIR])
  ])

AS_IF([ test ${slepc_found} -eq 1 ], [
  AC_DEFINE(HAVE_SLEPC)
])
])


AC_DEFUN([WASORA_RESUME_SLEPC],[
  AS_IF([ test ${slepc_found} -eq 1 ], [
    message="yes, ${SLEPC_DIR} (${PETSC_ARCH})"
    not=""
  ],[
    message="no"
    not=" NOT"
  ])
  echo "        SLEPc (${slepc_required}): ${message}"
  echo "    sparse generealized eigenproblems will${not} be solved"
  echo
])

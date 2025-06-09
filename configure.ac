AC_PREREQ([2.63])
AC_INIT([uniset], [0.1], [depp@zdome.net])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/token.h])
AC_CONFIG_FILES([Makefile])

m4_version_prereq([2.70], [AC_PROG_CC], [AC_PROG_CC_C99])
AM_PROG_CC_C_O
AS_VAR_IF([ac_cv_prog_cc_c99], [no],
      [AC_MSG_ERROR([A C99 compiler or better is required])])

ENABLE_WARNINGS

AC_OUTPUT

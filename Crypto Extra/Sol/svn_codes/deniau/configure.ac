dnl Process this file with autoconf to produce a configure script.

AC_INIT([dpaContestNativeReference], [0.1], [thomas.deniau@polytechnique.org])

AC_PROG_CXX

AC_PATH_PROG(PERL, perl, "NOT-FOUND")
if test "x${ac_cv_path_PERL}" = "xNOT-FOUND" ; then
        AC_MSG_ERROR(Perl interpreter not found; unable to continue)
fi
AC_SUBST(PERL)

AM_INIT_AUTOMAKE

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_PATH_PYTHON(2.4)
AC_PROG_SWIG(1.3.21)
SWIG_ENABLE_CXX
SWIG_PYTHON

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
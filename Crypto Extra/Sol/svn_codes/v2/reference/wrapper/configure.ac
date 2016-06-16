AC_PREREQ(2.61)
AC_INIT([attack_wrapper], [1.1.1], [contact@dpacontest.org])
AC_CONFIG_SRCDIR([src/attack_wrapper.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall foreign])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gettimeofday memset strdup])

#CT_CHECK_POSTGRES_DB
AX_LIB_POSTGRESQL
AX_PTHREAD

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

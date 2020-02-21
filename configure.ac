# Init and package info
AC_INIT(muze, 0.1)
AC_LANG(C)
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_AUX_DIR([tools/])

# Set variables
INCLUDE_DIR="include/"
AC_SUBST(INCLUDE_DIR)

# Check for programs
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Check headers
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
AC_CHECK_HEADER_STDBOOL

# Check for types
AC_TYPE_PID_T

# Check for library functions
AC_CHECK_FUNCS([dup2 memset pow strtol])
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_PREREQ

# Create the Makefile
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

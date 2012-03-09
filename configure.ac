#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT
AC_PREREQ(2.53)
AM_INIT_AUTOMAKE(fivi, 0.2)
dnl AC_REVISION ?
AM_MAINTAINER_MODE

AC_GNU_SOURCE
AM_CONFIG_HEADER(config.h)

# Checks for programs.

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_SYS_LARGEFILE

# Checks for libraries.

PKG_CHECK_MODULES(GLIB, glib-2.0)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(assert.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS(string.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(dup2)
AC_CHECK_FUNCS(select)
AC_CHECK_FUNCS(strdup)
AC_CHECK_FUNCS(strerror)

AC_MSG_CHECKING([for printf("%c") return values])
AC_TRY_RUN([
#include <stdio.h>
int main( int argc, char *argv[] )
{
	return printf( "%c",  ' ' ) != 1;
}
]
,AC_MSG_RESULT([printf])
,AC_MSG_RESULT([putchar])
AC_MSG_ERROR([]))

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

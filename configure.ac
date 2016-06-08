#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([drone-connect], [1.0], [christian.ehrig@fom-net.de])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE
LT_INIT

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
PKG_CHECK_MODULES(PTHREAD_STUBS,pthread-stubs)

PKG_CHECK_MODULES(BLUEZ,bluez)

AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_SEARCH_LIBS([hci_get_route], [bluetooth])
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_CHECK_FUNCS([dup2])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

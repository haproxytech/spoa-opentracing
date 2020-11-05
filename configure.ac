dnl Process this file with autoconf to produce a configure script.
dnl
dnl configure.ac - Miroslav Zagorac <mzagorac@haproxy.com>

AC_PREREQ([2.69])
AC_COPYRIGHT([Copyright (C) 2020 HAProxy Technologies])
AC_INIT(m4_esyscmd_s([basename ${PWD}]), m4_esyscmd_s([cat VERSION]), [mzagorac@haproxy.com])

echo "*** configuring for ${PACKAGE_NAME} v${PACKAGE_VERSION} ***"

AC_PREFIX_DEFAULT([/usr])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.13])
AX_VARIABLES_INIT
AM_SILENT_RULES([yes])
AX_HOST

dnl Check for source directory.
dnl
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config/config.h])

AX_PROG_PKGCONFIG

dnl Checking command line options.
dnl
AX_ENABLE_DEBUG
AX_ENABLE_GPROF
AX_ENABLE_THREADS
dnl
dnl Misc
dnl
AX_WITH_LIBEV
if test "${enable_debug}" = "yes"; then
	AX_WITH_OPENTRACING([_dbg])
else
	AX_WITH_OPENTRACING([])
fi

dnl Checks for programs.
dnl
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL

dnl This macro is needed only for automake <1.14 (CentOS 7).
dnl
AM_PROG_CC_C_O

dnl Checks for header files.
dnl
AC_CHECK_HEADERS([fcntl.h libgen.h stddef.h stdlib.h unistd.h])
AC_CHECK_HEADERS([inttypes.h limits.h stdint.h string.h strings.h])
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h])
AC_CHECK_HEADERS([sys/param.h sys/socket.h sys/time.h])

dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AX_CHECK___DATE__
AX_CHECK___FUNC__
AX_CHECK___STDC__
AX_CHECK___ATTRIBUTE__
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AX_TYPE_BOOL_T
AX_TYPE_UNCHAR
AX_TYPE_USHORT
AX_TYPE_ULONG
AX_TYPE_UINT

dnl Checks for libraries.
dnl
AC_SEARCH_LIBS([socket], [socket])

dnl Checks for library functions.
dnl
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([clock_gettime gettimeofday localtime_r])
AC_CHECK_FUNCS([memchr memmove memset])
AC_CHECK_FUNCS([dup2 socket])
AC_CHECK_FUNCS([strcasecmp strncasecmp strchr strdup strndup strerror strtoll strtoull])

dnl Set the compiler flags
dnl
AX_PROG_CC_SET([-fno-strict-aliasing])

AM_CONDITIONAL([WANT_DEBUG],       [test "${enable_debug}" = "yes"])
AM_CONDITIONAL([WANT_LIBEV],       [test "${HAVE_LIBEV}" = "yes"])
AM_CONDITIONAL([WANT_OPENTRACING], [test "${HAVE_OPENTRACING}" = "yes"])
AM_CONDITIONAL([WANT_THREADS],     [test "${HAVE_THREADS}" = "yes"])

AX_VARIABLES_SET

AX_VARIABLE_SET([SPOA_OPENTRACING_CPPFLAGS], [ ${LIBEV_CPPFLAGS} ${OPENTRACING_CPPFLAGS} ${THREADS_CPPFLAGS} ])
AX_VARIABLE_SET([SPOA_OPENTRACING_CFLAGS],   [ ${LIBEV_CFLAGS}   ${OPENTRACING_CFLAGS}   ${THREADS_CFLAGS}   ])
AX_VARIABLE_SET([SPOA_OPENTRACING_LDFLAGS],  [ ${LIBEV_LDFLAGS}  ${OPENTRACING_LDFLAGS}  ${THREADS_LDFLAGS}  ])
AX_VARIABLE_SET([SPOA_OPENTRACING_LIBS],     [ ${LIBEV_LIBS}     ${OPENTRACING_LIBS}     ${THREADS_LIBS}     ])

AC_SUBST([SPOA_OPENTRACING_CPPFLAGS])
AC_SUBST([SPOA_OPENTRACING_CFLAGS])
AC_SUBST([SPOA_OPENTRACING_LDFLAGS])
AC_SUBST([SPOA_OPENTRACING_LIBS])

dnl version.h
dnl
CONFIGURE_OPTIONS="${ac_configure_args}"
AC_SUBST([DATE])
AC_SUBST([PACKAGE_VERSION])
AC_SUBST([CONFIGURE_OPTIONS])

AC_CONFIG_FILES([Makefile src/Makefile include/common/version.h])
AC_OUTPUT

AX_SHOW_CONFIG


AH_TOP([
#ifndef _COMMON_CONFIG_H
#define _COMMON_CONFIG_H
])

AH_BOTTOM([
#endif /* _COMMON_CONFIG_H */
])

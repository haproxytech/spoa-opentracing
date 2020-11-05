dnl am-with-opentracing-c-wrapper.m4 by Miroslav Zagorac <mzagorac@haproxy.com>
dnl
AC_DEFUN([AX_WITH_OPENTRACING], [
	AC_ARG_WITH([opentracing],
		[AS_HELP_STRING([--with-opentracing@<:@=DIR@:>@], [use OPENTRACING library @<:@default=check@:>@])],
		[with_opentracing="${withval}"],
		[with_opentracing=check]
	)

	if test "${with_opentracing}" != "no"; then
		HAVE_OPENTRACING=
		OPENTRACING_CFLAGS=
		OPENTRACING_CPPFLAGS="-DHAVE_LIBOPENTRACING"
		OPENTRACING_LDFLAGS=
		OPENTRACING_LIBS=

		typeset _var_pkg="opentracing-c-wrapper$1"

		AX_PATH_PKGCONFIG([${with_opentracing}])
		if `PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --exists ${_var_pkg}`; then
			OPENTRACING_CPPFLAGS="${OPENTRACING_CPPFLAGS}${OPENTRACING_CPPFLAGS:+ }`PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --cflags ${_var_pkg}`"
			OPENTRACING_LDFLAGS="${OPENTRACING_LDFLAGS}${OPENTRACING_LDFLAGS:+ }`PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --libs-only-L ${_var_pkg}`"
			OPENTRACING_LDFLAGS="${OPENTRACING_LDFLAGS} `PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --libs-only-other ${_var_pkg}`"
			OPENTRACING_LIBS="${OPENTRACING_LIBS}${OPENTRACING_LIBS:+ }`PKG_CONFIG_PATH=${PKG_CONFIG_PATH} pkg-config --libs-only-l ${_var_pkg}`"
		fi

		AX_VARIABLES_STORE
		AX_GCC_SET_STDINC

		LDFLAGS="${LDFLAGS} ${OPENTRACING_LDFLAGS}"
		CPPFLAGS="${CPPFLAGS} ${OPENTRACING_CPPFLAGS}"

		AC_CHECK_LIB([opentracing-c-wrapper], [otc_tracer_init], [], [AC_MSG_ERROR([OPENTRACING library not found])])
		AC_CHECK_HEADER([opentracing-c-wrapper/define.h], [], [AC_MSG_ERROR([OPENTRACING library headers not found])])

		HAVE_OPENTRACING=yes

		AX_VARIABLES_RESTORE

		AC_MSG_NOTICE([OPENTRACING environment variables:])
		AC_MSG_NOTICE([  OPENTRACING_CFLAGS=${OPENTRACING_CFLAGS}])
		AC_MSG_NOTICE([  OPENTRACING_CPPFLAGS=${OPENTRACING_CPPFLAGS}])
		AC_MSG_NOTICE([  OPENTRACING_LDFLAGS=${OPENTRACING_LDFLAGS}])
		AC_MSG_NOTICE([  OPENTRACING_LIBS=${OPENTRACING_LIBS}])

		AC_SUBST([OPENTRACING_CFLAGS])
		AC_SUBST([OPENTRACING_CPPFLAGS])
		AC_SUBST([OPENTRACING_LDFLAGS])
		AC_SUBST([OPENTRACING_LIBS])
	fi
])

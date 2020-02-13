AC_PREREQ(2.60)
AC_INIT(connman2, 0.0)

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_MACRO_DIR(build-aux)

AM_INIT_AUTOMAKE([foreign subdir-objects color-tests silent-rules
                                       tar-pax no-dist-gzip dist-xz])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE

AC_PREFIX_DEFAULT(/usr/local)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_LN_S

LT_PREREQ(2.2)
LT_INIT([disable-static])

AC_CHECK_FUNCS(explicit_bzero)

AC_CHECK_HEADERS(linux/types.h linux/if_alg.h)

AC_ARG_ENABLE([external_ell], AC_HELP_STRING([--enable-external-ell],
				[enable external Embedded Linux library]),
					[enable_external_ell=${enableval}])
if (test "${enable_external_ell}" = "yes"); then
	if (test "${enable_daemon}" != "no" ||
			test "${enable_client}" != "no" ||
			test "${enable_monitor}" != "no" ||
			test "${enable_wired}" = "yes" ||
			test "${enable_hwsim}" = "yes"); then
		ell_min_version="0.27"
	else
		ell_min_version="0.5"
	fi
	PKG_CHECK_MODULES(ELL, ell >= ${ell_min_version}, dummy=yes,
		AC_MSG_ERROR(Embedded Linux library >= ${ell_min_version} is required))
	AC_SUBST(ELL_CFLAGS)
	AC_SUBST(ELL_LIBS)
fi
AM_CONDITIONAL(EXTERNAL_ELL, test "${enable_external_ell}" = "yes")

AC_CONFIG_FILES(Makefile)

AC_OUTPUT

AC_MSG_RESULT([
	$PACKAGE_NAME $VERSION
])

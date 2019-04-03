m4_define([LIBUL_MAJOR],   1)
m4_define([LIBUL_MINOR],   1)
m4_define([LIBUL_MICRO], 1)
m4_define([LIBUL_RC],)

AC_INIT([libuldaq],[LIBUL_MAJOR[.]LIBUL_MINOR[.]LIBUL_MICRO[]LIBUL_RC],[info@mccdaq.com],[libuldaq],[http://www.mccdaq.com])

# Library versioning
# These numbers should be tweaked on every release. Read carefully:
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# http://sourceware.org/autobook/autobook/autobook_91.html
lt_current=2
lt_revision=1
lt_age=1
LTLDFLAGS="-version-info ${lt_current}:${lt_revision}:${lt_age}"

CFLAGS='-O3'
CXXFLAGS='-O3'

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PREREQ([2.69])
AC_PROG_CXX
AC_PROG_CC
LT_INIT

libul_lib_error() {
	echo ""
	echo "  Library $1 was not found on this system."
	echo "  Please install it and re-run ./configure"
	echo ""
	exit 1
}

AC_MSG_CHECKING([operating system])
AC_MSG_RESULT($host)
case $host in
*-linux*)
	AC_MSG_RESULT([ Linux])
	AC_DEFINE(OS_LINUX, 1, [Linux implementations])
	AC_SUBST(OS_LINUX)
	backend="linux"
	os="linux"
	;;
*-darwin*)
	AC_MSG_RESULT([ Mac OS X])
	AC_DEFINE(OS_DARWIN, 1, [Mac implementation])
	AC_SUBST(OS_DARWIN)
	backend="mac"
	os="darwin"
	LIBS="${LIBS} -framework IOKit -framework CoreFoundation"
	;;
*)
	AC_MSG_ERROR([UL for Linux is not supported on your operating system yet])
esac

# Checks for libraries.
AC_CHECK_LIB([usb-1.0], [libusb_init], [], [libul_lib_error libusb-1.0])

AC_CHECK_HEADERS([libusb-1.0/libusb.h], [], [libul_lib_error libusb-1.0])

# OS info for Automake
AM_CONDITIONAL(OS_LINUX, test "x$os" = xlinux)
AM_CONDITIONAL(OS_DARWIN, test "x$os" = xdarwin)

# Examples build
AC_ARG_ENABLE([disable-examples-build], [AS_HELP_STRING([--disable-examples-build],
	[disable building example applications [default=no]])],
	[disable_build_examples=$enableval],
	[disable_build_examples=no])
AM_CONDITIONAL(BUILD_EXAMPLES, test "x$disable_build_examples" != xyes)
# Debug enable
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--debug],
	[turn on debugging [default=no]])],
	[debug=$enableval],
	[debug=no])
AM_CONDITIONAL(DEBUG, test "x$debug" != xno)

AC_SUBST(LTLDFLAGS)

AC_CONFIG_FILES([libuldaq.pc])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([examples/Makefile])
               
AC_OUTPUT


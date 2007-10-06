#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([LiveMix], [0.45svn], [stephane.brunner@gmail.com])

# Checks for programs.
AC_PROG_CXX
#AC_PROG_MOC

# Checks for libraries.
#AC_CHECK_LIB(lrdf,main,[LRDF_SUPPORT=yes],[LRDF_SUPPORT=no],-llrdf)

# Checks for header files.
AC_CHECK_HEADERS([sys/time.h jack/jack.h lrdf.h ladspa.h])
#AC_CHECK_HEADERS([sys/time.h jack/jack.h ladspa.h])

#PKG_CHECK_MODULES(QT, [QtCore >= 4.1.0 QtGui >= 4.1.0 QtXml >= 4.1.0 QtSvg >= 4.1.0])
#PKG_CHECK_MODULES(JACK, jack >= 0.100)
#PKG_CHECK_MODULES(LRDF, lrdf >= 0.4)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_HEADER_TIME

LRDF_SUPPORT=yes
LADSPA_SUPPORT=yes

AC_CONFIG_FILES([features.pri])
AC_OUTPUT


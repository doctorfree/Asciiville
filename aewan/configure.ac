#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([aewan])
AC_CONFIG_SRCDIR([aewan.c])
#AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Check for the standard stdbool header file
AC_HEADER_STDBOOL

# Checks for libraries.
AC_CHECK_LIB([ncurses], [initscr], [],
             AC_MSG_ERROR([Can't find ncurses library. Install it first.]))
AC_CHECK_LIB([z], [gzopen], [],
             AC_MSG_ERROR([Can't find zlib library. Install it first.]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([zlib.h fcntl.h limits.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T

# Checks for library functions.
AC_CHECK_FUNCS([atexit strdup strerror])

# Checks for cygwin
AC_MSG_CHECKING([for cygwin OSTYPE])
if test x$OSTYPE == xcygwin; then 
   AC_SUBST(EXESUF, [.exe])
   AC_MSG_RESULT([it is cygwin, using .exe])
else
   AC_MSG_RESULT([not cygwin])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT


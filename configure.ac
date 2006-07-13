AC_PREREQ(2.50)
AC_INIT([wendy], [0.0.7], [elmindreda@elmindreda.org])
AC_CONFIG_SRCDIR([src/Core.cpp])
AC_CONFIG_AUX_DIR(build/autotools)
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([check-news dist-zip no-define])
AM_CONFIG_HEADER([include/config.h])

# Check for required programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_MAKE_SET
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

# Check for system header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADER([GL/glfw.h], [], [AC_MSG_ERROR([GLFW not found])])
AC_CHECK_HEADER([GL/glew.h], [], [AC_MSG_ERROR([GLEW not found])])

# Check for platform profile and set up OpenGL build flags.
AC_MSG_CHECKING([for a suitable OpenGL profile])
case $host in
  *-*-mingw32* | *-*-cygwin*)
    AC_MSG_RESULT([Win32/WGL])
    AC_DEFINE([WENDY_WIN32], [1], [Build on a Win32 system.])
    LIBS="-mwin32 -lopengl32 -lglu32 $LIBS"
    ;;
  *-*-darwin* | *-*-macos10*)
    AC_MSG_RESULT([Carbon/AGL])
    AC_DEFINE([WENDY_MACOSX], [1], [Build on a Mac OS X system.])
    LIBS="-framework OpenGL -framework AGL -framework Carbon $LIBS"
    ;;
  *)
    AC_MSG_RESULT([POSIX/GLX (default)])
    AC_DEFINE([WENDY_POSIX], [1], [Build on a POSIX system.])
    AC_PATH_X
    if test "$have_x" = "no"; then
      AC_MSG_ERROR([X11 not found])
    fi
    if test -n "$ac_x_includes"; then
      CFLAGS="-I$ac_x_includes $CFLAGS"
    fi
    if test -n "$ac_x_libraries"; then
      LIBS="-L$ac_x_libraries $LIBS"
    fi
    AC_CHECK_LIB([X11], [XCreateWindow], [], [AC_MSG_ERROR([X11 GLX not found])])
    AC_CHECK_LIB([GL], [glGetString], [], [AC_MSG_ERROR([OpenGL not found])])
    AC_CHECK_LIB([GLU], [gluGetString], [], [AC_MSG_ERROR([GLU not found])])
    AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR([POSIX threads not found])])
    AC_CHECK_LIB([Xxf86vm], [XF86VidModeQueryExtension])
    ;;
esac

# Check for required GL interface libraries.
AC_CHECK_LIB([glfw], [glfwInit], [], [AC_MSG_ERROR([GLFW not found])])
AC_CHECK_LIB([GLEW], [glewInit], [], [AC_MSG_ERROR([GLEW not found])])

# Check for moira configuration.
PKG_CHECK_MODULES(moira, libmoira >= 0.0.2)

WENDY_CFLAGS="$libsigc_CFLAGS $moira_CFLAGS"
AC_SUBST(WENDY_CFLAGS)
WENDY_LIBS="$libsigc_LIBS $moira_LIBS"
AC_SUBST(WENDY_LIBS)

AC_OUTPUT(
  Makefile
  build/Makefile
  build/libwendy.pc
  docs/Makefile
  docs/Doxyfile
  examples/Makefile
  examples/fonts/Makefile
  examples/simple/Makefile
  examples/widgets/Makefile
  include/Makefile
  include/wendy/Makefile
  src/Makefile
  tools/Makefile)


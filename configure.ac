AC_PREREQ(2.50)
AC_INIT([wendy], [0.1.0], [elmindreda@elmindreda.org])
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

# Check for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADER([GL/glew.h], [], [AC_MSG_ERROR([GLEW not found])])
AC_CHECK_HEADER([AL/al.h], [], [AC_MSG_ERROR([OpenAL not found])])
AC_CHECK_HEADER([AL/alut.h], [], [AC_MSG_ERROR([ALUT not found])])

# Check for libraries.
AC_CHECK_LIB([GLEW], [glewInit], [], [AC_MSG_ERROR([GLEW not found])])
AC_CHECK_LIB([openal], [alEnable], [], [AC_MSG_ERROR([OpenAL not found])])
AC_CHECK_LIB([alut], [alutInit], [], [AC_MSG_ERROR([ALUT not found])])

# Check for packages.
PKG_CHECK_MODULES(moira, libmoira >= 0.0.2)
PKG_CHECK_MODULES(glfw, libglfw >= 2.6)
PKG_CHECK_MODULES(ogg, ogg >= 1.1.0)
PKG_CHECK_MODULES(vorbis, vorbis >= 1.1.0)

WENDY_CFLAGS="$glfw_CFLAGS $moira_CFLAGS $vorbis_CFLAGS $ogg_CFLAGS"
AC_SUBST(WENDY_CFLAGS)
WENDY_LIBS="$glfw_LIBS $moira_LIBS $vorbis_LIBS $ogg_LIBS"
AC_SUBST(WENDY_LIBS)

AC_OUTPUT(
  Makefile
  build/Makefile
  build/libwendy.pc
  docs/Makefile
  docs/Doxyfile
  examples/Makefile
  examples/editor/Makefile
  examples/fonts/Makefile
  examples/program/Makefile
  examples/shadow/Makefile
  examples/simple/Makefile
  examples/widgets/Makefile
  include/Makefile
  include/wendy/Makefile
  src/Makefile
  tools/Makefile)


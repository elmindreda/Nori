
project(glfw C)

find_package(OpenGL REQUIRED)

include_directories(${glfw_SOURCE_DIR})

set(glfw_SOURCES enable.c fullscreen.c glext.c image.c init.c input.c joystick.c
                 stream.c tga.c thread.c time.c window.c)

if(UNIX AND NOT APPLE AND NOT CYGWIN)

  include(${glfw_SOURCE_DIR}/CMake/CHECK_X11_EXTENSIONS.cmake)

  set(CMAKE_REQUIRED_LIBRARIES ${X11_X11_LIB} ${OPENGL_gl_LIBRARY})
  list(APPEND GLFW_INCLUDE_DIR ${X11_X11_INCLUDE_PATH})
  list(APPEND GLFW_LIBRARIES ${X11_X11_LIB})
  list(APPEND GLFW_INCLUDE_DIR ${OPENGL_INCLUDE_DIR})
  list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

  CHECK_X11_XRANDR()
  if (X11_XRANDR_FOUND)
    add_definitions(-D_GLFW_HAS_XRANDR) 
    list(APPEND GLFW_INCLUDE_DIR ${X11_XRANDR_INCLUDE_DIR})
    list(APPEND GLFW_LIBRARIES ${X11_XRANDR_LIBRARIES})
  endif(X11_XRANDR_FOUND)

  if (NOT X11_XRANDR_FOUND)
    CHECK_X11_XF86VIDMODE()
    if (X11_XF86VIDMODE_FOUND)
      add_definitions(-D_GLFW_HAS_XF86VIDMODE)
      list(APPEND GLFW_INCLUDE_DIR ${X11_XF86VIDMODE_INCLUDE_DIR})
      list(APPEND GLFW_LIBRARIES ${X11_XF86VIDMODE_LIBRARIES})
    endif(X11_XF86VIDMODE_FOUND) 
  endif (NOT X11_XRANDR_FOUND)

  list(APPEND glfw_SOURCES x11_enable.c x11_fullscreen.c x11_glext.c x11_init.c
                           x11_joystick.c x11_keysym2unicode.c x11_thread.c
                           x11_time.c x11_window.c)

endif(UNIX AND NOT APPLE AND NOT CYGWIN)

add_library(glfw STATIC ${glfw_SOURCES})


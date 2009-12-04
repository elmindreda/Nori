
project(glfw C)

set(glfw_SOURCES enable.c fullscreen.c glext.c image.c init.c input.c joystick.c
                 stream.c tga.c thread.c time.c window.c)

if(UNIX AND NOT APPLE AND NOT CYGWIN)

  set(CMAKE_REQUIRED_LIBRARIES ${X11_X11_LIB} ${OPENGL_gl_LIBRARY})
  list(APPEND GLFW_INCLUDE_DIRS ${X11_X11_INCLUDE_PATH})
  list(APPEND GLFW_LIBRARIES ${X11_X11_LIB})
  list(APPEND GLFW_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

  include(${glfw_SOURCE_DIR}/CMake/CHECK_X11_EXTENSIONS.cmake)

  CHECK_X11_XRANDR()
  if (X11_XRANDR_FOUND)
    add_definitions(-D_GLFW_HAS_XRANDR) 
    list(APPEND GLFW_INCLUDE_DIRS ${X11_XRANDR_INCLUDE_DIR})
    list(APPEND GLFW_LIBRARIES ${X11_XRANDR_LIBRARIES})
  endif(X11_XRANDR_FOUND)

  if (NOT X11_XRANDR_FOUND)
    CHECK_X11_XF86VIDMODE()
    if (X11_XF86VIDMODE_FOUND)
      add_definitions(-D_GLFW_HAS_XF86VIDMODE)
      list(APPEND GLFW_INCLUDE_DIRS ${X11_XF86VIDMODE_INCLUDE_DIR})
      list(APPEND GLFW_LIBRARIES ${X11_XF86VIDMODE_LIBRARIES})
    endif(X11_XF86VIDMODE_FOUND) 
  endif (NOT X11_XRANDR_FOUND)

  include(CheckFunctionExists)

  check_function_exists(glXGetProcAddress GLXGETPROCADDRESS)
  if(GLXGETPROCADDRESS)
    add_definitions(-D_GLFW_HAS_GLXGETPROCADDRESS)
  else()

    check_function_exists(glXGetProcAddressARB GLXGETPROCADDRESSARB)

    if(GLXGETPROCADDRESSARB)
      add_definitions(-D_GLFW_HAS_GLXGETPROCADDRESSARB)
    else()
      check_function_exists(glXGetProcAddressEXT GLXGETPROCADDRESSEXT)

      if(GLXGETPROCADDRESSEXT)
        add_definitions(-D_GLFW_HAS_GLXGETPROCADDRESSEXT)
      endif(GLXGETPROCADDRESSEXT)
    endif(GLXGETPROCADDRESSARB)

  endif(GLXGETPROCADDRESS)

  list(APPEND glfw_SOURCES x11_enable.c x11_fullscreen.c x11_glext.c x11_init.c
                           x11_joystick.c x11_keysym2unicode.c x11_thread.c
                           x11_time.c x11_window.c)

elseif(WIN32)

  set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})
  list(APPEND GLFW_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

  list(APPEND glfw_SOURCES win32_dllmain.c win32_enable.c win32_fullscreen.c
                           win32_glext.c win32_init.c win32_joystick.c
                           win32_thread.c win32_time.c win32_window.c)

endif(UNIX AND NOT APPLE AND NOT CYGWIN)

include_directories(${GLFW_INCLUDE_DIRS})

add_library(glfw STATIC ${glfw_SOURCES})

set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIRS} CACHE STRING "GLFW include directories")
set(GLFW_LIBRARIES ${GLFW_LIBRARIES} CACHE STRING "GLFW libraries")


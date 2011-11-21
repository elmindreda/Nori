
project(glfw C)

set(glfw_SOURCES enable.c fullscreen.c glext.c init.c input.c joystick.c
                 time.c window.c)

if(APPLE)

  list(APPEND GLFW_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

  find_library(COCOA_FRAMEWORK Cocoa)
  list(APPEND GLFW_LIBRARIES ${COCOA_FRAMEWORK})

  list(APPEND glfw_SOURCES cocoa_enable.m cocoa_fullscreen.m cocoa_glext.m
                           cocoa_init.m cocoa_joystick.m cocoa_time.m cocoa_window.m)

endif(APPLE)

if(UNIX AND NOT APPLE)

  set(CMAKE_REQUIRED_LIBRARIES ${X11_X11_LIB} ${OPENGL_gl_LIBRARY})
  list(APPEND GLFW_INCLUDE_DIRS ${X11_X11_INCLUDE_PATH})
  list(APPEND GLFW_LIBRARIES ${X11_X11_LIB})
  list(APPEND GLFW_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

  include(${glfw_SOURCE_DIR}/CMake/CHECK_X11_EXTENSIONS.cmake)

  check_x11_xrandr()
  if (X11_XRANDR_FOUND)
    add_definitions(-D_GLFW_HAS_XRANDR) 
    list(APPEND GLFW_INCLUDE_DIRS ${X11_XRANDR_INCLUDE_DIR})
    list(APPEND GLFW_LIBRARIES ${X11_XRANDR_LIBRARIES})
  endif(X11_XRANDR_FOUND)

  if (NOT X11_XRANDR_FOUND)
    check_x11_xf86vidmode()
    if (X11_XF86VIDMODE_FOUND)
      add_definitions(-D_GLFW_HAS_XF86VIDMODE)
      list(APPEND GLFW_INCLUDE_DIRS ${X11_XF86VIDMODE_INCLUDE_DIR})
      list(APPEND GLFW_LIBRARIES ${X11_XF86VIDMODE_LIBRARIES})
    endif(X11_XF86VIDMODE_FOUND) 
  endif (NOT X11_XRANDR_FOUND)

  include(CheckFunctionExists)

  if (NOT _GLFW_HAS_GLXGETPROCADDRESS)
    check_function_exists(glXGetProcAddressARB _GLFW_HAS_GLXGETPROCADDRESSARB)
  endif (NOT _GLFW_HAS_GLXGETPROCADDRESS)

  if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)
    check_function_exists(glXGetProcAddressEXT _GLFW_HAS_GLXGETPROCADDRESSEXT)
  endif (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)

  if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
      NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
      NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)
    message(WARNING "No glXGetProcAddressXXX variant found")
  endif (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
          NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
          NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)

  find_library(LIBRT rt)
  if (LIBRT)
    list(APPEND GLFW_LIBRARIES ${LIBRT})
  endif(LIBRT)

  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(_GLFW_USE_LINUX_JOYSTICKS 1)
  endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

  list(APPEND glfw_SOURCES x11_enable.c x11_fullscreen.c x11_glext.c x11_init.c
                           x11_joystick.c x11_keysym2unicode.c
                           x11_time.c x11_window.c)

  configure_file(${glfw_SOURCE_DIR}/x11_config.h.cmake 
                 ${glfw_BINARY_DIR}/x11_config.h @ONLY)

endif(UNIX AND NOT APPLE)

if(WIN32)

  set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})
  list(APPEND GLFW_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})
  list(APPEND GLFW_LIBRARIES winmm)

  list(APPEND glfw_SOURCES win32_dllmain.c win32_enable.c win32_fullscreen.c
                           win32_glext.c win32_init.c win32_joystick.c
                           win32_time.c win32_window.c)

endif(WIN32)

include_directories(${GLFW_INCLUDE_DIRS} ${glfw_BINARY_DIR})

add_library(glfw STATIC ${glfw_SOURCES})

set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIRS} CACHE STRING "GLFW include directories")
set(GLFW_LIBRARIES ${GLFW_LIBRARIES} CACHE STRING "GLFW libraries")



project(glfw C)

set(GLFW_VERSION_MAJOR "3")
set(GLFW_VERSION_MINOR "0")
set(GLFW_VERSION_PATCH "0")
set(GLFW_VERSION_EXTRA "")
set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}")
set(GLFW_VERSION_FULL "${GLFW_VERSION}.${GLFW_VERSION_PATCH}${GLFW_VERSION_EXTRA}")

set(glfw_SOURCES ${glfw_SOURCE_DIR}/GL/glfw3.h ${glfw_BINARY_DIR}/config.h
                 internal.h error.c fullscreen.c gamma.c init.c
                 input.c joystick.c opengl.c time.c window.c)

if (APPLE)

  set(_GLFW_COCOA_NSGL 1)

  list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})

  find_library(COCOA_FRAMEWORK Cocoa)
  list(APPEND glfw_LIBRARIES ${COCOA_FRAMEWORK})

  list(APPEND glfw_SOURCES cocoa_platform.h cocoa_clipboard.m cocoa_fullscreen.m
                           cocoa_gamma.m cocoa_init.m cocoa_input.m
                           cocoa_joystick.m cocoa_opengl.m cocoa_time.m
                           cocoa_window.m)

endif()

if (UNIX AND NOT APPLE)

  set(_GLFW_X11_GLX 1)

  set(CMAKE_REQUIRED_LIBRARIES ${X11_X11_LIB} ${OPENGL_gl_LIBRARY})
  list(APPEND glfw_INCLUDE_DIRS ${X11_X11_INCLUDE_PATH})
  list(APPEND glfw_LIBRARIES ${X11_X11_LIB})
  list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})

  include(${glfw_SOURCE_DIR}/CMake/CHECK_X11_EXTENSIONS.cmake)

  if (X11_Xrandr_FOUND)
    set(_GLFW_HAS_XRANDR 1) 
    list(APPEND glfw_INCLUDE_DIRS ${X11_Xrandr_INCLUDE_PATH})
    list(APPEND glfw_LIBRARIES ${X11_Xrandr_LIB})
  endif()

  if (X11_xf86vmode_FOUND)
    set(_GLFW_HAS_XF86VIDMODE 1)
    list(APPEND glfw_INCLUDE_DIRS ${X11_xf86vmode_INCLUDE_PATH})

    if (X11_xf86vmode_LIB)
      list(APPEND glfw_LIBRARIES ${X11_xf86vmode_LIB})
    else()
      list(APPEND glfw_LIBRARIES Xxf86vm)
    endif()
  endif() 

  if (X11_Xkb_FOUND)
    set(_GLFW_HAS_XKB 1)
    list(APPEND glfw_INCLUDE_DIRS ${X11_Xkb_INCLUDE_PATH})
  endif() 

  find_library(RT_LIBRARY rt)
  mark_as_advanced(RT_LIBRARY)
  if (RT_LIBRARY)
    list(APPEND glfw_LIBRARIES ${RT_LIBRARY})
  endif()

  find_library(MATH_LIBRARY m)
  mark_as_advanced(MATH_LIBRARY)
  if (MATH_LIBRARY)
    list(APPEND glfw_LIBRARIES ${MATH_LIBRARY})
  endif()

  include(CheckFunctionExists)

  check_function_exists(glXGetProcAddress _GLFW_HAS_GLXGETPROCADDRESS)

  if (NOT _GLFW_HAS_GLXGETPROCADDRESS)
    check_function_exists(glXGetProcAddressARB _GLFW_HAS_GLXGETPROCADDRESSARB)
  endif()

  if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)
    check_function_exists(glXGetProcAddressEXT _GLFW_HAS_GLXGETPROCADDRESSEXT)
  endif()

  if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
      NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
      NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)
    message(WARNING "No glXGetProcAddressXXX variant found")
  endif()

  if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
    NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
    NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)
    message(WARNING "No glXGetProcAddressXXX variant found")

    find_library(DL_LIBRARY dl)
    mark_as_advanced(DL_LIBRARY)
    if (DL_LIBRARY)
      set(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARY})
    else()
      set(CMAKE_REQUIRED_LIBRARIES "")
    endif()

    check_function_exists(dlopen _GLFW_HAS_DLOPEN)

    if (NOT _GLFW_HAS_DLOPEN)
      message(FATAL_ERROR "No entry point retrieval mechanism found")
    endif()

    if (DL_LIBRARY)
      list(APPEND glfw_LIBRARIES ${DL_LIBRARY})
    endif()
  endif()

  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(_GLFW_USE_LINUX_JOYSTICKS 1)
  endif()

  list(APPEND glfw_SOURCES x11_platform.h x11_clipboard.c x11_fullscreen.c
                           x11_gamma.c x11_init.c x11_input.c x11_joystick.c
                           x11_keysym2unicode.c x11_opengl.c x11_time.c
                           x11_window.c)

endif(UNIX AND NOT APPLE)

if (WIN32)

  set(_GLFW_WIN32_WGL 1)

  set(_GLFW_NO_DLOAD_WINMM 1)

  set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})
  list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})
  list(APPEND glfw_LIBRARIES winmm)

  list(APPEND glfw_SOURCES win32_platform.h win32_clipboard.c win32_dllmain.c
                           win32_fullscreen.c win32_gamma.c win32_init.c
                           win32_input.c win32_joystick.c win32_opengl.c
                           win32_time.c win32_window.c)

endif()

configure_file(${glfw_SOURCE_DIR}/config.h.in 
               ${glfw_BINARY_DIR}/config.h @ONLY)

include_directories(${glfw_INCLUDE_DIRS} ${glfw_BINARY_DIR})

add_library(glfw STATIC ${glfw_SOURCES})

set(GLFW_INCLUDE_DIRS ${glfw_INCLUDE_DIRS} CACHE STRING "GLFW include directories")
set(GLFW_LIBRARIES ${glfw_LIBRARIES} CACHE STRING "GLFW libraries")


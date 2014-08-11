
project(GLFW C)

cmake_minimum_required(VERSION 2.8)

set(GLFW_VERSION_MAJOR "3")
set(GLFW_VERSION_MINOR "0")
set(GLFW_VERSION_PATCH "0")
set(GLFW_VERSION_EXTRA "")
set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}")
set(GLFW_VERSION_FULL "${GLFW_VERSION}.${GLFW_VERSION_PATCH}${GLFW_VERSION_EXTRA}")

find_package(Threads REQUIRED)

set(_GLFW_USE_OPENGL 1)
find_package(OpenGL REQUIRED)

if (WIN32)
  set(_GLFW_WIN32 1)
  set(_GLFW_WGL 1)
elseif (APPLE)
  set(_GLFW_COCOA 1)
  set(_GLFW_NSGL 1)
elseif (UNIX)
  set(_GLFW_X11 1)
  set(_GLFW_GLX 1)
else()
  message(FATAL_ERROR "No supported platform was detected")
endif()

if (_GLFW_WIN32)
  set(_GLFW_NO_DLOAD_WINMM ${BUILD_SHARED_LIBS})
endif()

if (_GLFW_WGL)
  list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})
endif()

if (_GLFW_X11)
  find_package(X11 REQUIRED)

  list(APPEND glfw_INCLUDE_DIRS ${X11_X11_INCLUDE_PATH})
  list(APPEND glfw_LIBRARIES ${X11_X11_LIB} ${CMAKE_THREAD_LIBS_INIT})

  if (NOT X11_Xrandr_FOUND)
    message(FATAL_ERROR "The RandR library and headers were not found")
  endif()

  list(APPEND glfw_INCLUDE_DIRS ${X11_Xrandr_INCLUDE_PATH})
  list(APPEND glfw_LIBRARIES ${X11_Xrandr_LIB})

  if (NOT X11_xf86vmode_FOUND)
    message(FATAL_ERROR "The Xf86VidMode library and headers were not found")
  endif()

  list(APPEND glfw_INCLUDE_DIRS ${X11_xf86vmode_INCLUDE_PATH})

  if (X11_xf86vmode_LIB)
    list(APPEND glfw_LIBRARIES ${X11_xf86vmode_LIB})
  else()
    list(APPEND glfw_LIBRARIES Xxf86vm)
  endif()

  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "The Xinput library and headers were not found")
  endif()

  list(APPEND glfw_INCLUDE_DIRS ${X11_Xi_INCLUDE_PATH})
  list(APPEND glfw_LIBRARIES ${X11_Xi_LIB})

  if (NOT X11_Xcursor_FOUND)
    message(FATAL_ERROR "The Xcursor library and headers were not found")
  endif()

  list(APPEND glfw_INCLUDE_DIRS ${X11_Xcursor_INCLUDE_PATH})
  list(APPEND glfw_LIBRARIES ${X11_Xcursor_LIB})

  if (NOT X11_Xkb_FOUND)
    message(FATAL_ERROR "The X keyboard extension headers were not found")
  endif() 

  list(APPEND glfw_INCLUDE_DIR ${X11_Xkb_INCLUDE_PATH})

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
endif()

if (_GLFW_GLX)
  list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
  list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})

  include(CheckFunctionExists)

  set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})

  check_function_exists(glXGetProcAddress _GLFW_HAS_GLXGETPROCADDRESS)
  if (NOT _GLFW_HAS_GLXGETPROCADDRESS)
    message(FATAL_ERROR "glXGetProcAddress not found")
  endif()
endif()

if (_GLFW_COCOA AND _GLFW_NSGL)
  set(_GLFW_CD_RESOURCES 1)
  find_library(COCOA_FRAMEWORK Cocoa)
  find_library(IOKIT_FRAMEWORK IOKit)
  find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
  list(APPEND glfw_LIBRARIES ${COCOA_FRAMEWORK}
                              ${OPENGL_gl_LIBRARY}
                              ${IOKIT_FRAMEWORK}
                              ${CORE_FOUNDATION_FRAMEWORK})
endif()

set(GLFW_LIBRARIES ${glfw_LIBRARIES} CACHE STRING "Dependencies of GLFW")

add_definitions(-D_GLFW_USE_CONFIG_H)
configure_file(${GLFW_SOURCE_DIR}/glfw_config.h.in 
               ${GLFW_BINARY_DIR}/glfw_config.h @ONLY)

include_directories(${GLFW_SOURCE_DIR}
                    ${GLFW_BINARY_DIR}
                    ${glfw_INCLUDE_DIRS})

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(common_HEADERS GLFW/glfw3.h GLFW/glfw3native.h internal.h)
set(common_SOURCES clipboard.c context.c gamma.c init.c input.c joystick.c
                   monitor.c time.c window.c)

if (_GLFW_COCOA)
    set(glfw_HEADERS ${common_HEADERS} cocoa_platform.h iokit_joystick.h posix_tls.h)
    set(glfw_SOURCES ${common_SOURCES} cocoa_clipboard.m cocoa_gamma.c
                     cocoa_init.m cocoa_monitor.m cocoa_window.m
                     iokit_joystick.m mach_time.c posix_tls.c)
elseif (_GLFW_WIN32)
    set(glfw_HEADERS ${common_HEADERS} win32_platform.h winmm_joystick.h)
    set(glfw_SOURCES ${common_SOURCES} win32_clipboard.c win32_gamma.c
                     win32_init.c win32_monitor.c win32_time.c win32_window.c
                     winmm_joystick.c win32_tls.c)
elseif (_GLFW_X11)
    set(glfw_HEADERS ${common_HEADERS} x11_platform.h linux_joystick.h)
    set(glfw_SOURCES ${common_SOURCES} x11_clipboard.c x11_gamma.c x11_init.c
                     x11_monitor.c x11_window.c x11_unicode.c
                     linux_joystick.c posix_time.c posix_tls.c)
endif()

if (_GLFW_NSGL)
    list(APPEND glfw_HEADERS ${common_HEADERS} nsgl_context.h)
    list(APPEND glfw_SOURCES ${common_SOURCES} nsgl_context.m)
elseif (_GLFW_WGL)
    list(APPEND glfw_HEADERS ${common_HEADERS} wgl_context.h)
    list(APPEND glfw_SOURCES ${common_SOURCES} wgl_context.c)
elseif (_GLFW_X11)
    list(APPEND glfw_HEADERS ${common_HEADERS} glx_context.h)
    list(APPEND glfw_SOURCES ${common_SOURCES} glx_context.c)
endif()

if (APPLE)
    # For some reason, CMake doesn't know about .m
    set_source_files_properties(${glfw_SOURCES} PROPERTIES LANGUAGE C)
endif()

add_library(glfw ${glfw_SOURCES} ${glfw_HEADERS})


cmake_minimum_required(VERSION 2.6)

project(wendy C CXX)
set(VERSION 0.6.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

option(WENDY_INCLUDE_RENDERER "Include the renderer" ON)
option(WENDY_INCLUDE_SCENE_GRAPH "Include the scene graph" ON)
option(WENDY_INCLUDE_UI_SYSTEM "Include the UI library" ON)
option(WENDY_INCLUDE_DEMO_SYSTEM "Include the demo system" ON)
option(WENDY_BUILD_EXAMPLES "Build the bundled example programs" OFF)
option(WENDY_BUILD_TESTS "Build the bundled test programs" OFF)

if(UNIX AND NOT CYGWIN)
  option(WENDY_BUILD_DOCUMENTATION "Build the Doxygen documentation" OFF)
endif(UNIX AND NOT CYGWIN)

include(CheckFunctionExists)
check_function_exists(strtof WENDY_HAVE_STRTOF)
check_function_exists(strlcat WENDY_HAVE_STRLCAT)
check_function_exists(strlcpy WENDY_HAVE_STRLCPY)
check_function_exists(vasprintf WENDY_HAVE_VASPRINTF)

include(CheckIncludeFile)
check_include_file(sys/types.h WENDY_HAVE_SYS_TYPES_H)
check_include_file(sys/time.h WENDY_HAVE_SYS_TIME_H)
check_include_file(sys/stat.h WENDY_HAVE_SYS_STAT_H)
check_include_file(fcntl.h WENDY_HAVE_FCNTL_H)
check_include_file(string.h WENDY_HAVE_STRING_H)
check_include_file(malloc.h WENDY_HAVE_MALLOC_H)
check_include_file(errno.h WENDY_HAVE_ERRNO_H)
check_include_file(ctype.h WENDY_HAVE_CTYPE_H)
check_include_file(memory.h WENDY_HAVE_MEMORY_H)

if(WIN32)
  check_include_file(io.h WENDY_HAVE_IO_H)
  check_include_file(direct.h WENDY_HAVE_DIRECT_H)
  check_include_file(windows.h WENDY_HAVE_WINDOWS_H)
else()
  check_include_file(dirent.h WENDY_HAVE_DIRENT_H)
  check_include_file(unistd.h WENDY_HAVE_UNISTD_H)
endif(WIN32)

if(UNIX)
  find_package(Ruby REQUIRED)

  set(SIGNAL_INPUT ${wendy_SOURCE_DIR}/build/makesignal.rb)
  set(SIGNAL_OUTPUT ${wendy_BINARY_DIR}/include/wendy/Signal.h)

  add_custom_command(OUTPUT ${SIGNAL_OUTPUT}
                     COMMAND ${RUBY_EXECUTABLE} ${SIGNAL_INPUT} ${SIGNAL_OUTPUT} 4
                     MAIN_DEPENDENCY ${SIGNAL_INPUT}
                     COMMENT "Creating ${SIGNAL_OUTPUT}"
                     VERBATIM)
   
  add_custom_target(signal ALL DEPENDS ${SIGNAL_OUTPUT})
endif(UNIX)

if(WENDY_BUILD_DOCUMENTATION)
  find_package(Doxygen)
endif(WENDY_BUILD_DOCUMENTATION)

find_package(OpenGL REQUIRED)

add_subdirectory(libs)
list(APPEND wendy_LIBRARIES expat png z pcre GLEW glfw ${GLFW_LIBRARIES})

find_package(GLU REQUIRED)
list(APPEND wendy_LIBRARIES ${OPENGL_glu_LIBRARY})

if(APPLE)

  find_library(CG_LIBRARY Cg)
  if(NOT CG_LIBRARY)
    message(FATAL_ERROR "Cg framework not found")
  endif(NOT CG_LIBRARY)
  list(APPEND wendy_LIBRARIES ${CG_LIBRARY})

else()

  find_library(CG_LIBRARY Cg PATHS ENV CG_LIB_PATH)
  find_library(CGGL_LIBRARY CgGL PATHS ENV CG_LIB_PATH)
  if(NOT CG_LIBRARY OR NOT CGGL_LIBRARY)
    message(FATAL_ERROR "Cg libraries not found")
  endif(NOT CG_LIBRARY OR NOT CGGL_LIBRARY)
  list(APPEND wendy_LIBRARIES ${CG_LIBRARY} ${CGGL_LIBRARY})

endif(APPLE)


# Ugly fix for Cg depending on pthreads
if(UNIX AND NOT CYGWIN)
  list(APPEND wendy_LIBRARIES -pthread)
endif(UNIX AND NOT CYGWIN)

if(WIN32)
  include_directories($ENV{CG_INC_PATH})
endif(WIN32)

set(WENDY_INCLUDE_DIRS ${wendy_SOURCE_DIR}/include ${wendy_BINARY_DIR}/include)

if(WIN32)
  list(APPEND WENDY_INCLUDE_DIRS ${wendy_SOURCE_DIR}/build/windows)
endif(WIN32)

if(MSVC)
  list(APPEND WENDY_INCLUDE_DIRS ${wendy_SOURCE_DIR}/build/msvc)
endif(MSVC)

set(WENDY_LIBRARIES ${wendy_LIBRARIES} CACHE STRING "Libraries used by Wendy")
set(WENDY_INCLUDE_DIRS ${WENDY_INCLUDE_DIRS}
    CACHE STRING "Include directories for Wendy")

include_directories(${WENDY_INCLUDE_DIRS})

configure_file(${wendy_SOURCE_DIR}/include/wendy/Config.h.in
               ${wendy_BINARY_DIR}/include/wendy/Config.h @ONLY)

if(WENDY_BUILD_DOCUMENTATION)
  configure_file(${wendy_SOURCE_DIR}/docs/Doxyfile.in
                ${wendy_BINARY_DIR}/docs/Doxyfile @ONLY)
endif(WENDY_BUILD_DOCUMENTATION)

add_subdirectory(src)

if(WENDY_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(WENDY_BUILD_EXAMPLES)

if(WENDY_BUILD_TESTS)
  add_subdirectory(tests)
endif(WENDY_BUILD_TESTS)

install(DIRECTORY ${wendy_BINARY_DIR}/include/wendy
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${wendy_SOURCE_DIR}/include/wendy
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")


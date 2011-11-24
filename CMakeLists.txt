cmake_minimum_required(VERSION 2.6)

project(wendy C CXX)
set(VERSION 0.6.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Avoid source tree pollution
if(wendy_SOURCE_DIR STREQUAL wendy_BINARY_DIR)
  message(WARNING "In-source builds are not recommended. "
                  "Consider building in a separate directory, "
                  "to avoid polluting the source tree.")
endif(wendy_SOURCE_DIR STREQUAL wendy_BINARY_DIR)

# Add a sensible default build type
if(NOT CMAKE_BUILD_TYPE)
  message(WARNING "CMAKE_BUILD_TYPE is not defined; defaulting to Debug.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Build type: None Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(WENDY_INCLUDE_OPENAL "Include the OpenAL wrapper" ON)
option(WENDY_INCLUDE_RENDERER "Include the renderer" ON)
option(WENDY_INCLUDE_SCENE_GRAPH "Include the scene graph" ON)
option(WENDY_INCLUDE_UI_SYSTEM "Include the UI library" ON)
option(WENDY_INCLUDE_DEBUG_UI "Include the debug interface" ON)
option(WENDY_INCLUDE_SQUIRREL "Include the Squirrel bindings" ON)
option(WENDY_INCLUDE_BULLET "Include the Bullet library" ON)
option(WENDY_BUILD_EXAMPLES "Build the bundled example programs" OFF)
option(WENDY_BUILD_TESTS "Build the bundled test programs" OFF)
option(WENDY_BUILD_DOCUMENTATION "Build the Doxygen documentation" OFF)

include(TestBigEndian)
test_big_endian(WENDY_WORDS_BIGENDIAN)

include(CheckFunctionExists)
check_function_exists(vasprintf WENDY_HAVE_VASPRINTF)

include(CheckIncludeFile)
check_include_file(sys/stat.h WENDY_HAVE_SYS_STAT_H)
check_include_file(fcntl.h WENDY_HAVE_FCNTL_H)

if(WIN32)
  check_include_file(io.h WENDY_HAVE_IO_H)
  check_include_file(direct.h WENDY_HAVE_DIRECT_H)
  check_include_file(windows.h WENDY_HAVE_WINDOWS_H)
else()
  check_include_file(dirent.h WENDY_HAVE_DIRENT_H)
  check_include_file(unistd.h WENDY_HAVE_UNISTD_H)
endif(WIN32)

if(WIN32)
  set(WENDY_SYSTEM_WIN32 1)
elseif(APPLE)
  set(WENDY_SYSTEM_MACOSX 1)
else()
  set(WENDY_SYSTEM_LINUX 1)
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

  check_include_file(sys/inotify.h WENDY_HAVE_SYS_INOTIFY_H)
endif(UNIX)

if(WENDY_BUILD_DOCUMENTATION)
  find_package(Doxygen)
endif(WENDY_BUILD_DOCUMENTATION)

if(WENDY_INCLUDE_OPENAL)
  find_package(OpenAL REQUIRED)
endif(WENDY_INCLUDE_OPENAL)

find_package(OpenGL REQUIRED)

add_subdirectory(libs)

list(APPEND wendy_CORE_LIBRARIES pugixml png z pcre)

list(APPEND wendy_LIBRARIES GLEW glfw ${GLFW_LIBRARIES})
if(WENDY_INCLUDE_OPENAL)
  list(APPEND wendy_LIBRARIES vorbis ogg ${OPENAL_LIBRARY})
endif(WENDY_INCLUDE_OPENAL)

list(APPEND wendy_INCLUDE_DIRS ${wendy_SOURCE_DIR}/include
                               ${wendy_BINARY_DIR}/include
                               ${glm_SOURCE_DIR}
                               ${pugixml_SOURCE_DIR})

if(WENDY_INCLUDE_SQUIRREL)
  list(APPEND wendy_CORE_LIBRARIES squirrel)
  list(APPEND wendy_INCLUDE_DIRS ${squirrel_SOURCE_DIR})
endif(WENDY_INCLUDE_SQUIRREL)

if(WENDY_INCLUDE_BULLET)
  list(APPEND wendy_CORE_LIBRARIES bullet)
  list(APPEND wendy_INCLUDE_DIRS ${bullet_SOURCE_DIR})
endif(WENDY_INCLUDE_BULLET)

if(WIN32)
  list(APPEND wendy_INCLUDE_DIRS ${wendy_SOURCE_DIR}/build/windows)
endif(WIN32)

set(WENDY_CORE_LIBRARIES ${wendy_CORE_LIBRARIES} CACHE STRING
    "Libraries used by Wendy core" FORCE)
set(WENDY_LIBRARIES ${wendy_LIBRARIES} ${wendy_CORE_LIBRARIES} CACHE STRING
    "Libraries used by Wendy" FORCE)
set(WENDY_INCLUDE_DIRS ${wendy_INCLUDE_DIRS} CACHE STRING
    "Include directories for Wendy" FORCE)

include_directories(${wendy_INCLUDE_DIRS})

if(WENDY_BUILD_EXAMPLES OR WENDY_BUILD_TESTS)
  set(WENDY_MEDIA_DIR ${wendy_SOURCE_DIR}/media CACHE STRING
      "Media path used by Wendy tests and examples")
  set(EXECUTABLE_OUTPUT_PATH ${wendy_BINARY_DIR})
endif(WENDY_BUILD_EXAMPLES OR WENDY_BUILD_TESTS)

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


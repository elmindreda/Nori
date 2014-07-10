cmake_minimum_required(VERSION 2.8)

project(wendy C CXX)
set(VERSION 0.6.0)

if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()

# Avoid source tree pollution
if (wendy_SOURCE_DIR STREQUAL wendy_BINARY_DIR)
  message(WARNING "In-source builds are not recommended. "
                  "Consider building in a separate directory, "
                  "to avoid polluting the source tree.")
endif()

# Add a sensible default build type
if (NOT CMAKE_BUILD_TYPE)
  message(WARNING "CMAKE_BUILD_TYPE is not defined; defaulting to Debug.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Build type: None Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

option(WENDY_INCLUDE_NETWORK "Include the networking API" ON)
option(WENDY_INCLUDE_AUDIO "Include the audio library" ON)
option(WENDY_INCLUDE_RENDERER "Include the renderer" ON)
option(WENDY_INCLUDE_UI_SYSTEM "Include the UI library" ON)
option(WENDY_INCLUDE_DEBUG_UI "Include the debug interface" ON)
option(WENDY_INCLUDE_SQUIRREL "Include the Squirrel bindings" ON)
option(WENDY_INCLUDE_BULLET "Include the Bullet library" ON)
option(WENDY_BUILD_DOCUMENTATION "Build the Doxygen documentation" OFF)

include(TestBigEndian)
test_big_endian(WENDY_WORDS_BIGENDIAN)

include(CheckIncludeFile)
check_include_file(sys/stat.h WENDY_HAVE_SYS_STAT_H)
check_include_file(fcntl.h WENDY_HAVE_FCNTL_H)

if (WIN32)
  check_include_file(io.h WENDY_HAVE_IO_H)
  check_include_file(direct.h WENDY_HAVE_DIRECT_H)
  check_include_file(windows.h WENDY_HAVE_WINDOWS_H)
else()
  check_include_file(dirent.h WENDY_HAVE_DIRENT_H)
  check_include_file(unistd.h WENDY_HAVE_UNISTD_H)
  check_include_file(glob.h WENDY_HAVE_GLOB_H)
endif()

if (WIN32)
  set(WENDY_SYSTEM_WIN32 1)
elseif (APPLE)
  set(WENDY_SYSTEM_MACOSX 1)
else()
  set(WENDY_SYSTEM_LINUX 1)
endif()

if (WENDY_BUILD_DOCUMENTATION)
  find_package(Doxygen)
endif()

if (WENDY_INCLUDE_AUDIO)
  set(CMAKE_INCLUDE_PATH "C:\\Program\ Files\\OpenAL\ 1.1\ SDK\\include;C:\\Program\ Files\ (x86)\\OpenAL\ 1.1\ SDK\\include")
  set(CMAKE_LIBRARY_PATH "C:\\Program\ Files\\OpenAL\ 1.1\ SDK\\libs\\Win32;C:\\Program\ Files\ (x86)\\OpenAL\ 1.1\ SDK\\libs\\Win32")
  find_package(OpenAL REQUIRED)
endif()

find_package(OpenGL REQUIRED)

add_subdirectory(deps)

list(APPEND wendy_CORE_LIBRARIES pugixml png z pcre vorbis ogg)

list(APPEND wendy_LIBRARIES greg glfw ${GLFW_LIBRARIES})
if (WENDY_INCLUDE_AUDIO)
  list(APPEND wendy_LIBRARIES ${OPENAL_LIBRARY})
endif()

list(APPEND wendy_INCLUDE_DIRS ${wendy_SOURCE_DIR}/include
                               ${wendy_BINARY_DIR}/include
                               ${glm_SOURCE_DIR}
                               ${pugixml_SOURCE_DIR})

if (WENDY_INCLUDE_NETWORK)
  list(APPEND wendy_CORE_LIBRARIES enet)
endif()

if (WENDY_INCLUDE_SQUIRREL)
  list(APPEND wendy_CORE_LIBRARIES squirrel)
  list(APPEND wendy_INCLUDE_DIRS ${squirrel_SOURCE_DIR})
endif()

if (WENDY_INCLUDE_BULLET)
  list(APPEND wendy_CORE_LIBRARIES bullet)
  list(APPEND wendy_INCLUDE_DIRS ${bullet_SOURCE_DIR})
endif()

set(WENDY_CORE_LIBRARIES ${wendy_CORE_LIBRARIES} CACHE STRING
    "Libraries used by Wendy core" FORCE)
set(WENDY_LIBRARIES ${wendy_LIBRARIES} ${wendy_CORE_LIBRARIES} CACHE STRING
    "Libraries used by Wendy" FORCE)
set(WENDY_INCLUDE_DIRS ${wendy_INCLUDE_DIRS} CACHE STRING
    "Include directories for Wendy" FORCE)

include_directories(${wendy_INCLUDE_DIRS})

configure_file(${wendy_SOURCE_DIR}/include/wendy/Config.hpp.in
               ${wendy_BINARY_DIR}/include/wendy/Config.hpp @ONLY)

if (WENDY_BUILD_DOCUMENTATION)
  configure_file(${wendy_SOURCE_DIR}/docs/Doxyfile.in
                 ${wendy_BINARY_DIR}/docs/Doxyfile @ONLY)
endif()

add_subdirectory(src)


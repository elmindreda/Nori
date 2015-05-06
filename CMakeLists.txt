cmake_minimum_required(VERSION 2.8)

project(nori C CXX)
set(VERSION 0.6.0)

if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()

# Avoid source tree pollution
if (nori_SOURCE_DIR STREQUAL nori_BINARY_DIR)
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

option(NORI_INCLUDE_NETWORK "Include the networking API" ON)
option(NORI_INCLUDE_AUDIO "Include the audio library" ON)
option(NORI_INCLUDE_RENDERER "Include the renderer" ON)
option(NORI_INCLUDE_UI_SYSTEM "Include the UI library" ON)
option(NORI_INCLUDE_DEBUG_UI "Include the debug interface" ON)
option(NORI_INCLUDE_SQUIRREL "Include the Squirrel bindings" ON)
option(NORI_INCLUDE_BULLET "Include the Bullet library" ON)
option(NORI_BUILD_DOCUMENTATION "Build the Doxygen documentation" OFF)

include(TestBigEndian)
test_big_endian(NORI_WORDS_BIGENDIAN)

include(CheckIncludeFile)
check_include_file(sys/stat.h NORI_HAVE_SYS_STAT_H)
check_include_file(fcntl.h NORI_HAVE_FCNTL_H)

if (WIN32)
  check_include_file(io.h NORI_HAVE_IO_H)
  check_include_file(direct.h NORI_HAVE_DIRECT_H)
  check_include_file(windows.h NORI_HAVE_WINDOWS_H)
else()
  check_include_file(dirent.h NORI_HAVE_DIRENT_H)
  check_include_file(unistd.h NORI_HAVE_UNISTD_H)
endif()

if (WIN32)
  set(NORI_SYSTEM_WIN32 1)
elseif (APPLE)
  set(NORI_SYSTEM_MACOSX 1)
else()
  set(NORI_SYSTEM_LINUX 1)
endif()

if (NORI_BUILD_DOCUMENTATION)
  find_package(Doxygen)
endif()

if (NORI_INCLUDE_AUDIO)
  set(CMAKE_INCLUDE_PATH "C:\\Program\ Files\\OpenAL\ 1.1\ SDK\\include;C:\\Program\ Files\ (x86)\\OpenAL\ 1.1\ SDK\\include")
  set(CMAKE_LIBRARY_PATH "C:\\Program\ Files\\OpenAL\ 1.1\ SDK\\libs\\Win32;C:\\Program\ Files\ (x86)\\OpenAL\ 1.1\ SDK\\libs\\Win32")
  find_package(OpenAL REQUIRED)
endif()

find_package(OpenGL REQUIRED)

add_subdirectory(deps)

list(APPEND nori_CORE_LIBRARIES pugixml)

list(APPEND nori_LIBRARIES glfw ${GLFW_LIBRARIES})
if (NORI_INCLUDE_AUDIO)
  list(APPEND nori_LIBRARIES ${OPENAL_LIBRARY})
endif()

list(APPEND nori_INCLUDE_DIRS ${nori_SOURCE_DIR}/include
                              ${nori_BINARY_DIR}/include
                              ${glm_SOURCE_DIR}
                              ${pugixml_SOURCE_DIR})

if (NORI_INCLUDE_NETWORK)
  list(APPEND nori_CORE_LIBRARIES enet)
endif()

if (NORI_INCLUDE_SQUIRREL)
  list(APPEND nori_CORE_LIBRARIES squirrel)
  list(APPEND nori_INCLUDE_DIRS ${squirrel_SOURCE_DIR})
endif()

if (NORI_INCLUDE_BULLET)
  list(APPEND nori_CORE_LIBRARIES bullet)
  list(APPEND nori_INCLUDE_DIRS ${bullet_SOURCE_DIR})
endif()

set(NORI_CORE_LIBRARIES ${nori_CORE_LIBRARIES} CACHE STRING
    "Libraries used by Nori core" FORCE)
set(NORI_LIBRARIES ${nori_LIBRARIES} ${nori_CORE_LIBRARIES} CACHE STRING
    "Libraries used by Nori" FORCE)
set(NORI_INCLUDE_DIRS ${nori_INCLUDE_DIRS} CACHE STRING
    "Include directories for Nori" FORCE)

include_directories(${nori_INCLUDE_DIRS})

configure_file(${nori_SOURCE_DIR}/include/nori/Config.hpp.in
               ${nori_BINARY_DIR}/include/nori/Config.hpp @ONLY)

if (NORI_BUILD_DOCUMENTATION)
  configure_file(${nori_SOURCE_DIR}/docs/Doxyfile.in
                 ${nori_BINARY_DIR}/docs/Doxyfile @ONLY)
endif()

add_subdirectory(src)


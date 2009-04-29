cmake_minimum_required(VERSION 2.6)

project(wendy C CXX)
set(VERSION 0.6.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

option(WENDY_USE_INTERNAL "Use internal versions of library dependencies" ON)
option(WENDY_INCLUDE_UI_SYSTEM "Include the UI library" ON)
option(WENDY_INCLUDE_DEMO_SYSTEM "Include the demo system" ON)
option(WENDY_INCLUDE_OPENAL_WRAPPER "Include the OpenAL wrapper" OFF)
option(WENDY_BUILD_EXAMPLES "Build the bundled example programs" OFF)

if(WENDY_USE_INTERNAL)

  add_subdirectory(libs)
  list(APPEND wendy_LIBRARIES GLEW)

else()

  find_library(GLEW_LIBRARY GLEW)
  if(NOT GLEW_LIBRARY)
    message(FATAL_ERROR "GLEW not found")
  endif(NOT GLEW_LIBRARY)
  list(APPEND wendy_LIBRARIES ${GLEW_LIBRARY})

endif(WENDY_USE_INTERNAL)

if(WENDY_INCLUDE_OPENAL_WRAPPER)
  find_package(OpenAL REQUIRED)
  list(APPEND wendy_LIBRARIES ${OPENAL_LIBRARY})
endif(WENDY_INCLUDE_OPENAL_WRAPPER)

find_package(GLU REQUIRED)
list(APPEND wendy_LIBRARIES ${OPENGL_glu_LIBRARY})

find_library(CG_LIBRARY Cg PATHS ENV CG_LIB_PATH)
find_library(CGGL_LIBRARY CgGL PATHS ENV CG_LIB_PATH)
if(NOT CG_LIBRARY OR NOT CGGL_LIBRARY)
  message(FATAL_ERROR "Cg libraries not found")
endif(NOT CG_LIBRARY OR NOT CGGL_LIBRARY)
list(APPEND wendy_LIBRARIES ${CG_LIBRARY} ${CGGL_LIBRARY})

if(NOT MSVC)
  include(FindPkgConfig)
  pkg_check_modules(GLFW REQUIRED libglfw>=2.6.0)
  list(APPEND wendy_LIBRARIES ${GLFW_LIBRARIES})
endif(NOT MSVC)

if(WIN32)
  include_directories($ENV{CG_INC_PATH})
endif(WIN32)

find_package(Doxygen)

# Import Moira (assume parallell directories)
add_subdirectory(${wendy_SOURCE_DIR}/../moira ${wendy_BINARY_DIR}/moira)

set(WENDY_LIBRARIES ${wendy_LIBRARIES} ${MOIRA_LIBRARIES} CACHE STRING "Libraries used by Wendy")
set(WENDY_INCLUDE_DIRS ${MOIRA_INCLUDE_DIRS}
		       ${GLEW_SOURCE_DIR}
                       ${wendy_SOURCE_DIR}/include
		       ${wendy_BINARY_DIR}/include
    CACHE STRING "Include directories for Wendy")

include_directories(${WENDY_INCLUDE_DIRS})

configure_file(${wendy_SOURCE_DIR}/docs/Doxyfile.in
               ${wendy_BINARY_DIR}/docs/Doxyfile @ONLY)
configure_file(${wendy_SOURCE_DIR}/include/wendy/Config.h.in
               ${wendy_BINARY_DIR}/include/wendy/Config.h @ONLY)
configure_file(${wendy_SOURCE_DIR}/build/libwendy.pc.in
               ${wendy_BINARY_DIR}/build/libwendy.pc @ONLY)

add_subdirectory(src)

if(WENDY_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(WENDY_BUILD_EXAMPLES)

install(DIRECTORY include/wendy
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")
install(FILES ${wendy_SOURCE_DIR}/build/libwendy.pc
        DESTINATION lib/pkgconfig)


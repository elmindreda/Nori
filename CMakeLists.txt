cmake_minimum_required(VERSION 2.6)

project(wendy C CXX)
set(VERSION 0.6.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include(FindPkgConfig)

find_library(GLEW_LIBRARY GLEW)
if(NOT GLEW_LIBRARY)
  message(FATAL_ERROR "GLEW not found")
endif(NOT GLEW_LIBRARY)
list(APPEND WENDY_LIBRARIES ${GLEW_LIBRARY})

pkg_check_modules(GLFW REQUIRED libglfw>=2.6.0)
list(APPEND WENDY_LIBRARIES ${GLFW_LIBRARIES})

find_package(OpenAL REQUIRED)
link_directories(${OPENAL_LIBRARY_DIRS})
list(APPEND WENDY_LIBRARIES ${OPENAL_LIBRARY})

find_library(CG_LIBRARY Cg)
find_library(CGGL_LIBRARY CgGL)
if(NOT CG_LIBRARY OR NOT CGGL_LIBRARY)
  message(FATAL_ERROR "Cg not found")
endif(NOT CG_LIBRARY OR NOT CGGL_LIBRARY)
list(APPEND WENDY_LIBRARIES ${CG_LIBRARY} ${CGGL_LIBRARY})

find_package(Doxygen)

add_subdirectory(${wendy_SOURCE_DIR}/../moira ${wendy_BINARY_DIR}/moira)

set(WENDY_LIBRARIES ${WENDY_LIBRARIES} ${MOIRA_LIBRARIES} CACHE STRING "Libraries used by Wendy")
set(WENDY_INCLUDE_DIRS ${MOIRA_INCLUDE_DIRS} ${wendy_SOURCE_DIR}/include ${wendy_BINARY_DIR}/include CACHE STRING "Include directories for Wendy")

include_directories(${WENDY_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(examples/simple)
add_subdirectory(examples/fonts)
add_subdirectory(examples/program)
add_subdirectory(examples/editor)
#add_subdirectory(examples/widgets)

configure_file(${wendy_SOURCE_DIR}/docs/Doxyfile.in
               ${wendy_BINARY_DIR}/docs/Doxyfile @ONLY)
configure_file(${wendy_SOURCE_DIR}/include/wendy/Config.h.in
               ${wendy_BINARY_DIR}/include/wendy/Config.h @ONLY)
configure_file(${wendy_SOURCE_DIR}/build/libwendy.pc.in
               ${wendy_BINARY_DIR}/build/libwendy.pc @ONLY)

install(DIRECTORY include/wendy
        DESTINATION include
	FILES_MATCHING PATTERN "*.h")
install(FILES ${wendy_SOURCE_DIR}/build/libwendy.pc
        DESTINATION lib/pkgconfig)


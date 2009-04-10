cmake_minimum_required(VERSION 2.6)

project(wendy C CXX)
set(VERSION 0.6.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(WENDY_USE_CG 1 CACHE BOOL "Compile with Cg support")

include(FindPkgConfig)

pkg_check_modules(moira REQUIRED libmoira>=0.6.0)
list(APPEND WENDY_LIBRARIES ${moira_LIBRARIES})

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

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src)
#add_subdirectory(examples/editor)
#add_subdirectory(examples/fonts)
add_subdirectory(examples/program)
add_subdirectory(examples/simple)
#add_subdirectory(examples/widgets)

configure_file(docs/Doxyfile.in
               docs/Doxyfile @ONLY)
configure_file(include/wendy/Config.h.in
               include/wendy/Config.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/build/libwendy.pc.in
               ${CMAKE_BINARY_DIR}/build/libwendy.pc @ONLY)

install(DIRECTORY include/wendy
        DESTINATION include
	FILES_MATCHING PATTERN "*.h")
install(FILES ${CMAKE_SOURCE_DIR}/build/libwendy.pc
        DESTINATION lib/pkgconfig)


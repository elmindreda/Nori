project(wendy CXX)

set(VERSION 0.6.0)

include(UsePkgConfig)

find_package(OpenAL REQUIRED)
link_directories(${OPENAL_LIBRARY_DIRS})
link_libraries(${OPENAL_LIBRARIES})

find_package(OpenGL REQUIRED)
link_directories(${OPENGL_LIBRARY_DIRS})
link_libraries(${OPENGL_LIBRARIES})

find_library(GLEW_LIBRARY GLEW)

pkgconfig(libmoira MOIRA_INCLUDE_DIR MOIRA_LIBRARY_DIR MOIRA_LDFLAGS MOIRA_CFLAGS)
pkgconfig(libglfw GLFW_INCLUDE_DIR GLFW_LIBRARY_DIR GLFW_LDFLAGS GLFW_CFLAGS)

include_directories(include)
link_directories(src)

install(DIRECTORY include/wendy DESTINATION include)

add_subdirectory(src)
add_subdirectory(examples/editor)
add_subdirectory(examples/fonts)
add_subdirectory(examples/simple)
add_subdirectory(examples/widgets)

configure_file(docs/Doxyfile.in docs/Doxyfile @ONLY)


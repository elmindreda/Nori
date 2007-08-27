project(wendy)

include(UsePkgConfig)

find_package(OpenAL REQUIRED)
find_package(OpenGL REQUIRED)

find_library(GLEW_LIBRARY GLEW)

pkgconfig(libmoira MOIRA_INCLUDE_DIR MOIRA_LIBRARY_DIR MOIRA_LDFLAGS MOIRA_CFLAGS)
pkgconfig(libglfw GLFW_INCLUDE_DIR GLFW_LIBRARY_DIR GLFW_LDFLAGS GLFW_CFLAGS)

include_directories(include)

add_subdirectory(src)
add_subdirectory(examples/editor)
add_subdirectory(examples/fonts)
add_subdirectory(examples/simple)
add_subdirectory(examples/widgets)


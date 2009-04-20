
project(GLEW C)

include_directories(${GLEW_SOURCE_DIR})

add_definitions(-DGLEW_STATIC)

add_library(GLEW STATIC glew.c)


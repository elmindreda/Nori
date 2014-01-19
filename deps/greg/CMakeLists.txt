
project(greg C)

include_directories(${GLFW_SOURCE_DIR} ${greg_SOURCE_DIR})

add_definitions(-D_GREG_USE_GLFW3)

add_library(greg STATIC greg.c GREG/greg.h)


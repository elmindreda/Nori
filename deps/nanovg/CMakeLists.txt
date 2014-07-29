
project(nanovg C)

set(nanovg_HEADERS fontstash.h nanovg.h nanovg_gl.h nanovg_gl_utils.h)
set(nanovg_SOURCES nanovg.c)

include_directories(${stb_SOURCE_DIR} ${nanovg_SOURCE_DIR})

add_library(nanovg STATIC ${nanovg_SOURCES})


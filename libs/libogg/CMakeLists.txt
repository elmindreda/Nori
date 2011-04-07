
project(libogg C)

include(CheckIncludeFile)

if(NOT WIN32)
  check_include_file(stdint.h HAVE_STDINT_H)
  if(NOT HAVE_STDINT_H)
    message(FATAL_ERROR "Could not find stdint.h")
  endif(NOT HAVE_STDINT_H)
endif(NOT WIN32)

include_directories(${libogg_SOURCE_DIR})

add_library(ogg STATIC bitwise.c framing.c)


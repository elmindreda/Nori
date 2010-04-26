
project(expat C)

include(CheckIncludeFile)
include(CheckFunctionExists)
include(TestBigEndian)

check_include_file(unistd.h HAVE_UNISTD_H)

check_function_exists(bcopy HAVE_BCOPY)
check_function_exists(memmove HAVE_MEMMOVE)
check_function_exists(mmap HAVE_MMAP)

test_big_endian(WORDS_BIGENDIAN)

set(XML_CONTEXT_BYTES "1024" CACHE STRING
    "Define to specify how much context to retain around the current parse point.")

if(WORDS_BIGENDIAN)
  set(BYTEORDER 4321)
else()
  set(BYTEORDER 1234)
endif(WORDS_BIGENDIAN)

configure_file(${expat_SOURCE_DIR}/expat_config.h.cmake
               ${expat_BINARY_DIR}/expat_config.h)

include_directories(${expat_SOURCE_DIR} ${expat_BINARY_DIR})

add_definitions(-DHAVE_EXPAT_CONFIG_H)

add_library(expat STATIC xmlparse.c xmlrole.c xmltok.c xmltok_impl.c xmltok_ns.c)


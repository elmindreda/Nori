
project(zlib C)

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif()

include_directories(${zlib_SOURCE_DIR})

add_library(z STATIC adler32.c compress.c crc32.c deflate.c gzio.c infback.c
                     inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c)



project(libpng C)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(MSVC)

include_directories(${zlib_SOURCE_DIR} ${libpng_SOURCE_DIR})

add_library(png STATIC png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c
                       pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c
                       pngwio.c pngwrite.c pngwtran.c pngwutil.c)

add_dependencies(png z)


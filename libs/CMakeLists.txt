
add_subdirectory(zlib)
add_subdirectory(libpng)
add_subdirectory(pcre)
add_subdirectory(pugixml)
add_subdirectory(glew)
add_subdirectory(glfw)
add_subdirectory(glm)

if(WENDY_INCLUDE_OPENAL)
  add_subdirectory(libogg)
  add_subdirectory(libvorbis)
endif(WENDY_INCLUDE_OPENAL)

if(WENDY_INCLUDE_SQUIRREL)
  add_subdirectory(squirrel)
endif(WENDY_INCLUDE_SQUIRREL)

if(WENDY_INCLUDE_BULLET)
  add_subdirectory(bullet)
endif(WENDY_INCLUDE_BULLET)


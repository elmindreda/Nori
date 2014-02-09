
add_subdirectory(pcre)
add_subdirectory(pugixml)
add_subdirectory(glfw)
add_subdirectory(greg)
add_subdirectory(glm)
add_subdirectory(libogg)
add_subdirectory(libvorbis)
add_subdirectory(stb)
add_subdirectory(utf8cpp)

if (WENDY_INCLUDE_NETWORK)
  add_subdirectory(enet)
endif()

if (WENDY_INCLUDE_SQUIRREL)
  add_subdirectory(squirrel)
endif()

if (WENDY_INCLUDE_BULLET)
  add_subdirectory(bullet)
endif()



add_subdirectory(pugixml)
add_subdirectory(glfw)
add_subdirectory(greg)
add_subdirectory(glm)
add_subdirectory(stb)
add_subdirectory(utf8cpp)

if (NORI_INCLUDE_NETWORK)
  add_subdirectory(enet)
endif()

if (NORI_INCLUDE_SQUIRREL)
  add_subdirectory(squirrel)
endif()

if (NORI_INCLUDE_BULLET)
  add_subdirectory(bullet)
endif()


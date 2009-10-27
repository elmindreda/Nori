
add_executable(sprite WIN32 MACOSX_BUNDLE sprite.cpp)
target_link_libraries(sprite wendy moira ${WENDY_LIBRARIES})

set_target_properties(sprite PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Sprite
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sprite
  DEBUG_POSTFIX "_debug")


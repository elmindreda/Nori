
add_executable(sprite2 WIN32 MACOSX_BUNDLE sprite2.cpp)
add_executable(clear WIN32 MACOSX_BUNDLE clear.cpp)

target_link_libraries(sprite2 wendy moira ${WENDY_LIBRARIES})
target_link_libraries(clear wendy moira ${WENDY_LIBRARIES})

set_target_properties(sprite2 PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Sprite2
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sprite2
  DEBUG_POSTFIX "_debug")

set_target_properties(clear PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Clear
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.clear
  DEBUG_POSTFIX "_debug")


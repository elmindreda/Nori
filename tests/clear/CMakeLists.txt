
add_executable(clear WIN32 MACOSX_BUNDLE clear.cpp)
target_link_libraries(clear wendy moira ${WENDY_LIBRARIES})

set_target_properties(clear PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Clear
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.clear
  DEBUG_POSTFIX "_debug")


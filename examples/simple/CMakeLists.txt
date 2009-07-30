add_executable(simple WIN32 MACOSX_BUNDLE simple.cpp)
target_link_libraries(simple wendy moira ${WENDY_LIBRARIES})

set_target_properties(simple PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Simple
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.simple
  DEBUG_POSTFIX "_debug")


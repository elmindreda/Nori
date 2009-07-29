add_executable(bloom WIN32 MACOSX_BUNDLE bloom.cpp)
target_link_libraries(bloom wendy moira ${WENDY_LIBRARIES})

set_target_properties(bloom PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Bloom
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.bloom
  DEBUG_POSTFIX "_debug")


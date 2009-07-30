add_executable(program WIN32 MACOSX_BUNDLE program.cpp)
target_link_libraries(program wendy moira ${WENDY_LIBRARIES})

set_target_properties(program PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Program
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.program
  DEBUG_POSTFIX "_debug")


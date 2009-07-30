add_executable(fonts WIN32 MACOSX_BUNDLE fonts.cpp)
target_link_libraries(fonts wendy moira ${WENDY_LIBRARIES})

#set(data default.font default.png)
#set_target_properties(fonts PROPERTIES RESOURCE ${data})
set_target_properties(fonts PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Fonts
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.fonts
  DEBUG_POSTFIX "_debug")


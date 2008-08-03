add_executable(widgets WIN32 MACOSX_BUNDLE widgets.cpp)
target_link_libraries(widgets wendy ${WENDY_LIBRARIES})

#set(data default.font default.png)
#set_target_properties(fonts PROPERTIES RESOURCE ${data})
set_target_properties(widgets PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME Widgets)
set_target_properties(widgets PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.widgets)


add_executable(water WIN32 MACOSX_BUNDLE water.cpp)
target_link_libraries(water wendy moira ${WENDY_LIBRARIES})

set_target_properties(water PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME Water)
set_target_properties(water PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.water)


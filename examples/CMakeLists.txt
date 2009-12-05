
add_executable(bloom WIN32 MACOSX_BUNDLE bloom.cpp)
add_executable(editor WIN32 MACOSX_BUNDLE editor.cpp)
add_executable(font WIN32 MACOSX_BUNDLE font.cpp)
add_executable(graph WIN32 MACOSX_BUNDLE graph.cpp)
add_executable(water WIN32 MACOSX_BUNDLE water.cpp)
add_executable(widgets WIN32 MACOSX_BUNDLE widgets.cpp)

target_link_libraries(bloom wendy moira ${WENDY_LIBRARIES})
target_link_libraries(editor wendy moira ${WENDY_LIBRARIES})
target_link_libraries(font wendy moira ${WENDY_LIBRARIES})
target_link_libraries(graph wendy moira ${WENDY_LIBRARIES})
target_link_libraries(water wendy moira ${WENDY_LIBRARIES})
target_link_libraries(widgets wendy moira ${WENDY_LIBRARIES})

set_target_properties(bloom PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Bloom
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.bloom)

set_target_properties(editor PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Editor
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.editor)

set_target_properties(font PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Font
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.font)

set_target_properties(graph PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Graph
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.graph)

set_target_properties(water PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Water
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.water)

set_target_properties(widgets PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Widgets
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.widgets)

set_target_properties(bloom editor font graph water widgets PROPERTIES
  DEBUG_POSTFIX "_debug")

if(MSVC)
  set_target_properties(bloom editor font graph water widgets PROPERTIES
    LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  set_target_properties(bloom editor font graph water widgets PROPERTIES
    COMPILE_FLAGS "-mno-cygwin"
    LINK_FLAGS "-mno-cygwin -mwindows")
endif(CYGWIN)


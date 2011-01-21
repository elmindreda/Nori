
add_executable(clear WIN32 MACOSX_BUNDLE clear.cpp)
add_executable(cubemap WIN32 MACOSX_BUNDLE cubemap.cpp)
add_executable(deferred WIN32 MACOSX_BUNDLE deferred.cpp)
add_executable(font WIN32 MACOSX_BUNDLE font.cpp)
add_executable(graph WIN32 MACOSX_BUNDLE graph.cpp)
add_executable(sprite2 WIN32 MACOSX_BUNDLE sprite2.cpp)
add_executable(sprite3 WIN32 MACOSX_BUNDLE sprite3.cpp)

target_link_libraries(clear wendy ${WENDY_LIBRARIES})
target_link_libraries(cubemap wendy ${WENDY_LIBRARIES})
target_link_libraries(deferred wendy ${WENDY_LIBRARIES})
target_link_libraries(font wendy ${WENDY_LIBRARIES})
target_link_libraries(graph wendy ${WENDY_LIBRARIES})
target_link_libraries(sprite2 wendy ${WENDY_LIBRARIES})
target_link_libraries(sprite3 wendy ${WENDY_LIBRARIES})

set_target_properties(clear PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Clear
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.clear)

set_target_properties(cubemap PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "Cube Map"
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.cubemap)

set_target_properties(deferred PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Deferred
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.deferred)

set_target_properties(font PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Font
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.font)

set_target_properties(graph PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Graph
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.graph)

set_target_properties(sprite2 PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Sprite2
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sprite2)

set_target_properties(sprite3 PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Sprite3
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sprite3)

set_target_properties(clear cubemap deferred font graph sprite2 sprite3 PROPERTIES
  DEBUG_POSTFIX "_debug")

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables

  set_target_properties(clear cubemap deferred font graph sprite2 sprite3 PROPERTIES
    LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  # Set cross-compile and subsystem compile and link flags

  set_target_properties(clear cubemap deferred font graph sprite2 sprite3 PROPERTIES
    COMPILE_FLAGS "-mno-cygwin"
    LINK_FLAGS "-mno-cygwin -mwindows")
endif(CYGWIN)


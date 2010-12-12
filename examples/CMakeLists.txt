
add_executable(bloom WIN32 MACOSX_BUNDLE bloom.cpp)
add_executable(deferred WIN32 MACOSX_BUNDLE deferred.cpp)
add_executable(font WIN32 MACOSX_BUNDLE font.cpp)
add_executable(graph WIN32 MACOSX_BUNDLE graph.cpp)
add_executable(shadowmap WIN32 MACOSX_BUNDLE shadowmap.cpp)
add_executable(sponza WIN32 MACOSX_BUNDLE sponza.cpp)
add_executable(water WIN32 MACOSX_BUNDLE water.cpp)

target_link_libraries(bloom wendy ${WENDY_LIBRARIES})
target_link_libraries(deferred wendy ${WENDY_LIBRARIES})
target_link_libraries(font wendy ${WENDY_LIBRARIES})
target_link_libraries(graph wendy ${WENDY_LIBRARIES})
target_link_libraries(shadowmap wendy ${WENDY_LIBRARIES})
target_link_libraries(sponza wendy ${WENDY_LIBRARIES})
target_link_libraries(water wendy ${WENDY_LIBRARIES})

set_target_properties(bloom PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Bloom
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.bloom)

set_target_properties(deferred PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Deferred
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.deferred)

set_target_properties(font PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Font
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.font)

set_target_properties(graph PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Graph
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.graph)

set_target_properties(shadowmap PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "Shadow Map"
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.shadowmap)

set_target_properties(sponza PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "Sponza Atrium"
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sponza)

set_target_properties(water PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Water
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.water)

set_target_properties(bloom deferred font graph shadowmap sponza water PROPERTIES
  DEBUG_POSTFIX "_debug")

if(MSVC)
  set_target_properties(bloom deferred font graph shadowmap sponza water PROPERTIES
    LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  set_target_properties(bloom deferred font graph shadowmap sponza water PROPERTIES
    COMPILE_FLAGS "-mno-cygwin"
    LINK_FLAGS "-mno-cygwin -mwindows")
endif(CYGWIN)



add_executable(bloom WIN32 MACOSX_BUNDLE bloom.cpp)
add_executable(shadowmap WIN32 MACOSX_BUNDLE shadowmap.cpp)
add_executable(sponza WIN32 MACOSX_BUNDLE sponza.cpp)

target_link_libraries(bloom wendy ${WENDY_LIBRARIES})
target_link_libraries(shadowmap wendy ${WENDY_LIBRARIES})
target_link_libraries(sponza wendy ${WENDY_LIBRARIES})

set_target_properties(bloom PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Bloom
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.bloom)

set_target_properties(shadowmap PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "Shadow Map"
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.shadowmap)

set_target_properties(sponza PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "Sponza Atrium"
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sponza)

set_target_properties(bloom shadowmap sponza PROPERTIES
  DEBUG_POSTFIX "_debug")

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables

  set_target_properties(bloom shadowmap sponza PROPERTIES
    LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  # Set cross-compile and subsystem compile and link flags

  set_target_properties(bloom shadowmap sponza PROPERTIES
    COMPILE_FLAGS "-mno-cygwin"
    LINK_FLAGS "-mno-cygwin -mwindows")
endif(CYGWIN)


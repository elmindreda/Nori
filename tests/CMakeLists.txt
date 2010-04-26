
add_executable(sprite2 WIN32 MACOSX_BUNDLE sprite2.cpp)
add_executable(clear WIN32 MACOSX_BUNDLE clear.cpp)
add_executable(particles WIN32 MACOSX_BUNDLE particles.cpp)
add_executable(sprite3 WIN32 MACOSX_BUNDLE sprite3.cpp)

target_link_libraries(sprite2 wendy ${WENDY_LIBRARIES})
target_link_libraries(clear wendy ${WENDY_LIBRARIES})
target_link_libraries(particles wendy ${WENDY_LIBRARIES})
target_link_libraries(sprite3 wendy ${WENDY_LIBRARIES})

set_target_properties(sprite2 PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Sprite2
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sprite2
  DEBUG_POSTFIX "_debug")

set_target_properties(clear PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Clear
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.clear
  DEBUG_POSTFIX "_debug")

set_target_properties(particles PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Particles
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.particles
  DEBUG_POSTFIX "_debug")

set_target_properties(sprite3 PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME Sprite3
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sprite3
  DEBUG_POSTFIX "_debug")

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables
  set_target_properties(clear sprite2 sprite3 particles PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  # Set cross-compile and subsystem compile and link flags
  set_target_properties(clear sprite2 sprite3 particles PROPERTIES COMPILE_FLAGS "-mno-cygwin")
  set_target_properties(clear sprite2 sprite3 particles PROPERTIES LINK_FLAGS "-mno-cygwin -mwindows")
endif(CYGWIN)


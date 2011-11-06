
set(TESTS clear cubemap deferred font graph sprite2 sprite3 tessellation)

foreach(test ${TESTS})
  add_executable(${test} WIN32 MACOSX_BUNDLE ${test}.cpp)
  target_link_libraries(${test} wendy ${WENDY_LIBRARIES})

  # Beautify the name for OSX bundle
  string(SUBSTRING ${test} 0 1 first)
  string(SUBSTRING ${test} 1 -1 rest)
  string(TOUPPER ${first} first)
  set_target_properties(${test} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "${first}${rest}"
    MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.${test})
endforeach()

set_target_properties(${TESTS} PROPERTIES DEBUG_POSTFIX "_debug")

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables
  set_target_properties(${TESTS} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  # Set cross-compile and subsystem compile and link flags
  set_target_properties(${TESTS} PROPERTIES COMPILE_FLAGS "-mno-cygwin" LINK_FLAGS "-mno-cygwin -mwindows")
endif(CYGWIN)


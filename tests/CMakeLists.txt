
if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-ansi)
endif(CMAKE_COMPILER_IS_GNUCXX)

add_definitions(-DWENDY_MEDIA_DIR=\"${WENDY_MEDIA_DIR}\")

set(TESTS clear cubemap deferred font geometry tessellation)

foreach(test ${TESTS})
  add_executable(${test} WIN32 MACOSX_BUNDLE ${test}.cpp)
  target_link_libraries(${test} wendy ${WENDY_LIBRARIES})

  # Beautify the name for OSX bundle
  if(APPLE)
    string(SUBSTRING ${test} 0 1 first)
    string(SUBSTRING ${test} 1 -1 rest)
    string(TOUPPER ${first} first)
    set_target_properties(${test} PROPERTIES
                          MACOSX_BUNDLE_BUNDLE_NAME "${first}${rest}"
                          MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.${test})
  endif(APPLE)
endforeach()

set_target_properties(${TESTS} PROPERTIES DEBUG_POSTFIX "_debug")

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables
  set_target_properties(${TESTS} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)



if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-ansi)
endif(CMAKE_COMPILER_IS_GNUCXX)

add_definitions(-DWENDY_MEDIA_DIR=\"${WENDY_MEDIA_DIR}\")

add_executable(sponza WIN32 MACOSX_BUNDLE sponza.cpp)

target_link_libraries(sponza wendy ${WENDY_LIBRARIES})

set_target_properties(sponza PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME "Sponza Atrium"
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.sponza)

set_target_properties(sponza PROPERTIES
  DEBUG_POSTFIX "_debug")

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables

  set_target_properties(sponza PROPERTIES
    LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)


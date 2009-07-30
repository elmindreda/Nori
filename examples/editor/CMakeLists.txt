add_executable(editor WIN32 MACOSX_BUNDLE editor.cpp)
target_link_libraries(editor wendy moira ${WENDY_LIBRARIES})

set(data cube.fs cube.mesh.gz cube.program cube.style cube.vs default.font default.png)

set_target_properties(editor PROPERTIES
  RESOURCE ${data}
  MACOSX_BUNDLE_BUNDLE_NAME Editor
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.wendy.editor
  DEBUG_POSTFIX "_debug")


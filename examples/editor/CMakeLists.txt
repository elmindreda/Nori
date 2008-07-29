add_executable(editor WIN32 MACOSX_BUNDLE editor.cpp)
target_link_libraries(editor wendy ${WENDY_LIBRARIES})

set(data cube.fs cube.mesh.gz cube.program cube.style cube.vs default.font default.png)
set_target_properties(editor PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME Editor)
set_target_properties(editor PROPERTIES RESOURCE ${data})

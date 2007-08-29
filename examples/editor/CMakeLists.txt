add_executable(editor WIN32 MACOSX_BUNDLE editor.cpp)
target_link_libraries(editor wendy ${MOIRA_LDFLAGS} ${GLFW_LDFLAGS} ${GLEW_LIBRARY})

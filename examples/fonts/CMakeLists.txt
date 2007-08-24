add_executable(fonts WIN32 MACOSX_BUNDLE fonts.cpp)
target_link_libraries(fonts wendy ${MOIRA_LDFLAGS} ${GLFW_LDFLAGS} ${GLEW_LIBRARY} ${OPENGL_LIBRARIES})

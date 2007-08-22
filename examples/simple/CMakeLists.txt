set(MACOSX_BUNDLE_BUNDLE_NAME "Simple")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.elmindreda.wendy.simple")

add_executable(simple WIN32 MACOSX_BUNDLE simple.cpp)
target_link_libraries(simple wendy ${MOIRA_LDFLAGS} ${GLFW_LDFLAGS} ${GLEW_LIBRARY} ${OPENGL_LIBRARIES})

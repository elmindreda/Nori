add_executable(widgets WIN32 MACOSX_BUNDLE widgets.cpp)
target_link_libraries(widgets wendy ${MOIRA_LDFLAGS} ${GLFW_LDFLAGS} ${GLEW_LIBRARY})

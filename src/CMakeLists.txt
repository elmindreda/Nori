
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_definitions(-std=c++0x)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_definitions(-std=c++11)
endif()

include_directories(${libpng_SOURCE_DIR}
                    ${zlib_SOURCE_DIR}
                    ${pugixml_SOURCE_DIR}
                    ${PCRE_SOURCE_DIR}
                    ${GLEW_SOURCE_DIR}
                    ${GLFW_SOURCE_DIR}
                    ${libogg_SOURCE_DIR}
                    ${libvorbis_SOURCE_DIR}
                    ${stb_SOURCE_DIR}
                    ${utf8cpp_SOURCE_DIR})

set(wendy_SOURCES
    Wendy.cpp

    AABB.cpp Core.cpp Camera.cpp Face.cpp Frustum.cpp Image.cpp Mesh.cpp OBB.cpp
    Pattern.cpp Path.cpp Pixel.cpp Plane.cpp Profile.cpp Ray.cpp Rect.cpp
    Resource.cpp Sample.cpp Signal.cpp Sphere.cpp Timer.cpp Transform.cpp
    Triangle.cpp Vertex.cpp

    GLBuffer.cpp GLContext.cpp GLHelper.cpp GLParser.cpp GLProgram.cpp
    GLQuery.cpp GLTexture.cpp

    Window.cpp)

if (WENDY_INCLUDE_NETWORK)
  include_directories(${enet_SOURCE_DIR})
  list(APPEND wendy_SOURCES Network.cpp)
endif()

if (WENDY_INCLUDE_OPENAL)
  include_directories(${OPENAL_INCLUDE_DIR})
  list(APPEND wendy_SOURCES ALBuffer.cpp ALContext.cpp ALHelper.cpp ALSource.cpp)
endif()

if (WENDY_INCLUDE_RENDERER)
  list(APPEND wendy_SOURCES
       RenderFont.cpp RenderModel.cpp RenderMaterial.cpp RenderPool.cpp
       RenderScene.cpp RenderSprite.cpp RenderState.cpp RenderSystem.cpp

       Forward.cpp)
endif()

if (WENDY_INCLUDE_SQUIRREL)
  include_directories(${squirrel_SOURCE_DIR})
  list(APPEND wendy_SOURCES Squirrel.cpp)
endif()

if (WENDY_INCLUDE_BULLET)
  include_directories(${bullet_SOURCE_DIR})
  list(APPEND wendy_SOURCES Bullet.cpp)
endif()

if (WENDY_INCLUDE_SCENE_GRAPH)
  list(APPEND wendy_SOURCES SceneGraph.cpp)
endif()

if (WENDY_INCLUDE_UI_SYSTEM)
  list(APPEND wendy_SOURCES
       UIBook.cpp UIButton.cpp UICanvas.cpp UIDrawer.cpp UIEntry.cpp UIItem.cpp
       UILabel.cpp UILayer.cpp UILayout.cpp UIList.cpp UIMenu.cpp UIPopup.cpp
       UIProgress.cpp UIScroller.cpp UISlider.cpp UIWidget.cpp)
endif()

if (WENDY_INCLUDE_DEBUG_UI)
  list(APPEND wendy_SOURCES DebugUI.cpp)
endif()

# We add headers to list of sources so that CMake's generator
# knows to add them to IDE project files for e.g. Qt Creator
file(GLOB_RECURSE wendy_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)

add_library(wendy STATIC ${wendy_SOURCES} ${wendy_HEADERS})
set_target_properties(wendy PROPERTIES COMPILE_DEFINITIONS_DEBUG WENDY_DEBUG)

if (UNIX)
  add_dependencies(wendy signal)
endif()


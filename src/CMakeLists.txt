
if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-ansi)
endif(CMAKE_COMPILER_IS_GNUCXX)

include_directories(${libpng_SOURCE_DIR}
                    ${zlib_SOURCE_DIR}
                    ${pugixml_SOURCE_DIR}
                    ${PCRE_SOURCE_DIR}
                    ${GLEW_SOURCE_DIR}
                    ${glfw_SOURCE_DIR})

set(wendy_SOURCES
    Wendy.cpp

    AABB.cpp Animation.cpp Block.cpp Core.cpp Frustum.cpp Image.cpp Mesh.cpp
    OBB.cpp Pattern.cpp Path.cpp Pixel.cpp Plane.cpp Random.cpp Ray.cpp
    Rectangle.cpp Resource.cpp Signal.cpp Sphere.cpp Timer.cpp Transform.cpp
    Triangle.cpp Vertex.cpp

    OpenGL.cpp GLBuffer.cpp GLContext.cpp GLConvert.cpp GLProgram.cpp
    GLQuery.cpp GLState.cpp GLTexture.cpp

    Input.cpp)

if(WENDY_INCLUDE_OPENAL)
  include_directories(${OPENAL_INCLUDE_DIR}
                      ${libogg_SOURCE_DIR}
                      ${libvorbis_SOURCE_DIR})
  list(APPEND wendy_SOURCES OpenAL.cpp ALBuffer.cpp ALContext.cpp ALSource.cpp)
endif(WENDY_INCLUDE_OPENAL)

if(WENDY_INCLUDE_RENDERER)
  list(APPEND wendy_SOURCES
       RenderCamera.cpp RenderFont.cpp RenderLight.cpp RenderModel.cpp
       RenderMaterial.cpp RenderPool.cpp RenderScene.cpp RenderSprite.cpp
       RenderState.cpp

       Deferred.cpp Forward.cpp)
endif(WENDY_INCLUDE_RENDERER)

if(WENDY_INCLUDE_SQUIRREL)
  include_directories(${squirrel_SOURCE_DIR})
  list(APPEND wendy_SOURCES Squirrel.cpp)
endif(WENDY_INCLUDE_SQUIRREL)

if(WENDY_INCLUDE_BULLET)
  include_directories(${bullet_SOURCE_DIR})
  list(APPEND wendy_SOURCES Bullet.cpp)
endif(WENDY_INCLUDE_BULLET)

if(WENDY_INCLUDE_SCENE_GRAPH)
  list(APPEND wendy_SOURCES SceneGraph.cpp)
endif(WENDY_INCLUDE_SCENE_GRAPH)

if(WENDY_INCLUDE_UI_SYSTEM)
  list(APPEND wendy_SOURCES
       UIBook.cpp UIButton.cpp UICanvas.cpp UIDrawer.cpp UIEntry.cpp UIItem.cpp
       UILabel.cpp UILayer.cpp UILayout.cpp UIList.cpp UIMenu.cpp UIPopup.cpp
       UIProgress.cpp UIScroller.cpp UISlider.cpp UIWidget.cpp)
endif(WENDY_INCLUDE_UI_SYSTEM)

# We add headers to list of sources so that CMake's generator
# knows to add them to IDE project files for e.g. Qt Creator
file(GLOB_RECURSE wendy_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)

add_library(wendy STATIC ${wendy_SOURCES} ${wendy_HEADERS})
set_target_properties(wendy PROPERTIES COMPILE_DEFINITIONS_DEBUG WENDY_DEBUG)

if(UNIX)
  add_dependencies(wendy signal)
endif(UNIX)


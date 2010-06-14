
include_directories(${libpng_SOURCE_DIR}
                    ${zlib_SOURCE_DIR}
		    ${expat_SOURCE_DIR}
		    ${PCRE_SOURCE_DIR}
                    ${GLEW_SOURCE_DIR}
                    ${glfw_SOURCE_DIR})

set(wendy_SOURCES
    Wendy.cpp

    AABB.cpp Animation.cpp AnimationIO.cpp Block.cpp Color.cpp Core.cpp
    Font.cpp FontIO.cpp Frustum.cpp Image.cpp ImageIO.cpp Managed.cpp
    Matrix.cpp Mesh.cpp MeshIO.cpp OBB.cpp Pattern.cpp Path.cpp Pixel.cpp
    Plane.cpp Quaternion.cpp Random.cpp Ray.cpp Rectangle.cpp Signal.cpp
    Sphere.cpp Stream.cpp Timer.cpp Transform.cpp Triangle.cpp Variant.cpp
    Vector.cpp Vertex.cpp XML.cpp

    OpenGL.cpp GLBuffer.cpp GLContext.cpp GLConvert.cpp GLProgram.cpp
    GLProgramIO.cpp GLState.cpp GLTexture.cpp GLTextureIO.cpp GLVertex.cpp

    Input.cpp InputCamera.cpp)

if(WENDY_INCLUDE_RENDERER)
    list(APPEND wendy_SOURCES RenderCamera.cpp RenderFont.cpp RenderLight.cpp RenderMesh.cpp
                              RenderMaterial.cpp RenderMaterialIO.cpp
                              RenderParticle.cpp RenderPool.cpp RenderQueue.cpp
                              RenderSprite.cpp Deferred.cpp)
endif(WENDY_INCLUDE_RENDERER)

if(WENDY_INCLUDE_SCENE_GRAPH)
  list(APPEND wendy_SOURCES SceneGraph.cpp SceneIO.cpp)
endif(WENDY_INCLUDE_SCENE_GRAPH)

if(WENDY_INCLUDE_UI_SYSTEM)
  list(APPEND wendy_SOURCES UIBook.cpp UIButton.cpp UICanvas.cpp UIColor.cpp
                            UIDesktop.cpp UIEntry.cpp UIItem.cpp UILabel.cpp
                            UILayout.cpp UIList.cpp UIMenu.cpp UIPopup.cpp
                            UIProgress.cpp UIRender.cpp UIScroller.cpp
                            UISlider.cpp UIWidget.cpp)
endif(WENDY_INCLUDE_UI_SYSTEM)

if(WENDY_INCLUDE_DEMO_SYSTEM)
  list(APPEND wendy_SOURCES DemoEffect.cpp DemoProperty.cpp DemoShow.cpp
                            DemoShowIO.cpp)
endif(WENDY_INCLUDE_DEMO_SYSTEM)

add_library(wendy STATIC ${wendy_SOURCES})
set_target_properties(wendy PROPERTIES COMPILE_DEFINITIONS_DEBUG WENDY_DEBUG)

if(UNIX)
  add_dependencies(wendy signal)
endif(UNIX)


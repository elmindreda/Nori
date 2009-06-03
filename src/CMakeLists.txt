
set(wendy_SOURCES
    Wendy.cpp
    GLBuffer.cpp GLContext.cpp GLProgram.cpp GLProgramIO.cpp GLRender.cpp GLState.cpp GLTexture.cpp GLTextureIO.cpp GLVertex.cpp
    RenderCamera.cpp RenderFont.cpp RenderLight.cpp RenderMesh.cpp RenderMaterial.cpp RenderMaterialIO.cpp RenderParticle.cpp RenderQueue.cpp RenderSprite.cpp RenderTerrain.cpp RenderTerrainIO.cpp
    SceneGraph.cpp SceneIO.cpp
    Input.cpp)

if(WENDY_INCLUDE_UI_SYSTEM)
  list(APPEND wendy_SOURCES
  UIBook.cpp UIButton.cpp UICanvas.cpp UIColor.cpp UIDesktop.cpp UIEntry.cpp
  UIItem.cpp UILabel.cpp UILayout.cpp UIList.cpp UIMenu.cpp UIPopup.cpp
  UIProgress.cpp UIRender.cpp UIScroller.cpp UISlider.cpp UIView.cpp
  UIWidget.cpp UIWindow.cpp)
endif(WENDY_INCLUDE_UI_SYSTEM)

if(WENDY_INCLUDE_DEMO_SYSTEM)
  list(APPEND wendy_SOURCES 
    DemoConfig.cpp DemoEditor.cpp DemoEffect.cpp DemoPlayer.cpp
    DemoProperty.cpp DemoShow.cpp DemoShowIO.cpp DemoTimeline.cpp)
endif(WENDY_INCLUDE_DEMO_SYSTEM)

add_library(wendy STATIC ${wendy_SOURCES})
add_dependencies(wendy moira)

install(TARGETS wendy ARCHIVE DESTINATION lib)


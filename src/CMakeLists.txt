
set(wendy_SOURCES
    Core.cpp Wendy.cpp
    #ALBuffer.cpp ALContext.cpp ALSource.cpp ALStreamer.cpp
    GLBuffer.cpp GLCanvas.cpp GLContext.cpp GLFBO.cpp GLRender.cpp GLShader.cpp GLShaderIO.cpp GLState.cpp GLStatistics.cpp GLTexture.cpp GLTextureIO.cpp GLVertex.cpp
    #DEBody.cpp DEMass.cpp DEShape.cpp DEWorld.cpp
    RenderCamera.cpp RenderFont.cpp RenderIO.cpp RenderLight.cpp RenderMesh.cpp RenderParticle.cpp RenderQueue.cpp RenderScene.cpp RenderSprite.cpp RenderStage.cpp RenderStyle.cpp RenderTerrain.cpp
    Input.cpp
    UIBook.cpp UIButton.cpp UICanvas.cpp UIColor.cpp UIEntry.cpp UIItem.cpp UILabel.cpp UILayout.cpp UIList.cpp UIMenu.cpp UIPopup.cpp UIProgress.cpp UIRender.cpp UIScroller.cpp UISlider.cpp UIView.cpp UIWidget.cpp UIWindow.cpp
    #DebugHUD.cpp
    DemoConfig.cpp DemoEditor.cpp DemoEffect.cpp DemoPlayer.cpp DemoProperty.cpp DemoShow.cpp DemoTimeline.cpp)

add_library(wendy STATIC ${wendy_SOURCES})
install(TARGETS wendy ARCHIVE DESTINATION lib)


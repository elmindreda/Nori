///////////////////////////////////////////////////////////////////////
// Wendy library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <moira/Moira.h>

#include <wendy/Wendy.h>

#include <GL/glfw.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

Ptr<GL::TextureCodec> textureCodec;
Ptr<GL::VertexProgramCodec> vertexProgramCodec;
Ptr<GL::FragmentProgramCodec> fragmentProgramCodec;
Ptr<GL::ProgramCodec> programCodec;

Ptr<render::MaterialCodec> renderMaterialCodec;
Ptr<render::TerrainCodec> renderTerrainCodec;

#if WENDY_INCLUDE_SCENE_GRAPH
Ptr<scene::GraphCodecXML> sceneGraphCodec;
Ptr<scene::NodeType> sceneLightNodeType;
Ptr<scene::NodeType> sceneMeshNodeType;
Ptr<scene::NodeType> sceneCameraNodeType;
Ptr<scene::NodeType> sceneTerrainNodeType;
Ptr<scene::NodeType> sceneSpriteNodeType;
Ptr<scene::NodeType> sceneParticleNodeType;
#endif

#if WENDY_INCLUDE_DEMO_SYSTEM
Ptr<demo::ShowCodec> showCodec;
#endif

}

///////////////////////////////////////////////////////////////////////

bool initialize(void)
{
  if (!moira::initialize())
    return false;

  atexit(shutdown);

  if (!glfwInit())
  {
    Log::writeError("Failed to initialize GLFW");
    return false;
  }

  textureCodec = new GL::TextureCodec();
  vertexProgramCodec = new GL::VertexProgramCodec();
  fragmentProgramCodec = new GL::FragmentProgramCodec();
  programCodec = new GL::ProgramCodec();
  renderMaterialCodec = new render::MaterialCodec();
  renderTerrainCodec = new render::TerrainCodec();

#if WENDY_INCLUDE_SCENE_GRAPH
  sceneGraphCodec = new scene::GraphCodecXML();
  sceneLightNodeType = new scene::NodeTemplate<scene::LightNode>("Light");
  sceneMeshNodeType = new scene::NodeTemplate<scene::MeshNode>("Mesh");
  sceneCameraNodeType = new scene::NodeTemplate<scene::CameraNode>("Camera");
  sceneTerrainNodeType = new scene::NodeTemplate<scene::TerrainNode>("Terrain");
  sceneSpriteNodeType = new scene::NodeTemplate<scene::SpriteNode>("Sprite");
  sceneParticleNodeType = new scene::NodeTemplate<scene::ParticleSystemNode>("ParticleSystem");
#endif

#if WENDY_INCLUDE_DEMO_SYSTEM
  showCodec = new demo::ShowCodec();
#endif

  return true;
}

void shutdown(void)
{
  GL::Context::destroy();

  textureCodec = NULL;
  vertexProgramCodec = NULL;
  fragmentProgramCodec = NULL;
  programCodec = NULL;
  renderMaterialCodec = NULL;
  renderTerrainCodec = NULL;

#if WENDY_INCLUDE_SCENE_GRAPH
  sceneGraphCodec = NULL;
  sceneLightNodeType = NULL;
  sceneMeshNodeType = NULL;
  sceneCameraNodeType = NULL;
  sceneTerrainNodeType = NULL;
  sceneSpriteNodeType = NULL;
  sceneParticleNodeType = NULL;
#endif

#if WENDY_INCLUDE_DEMO_SYSTEM
  showCodec = NULL;
#endif

  glfwTerminate();

  moira::shutdown();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

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

Ptr<demo::ShowCodec> showCodec;

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
  showCodec = new demo::ShowCodec();

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
  showCodec = NULL;

  glfwTerminate();

  moira::shutdown();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

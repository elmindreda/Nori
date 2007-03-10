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

Ptr<GL::VertexShaderCodec> vertexShaderCodec;
Ptr<GL::FragmentShaderCodec> fragmentShaderCodec;
Ptr<GL::ShaderProgramCodec> shaderProgramCodec;

Ptr<render::StyleCodec> renderStyleCodec;

Ptr<demo::ShowCodec> showCodec;

}

///////////////////////////////////////////////////////////////////////

bool initialize(void)
{
  if (!moira::initialize())
    return false;

  if (!glfwInit())
    return false;

  vertexShaderCodec = new GL::VertexShaderCodec();
  fragmentShaderCodec = new GL::FragmentShaderCodec();
  shaderProgramCodec = new GL::ShaderProgramCodec();
  renderStyleCodec = new render::StyleCodec();
  showCodec = new demo::ShowCodec();

  if (!GL::VertexShader::addSearchPath(Path(".")))
    return false;

  if (!GL::FragmentShader::addSearchPath(Path(".")))
    return false;

  if (!GL::ShaderProgram::addSearchPath(Path(".")))
    return false;

  if (!render::Style::addSearchPath(Path(".")))
    return false;

  if (!demo::Show::addSearchPath(Path(".")))
    return false;

  return true;
}

void shutdown(void)
{
  GL::Context::destroy();

  vertexShaderCodec = NULL;
  fragmentShaderCodec = NULL;
  shaderProgramCodec = NULL;
  renderStyleCodec = NULL;

  showCodec = NULL;

  glfwTerminate();

  moira::shutdown();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

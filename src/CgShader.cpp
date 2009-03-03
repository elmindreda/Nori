///////////////////////////////////////////////////////////////////////
// Wendy Cg library
// Copyright (c) 2008 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.h>
#include <wendy/Cg.h>
#include <wendy/CgContext.h>
#include <wendy/CgShader.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace Cg
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Shader::~Shader(void)
{
  if (programID)
    cgDestroyProgram(programID);
}

Shader* Shader::createInstance(Domain domain, const String& text, const String& name)
{
  Ptr<Shader> shader = new Shader(domain, name);
  if (!shader->init(text))
    return NULL;

  return shader.detachObject();
}

Shader::Shader(Domain initDomain, const String& name):
  Resource<Shader>(name),
  domain(initDomain),
  programID(NULL)
{
}

bool Shader::init(const String& text)
{
  Context* context = Context::get();
  if (!context)
  {
    Log::writeError("Cannot create Cg shader without Cg context");
    return false;
  }

  CGprofile profile = context->getProfile(domain);
  if (profile == CG_PROFILE_UNKNOWN)
  {
    Log::writeError("Cannot create Cg shaders for domain");
    return false;
  }

  programID = cgCreateProgram(context->getID(), CG_SOURCE, text.c_str(), profile, NULL, NULL);
  if (!programID)
  {
    Log::writeError("Failed to compile Cg shader: %s", cgGetErrorString(cgGetError()));
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace Cg*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

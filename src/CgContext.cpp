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

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace Cg
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Context::~Context(void)
{
  if (contextID)
    cgDestroyContext(contextID);
}

CGcontext Context::getID(void) const
{
  return contextID;
}

CGprofile Context::getProfile(Domain domain) const
{
  switch (domain)
  {
    case VERTEX:
      return profiles[VERTEX];
    case FRAGMENT:
      return profiles[FRAGMENT];
    case GEOMETRY:
      return profiles[GEOMETRY];
    default:
      throw Exception("Invalid shader domain");
  }
}

bool Context::create(void)
{
  if (get())
    return true;

  Ptr<Context> context = new Context();
  if (!context->init())
    return false;

  set(context.detachObject());
  return true;
}

Context::Context(void):
  contextID(NULL)
{
}

bool Context::init(void)
{
  contextID = cgCreateContext();
  if (!contextID)
    return false;

  profiles[VERTEX] = cgGLGetLatestProfile(CG_GL_VERTEX);
  if (profiles[VERTEX] == CG_PROFILE_UNKNOWN)
    Log::writeWarning("No suitable Cg vertex shader profile found");

  profiles[FRAGMENT] = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  if (profiles[FRAGMENT] == CG_PROFILE_UNKNOWN)
    Log::writeWarning("No suitable Cg fragment shader profile found");

  profiles[GEOMETRY] = cgGLGetLatestProfile(CG_GL_GEOMETRY);
  if (profiles[GEOMETRY] == CG_PROFILE_UNKNOWN)
    Log::writeWarning("No suitable Cg geometry shader profile found");

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace Cg*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

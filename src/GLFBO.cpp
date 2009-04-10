///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/GLContext.h>
#include <wendy/GLFBO.h>

#define GLEW_STATIC
#include <GL/glew.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Framebuffer::~Framebuffer(void)
{
  if (bufferID)
    glDeleteFramebuffersEXT(1, &bufferID);
}

Framebuffer* Framebuffer::createInstance(Context& context)
{
  Ptr<Framebuffer> framebuffer = new Framebuffer(context);
  if (!framebuffer->init())
    return NULL;

  return framebuffer.detachObject();
}

Framebuffer::Framebuffer(Context& initContext):
  context(initContext)
{
}

bool Framebuffer::init(void)
{
  if (!GLEW_EXT_framebuffer_object)
  {
    Log::writeError("Framebuffer objects are not supported by the current OpenGL context");
    return false;
  }

  glGenFramebuffersEXT(1, &bufferID);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bufferID);

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

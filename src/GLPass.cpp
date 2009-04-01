///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLStatistics.h>
#include <wendy/GLTexture.h>
#include <wendy/GLShader.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLState.h>
#include <wendy/GLPass.h>

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

namespace
{

GLenum convertCullMode(CullMode mode)
{
  switch (mode)
  {
    case CULL_FRONT:
      return GL_FRONT;
    case CULL_BACK:
      return GL_BACK;
    case CULL_BOTH:
      return GL_FRONT_AND_BACK;
    default:
      throw Exception("Invalid cull mode");
  }
}

CullMode invertCullMode(CullMode mode)
{
  switch (mode)
  {
    case CULL_NONE:
      return CULL_BOTH;
    case CULL_FRONT:
      return CULL_BACK;
    case CULL_BACK:
      return CULL_FRONT;
    case CULL_BOTH:
      return CULL_NONE;
    default:
      throw Exception("Invalid cull mode");
  }
}

GLenum convertFunction(Function function)
{
  switch (function)
  {
    case ALLOW_NEVER:
      return GL_NEVER;
    case ALLOW_ALWAYS:
      return GL_ALWAYS;
    case ALLOW_EQUAL:
      return GL_EQUAL;
    case ALLOW_NOT_EQUAL:
      return GL_NOTEQUAL;
    case ALLOW_LESSER:
      return GL_LESS;
    case ALLOW_LESSER_EQUAL:
      return GL_LEQUAL;
    case ALLOW_GREATER:
      return GL_GREATER;
    case ALLOW_GREATER_EQUAL:
      return GL_GEQUAL;
    default:
      throw Exception("Invalid function");
  }
}

GLenum convertBlendFactor(BlendFactor factor)
{
  switch (factor)
  {
    case BLEND_ZERO:
      return GL_ZERO;
    case BLEND_ONE:
      return GL_ONE;
    case BLEND_SRC_COLOR:
      return GL_SRC_COLOR;
    case BLEND_DST_COLOR:
      return GL_DST_COLOR;
    case BLEND_SRC_ALPHA:
      return GL_SRC_ALPHA;
    case BLEND_DST_ALPHA:
      return GL_DST_ALPHA;
    case BLEND_ONE_MINUS_SRC_COLOR:
      return GL_ONE_MINUS_SRC_COLOR;
    case BLEND_ONE_MINUS_DST_COLOR:
      return GL_ONE_MINUS_DST_COLOR;
    case BLEND_ONE_MINUS_SRC_ALPHA:
      return GL_ONE_MINUS_SRC_ALPHA;
    case BLEND_ONE_MINUS_DST_ALPHA:
      return GL_ONE_MINUS_DST_ALPHA;
    default:
      throw Exception("Invalid blend factor");
  }
}

}

///////////////////////////////////////////////////////////////////////

Pass::Pass(const String& initName):
  name(initName)
{
}

Pass::~Pass(void)
{
}

void Pass::apply(void) const
{
  // NOTE: Yes, I know this is huge.  You don't need to point it out.

  if (Statistics* statistics = Statistics::get())
    statistics->addPasses(1);

  if (cache.dirty)
  {
    force();
    return;
  }
  
  CullMode cullMode = data.cullMode;
  if (cullingInverted)
    cullMode = invertCullMode(cullMode);

  if (cullMode != cache.cullMode)
  {
    if ((cullMode == CULL_NONE) != (cache.cullMode == CULL_NONE))
      setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);

    if (cullMode != CULL_NONE)
      glCullFace(convertCullMode(cullMode));
      
    cache.cullMode = cullMode;
  }

  if (data.srcFactor != cache.srcFactor || data.dstFactor != cache.dstFactor)
  {
    setBooleanState(GL_BLEND, data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO);

    if (data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO)
      glBlendFunc(convertBlendFactor(data.srcFactor), convertBlendFactor(data.dstFactor));
    
    cache.srcFactor = data.srcFactor;
    cache.dstFactor = data.dstFactor;
  }

  if (data.depthTesting || data.depthWriting)
  {
    // Set depth buffer writing.
    if (data.depthWriting != cache.depthWriting)
      glDepthMask(data.depthWriting ? GL_TRUE : GL_FALSE);

    if (data.depthTesting)
    {
      // Set depth buffer function.
      if (data.depthFunction != cache.depthFunction)
      {
        glDepthFunc(convertFunction(data.depthFunction));
        cache.depthFunction = data.depthFunction;
      }
    }
    else if (data.depthWriting)
    {
      // NOTE: Special case; depth buffer filling.
      //       Set specific depth buffer function.
      const Function depthFunction = ALLOW_ALWAYS;

      if (cache.depthFunction != depthFunction)
      {
        glDepthFunc(convertFunction(depthFunction));
        cache.depthFunction = depthFunction;
      }
    }

    if (!(cache.depthTesting || cache.depthWriting))
      glEnable(GL_DEPTH_TEST);
  }
  else
  {
    if (cache.depthTesting || cache.depthWriting)
      glDisable(GL_DEPTH_TEST);
  }

  cache.depthTesting = data.depthTesting;
  cache.depthWriting = data.depthWriting;

  if (data.colorWriting != cache.colorWriting)
  {
    const GLboolean state = data.colorWriting ? GL_TRUE : GL_FALSE;
    glColorMask(state, state, state, state);
    cache.colorWriting = data.colorWriting;
  }

  if (data.wireframe != cache.wireframe)
  {
    const GLenum state = data.wireframe ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, state);
    cache.wireframe = data.wireframe;
  }
  
  ProgramState::apply();

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying render pass: %s", gluErrorString(error));
#endif

  data.dirty = false;
}

bool Pass::isCompatible(void) const
{
  // TODO: The code.

  return true;
}

bool Pass::isCulling(void) const
{
  return data.cullMode != CULL_NONE;
}

bool Pass::isBlending(void) const
{
  return data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO;
}

bool Pass::isDepthTesting(void) const
{
  return data.depthTesting;
}

bool Pass::isDepthWriting(void) const
{
  return data.depthWriting;
}

bool Pass::isColorWriting(void) const
{
  return data.colorWriting;
}

bool Pass::isWireframe(void) const
{
  return data.wireframe;
}

CullMode Pass::getCullMode(void) const
{
  return data.cullMode;
}

BlendFactor Pass::getSrcFactor(void) const
{
  return data.srcFactor;
}

BlendFactor Pass::getDstFactor(void) const
{
  return data.dstFactor;
}

Function Pass::getDepthFunction(void) const
{
  return data.depthFunction;
}

const String& Pass::getName(void) const
{
  return name;
}

void Pass::setDepthTesting(bool enable)
{
  data.depthTesting = enable;
  data.dirty = true;
}

void Pass::setDepthWriting(bool enable)
{
  data.depthWriting = enable;
  data.dirty = true;
}

void Pass::setCullMode(CullMode mode)
{
  data.cullMode = mode;
  data.dirty = true;
}

void Pass::setBlendFactors(BlendFactor src, BlendFactor dst)
{
  data.srcFactor = src;
  data.dstFactor = dst;
  data.dirty = true;
}

void Pass::setDepthFunction(Function function)
{
  data.depthFunction = function;
  data.dirty = true;
}

void Pass::setColorWriting(bool enabled)
{
  data.colorWriting = enabled;
  data.dirty = true;
}

void Pass::setWireframe(bool enabled)
{
  data.wireframe = enabled;
  data.dirty = true;
}

void Pass::setDefaults(void)
{
  data.setDefaults();
}

bool Pass::isCullingInverted(void)
{
  return cullingInverted;
}

void Pass::setCullingInversion(bool newState)
{
  cullingInverted = newState;
}

void Pass::force(void) const
{
  cache = data;

  CullMode cullMode = data.cullMode;
  if (cullingInverted)
    cullMode = invertCullMode(cullMode);

  setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);
  if (cullMode != CULL_NONE)
    glCullFace(convertCullMode(cullMode));

  setBooleanState(GL_BLEND, data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO);
  glBlendFunc(convertBlendFactor(data.srcFactor), convertBlendFactor(data.dstFactor));
  
  unsigned int height;

  if (Canvas* canvas = Canvas::getCurrent())
    height = canvas->getPhysicalHeight();
  else
    height = Context::get()->getHeight();

  glDepthMask(data.depthWriting ? GL_TRUE : GL_FALSE);
  setBooleanState(GL_DEPTH_TEST, data.depthTesting || data.depthWriting);

  if (data.depthWriting && !data.depthTesting)
  {
    const Function depthFunction = ALLOW_ALWAYS;
    glDepthFunc(convertFunction(depthFunction));
    cache.depthFunction = depthFunction;
  }
  else
    glDepthFunc(convertFunction(data.depthFunction));

  const GLboolean state = data.colorWriting ? GL_TRUE : GL_FALSE;
  glColorMask(state, state, state, state);

  const GLenum polygonMode = data.wireframe ? GL_LINE : GL_FILL;
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

  ProgramState::apply();

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeWarning("Error when forcing render pass: %s", gluErrorString(error));
#endif

  cache.dirty = data.dirty = false;
}

void Pass::setBooleanState(unsigned int state, bool value) const
{
  if (value)
    glEnable(state);
  else
    glDisable(state);
}

Pass::Data Pass::cache;

bool Pass::cullingInverted = false;

///////////////////////////////////////////////////////////////////////

Pass::Data::Data(void)
{
  setDefaults();
}

void Pass::Data::setDefaults(void)
{
  dirty = true;
  depthTesting = true;
  depthWriting = true;
  colorWriting = true;
  wireframe = false;
  cullMode = CULL_BACK;
  srcFactor = BLEND_ONE;
  dstFactor = BLEND_ZERO;
  depthFunction = ALLOW_LESSER;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

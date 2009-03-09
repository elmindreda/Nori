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
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLStatistics.h>
#include <wendy/GLTexture.h>
#include <wendy/GLShader.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLPass.h>

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

}

///////////////////////////////////////////////////////////////////////

Pass::Pass(const String& initName):
  name(initName)
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
      glCullFace(cullMode);
      
    cache.cullMode = cullMode;
  }

  if (data.srcFactor != cache.srcFactor || data.dstFactor != cache.dstFactor)
  {
    setBooleanState(GL_BLEND, data.srcFactor != GL_ONE || data.dstFactor != GL_ZERO);

    if (data.srcFactor != GL_ONE || data.dstFactor != GL_ZERO)
      glBlendFunc(data.srcFactor, data.dstFactor);
    
    cache.srcFactor = data.srcFactor;
    cache.dstFactor = data.dstFactor;
  }

  if (data.polygonMode != cache.polygonMode)
  {
    glPolygonMode(GL_FRONT_AND_BACK, data.polygonMode);
    cache.polygonMode = data.polygonMode;
  }

  if (data.lineWidth != cache.lineWidth)
  {
    unsigned int height;

    if (Canvas* canvas = Canvas::getCurrent())
      height = canvas->getPhysicalHeight();
    else
      height = Context::get()->getHeight();

    glLineWidth(data.lineWidth * height / 100.f);
    cache.lineWidth = data.lineWidth;
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
        glDepthFunc(data.depthFunction);
        cache.depthFunction = data.depthFunction;
      }
    }
    else if (data.depthWriting)
    {
      // NOTE: Special case; depth buffer filling.
      //       Set specific depth buffer function.
      const GLenum depthFunction = GL_ALWAYS;

      if (cache.depthFunction != depthFunction)
      {
        glDepthFunc(depthFunction);
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
  
  if (data.program)
  {
    data.program->apply();
    cache.program = data.program;
  }
  else
  {
    // TODO: Get default shader program.
  }

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying render pass: %s", gluErrorString(error));
#endif

  TextureStack::apply();
  
  data.dirty = false;
}

bool Pass::isCompatible(void) const
{
  return TextureStack::isCompatible();
}

bool Pass::isCulling(void) const
{
  return data.cullMode != CULL_NONE;
}

bool Pass::isBlending(void) const
{
  return data.srcFactor != GL_ONE || data.dstFactor != GL_ZERO;
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

float Pass::getLineWidth(void) const
{
  return data.lineWidth;
}

CullMode Pass::getCullMode(void) const
{
  return data.cullMode;
}

GLenum Pass::getPolygonMode(void) const
{
  return data.polygonMode;
}

GLenum Pass::getSrcFactor(void) const
{
  return data.srcFactor;
}

GLenum Pass::getDstFactor(void) const
{
  return data.dstFactor;
}

GLenum Pass::getDepthFunction(void) const
{
  return data.depthFunction;
}

GLenum Pass::getAlphaFunction(void) const
{
  return data.alphaFunction;
}

Texture& Pass::getSamplerTexture(unsigned int index) const
{
  if (index >= samplers.size())
    throw Exception("Invalid sampler index");

  return *samplers[index].texture;
}

Texture& Pass::getSamplerTexture(const String& name) const
{
  Sampler* sampler = findSampler(name)
  if (!sampler)
    throw Exception("Invalid sampler name");

  return *(sampler->texture);
}

unsigned int Pass::getSamplerCount(void) const
{
  return samplers.size();
}

Program* Pass::getProgram(void) const
{
  return data.program;
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

void Pass::setLineWidth(float width)
{
  data.lineWidth = width;
  data.dirty = true;
}

void Pass::setCullMode(CullMode mode)
{
  data.cullMode = mode;
  data.dirty = true;
}

void Pass::setPolygonMode(GLenum mode)
{
  data.polygonMode = mode;
  data.dirty = true;
}

void Pass::setBlendFactors(GLenum src, GLenum dst)
{
  data.srcFactor = src;
  data.dstFactor = dst;
  data.dirty = true;
}

void Pass::setDepthFunction(GLenum function)
{
  data.depthFunction = function;
  data.dirty = true;
}

void Pass::setAlphaFunction(GLenum function)
{
  data.alphaFunction = function;
  data.dirty = true;
}

void Pass::setColorWriting(bool enabled)
{
  data.colorWriting = enabled;
  data.dirty = true;
}

bool Pass::setSamplerTexture(const String& samplerName, Texture& texture)
{
}

void Pass::setProgram(Program* newProgram)
{
  if (newProgram)
    data.program = &newProgram;
  else
  {
    data.program = Program::readInstance("default");
    if (!data.program)
      throw Exception("Default shader program missing");
  }

  data.dirty = true;

  Ref<Texture> texture = Texture::readInstance("default");
  if (!texture)
    throw Exception("Default texture missing");

  samplers.clean();

  for (unsigned int i = 0;  i < data.program->getUniformCount();  i++)
  {
    ShaderUniform* uniform = data.program->getUniform(i);
    if (uniform->isSampler())
      samplers.push_back(Sampler(uniform->getName(), ));

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
    glCullFace(cullMode);

  setBooleanState(GL_BLEND, data.srcFactor != GL_ONE || data.dstFactor != GL_ZERO);
  glBlendFunc(data.srcFactor, data.dstFactor);
  
  glPolygonMode(GL_FRONT_AND_BACK, data.polygonMode);

  unsigned int height;

  if (Canvas* canvas = Canvas::getCurrent())
    height = canvas->getPhysicalHeight();
  else
    height = Context::get()->getHeight();

  glLineWidth(data.lineWidth * height / 100.f);

  glDepthMask(data.depthWriting ? GL_TRUE : GL_FALSE);
  setBooleanState(GL_DEPTH_TEST, data.depthTesting || data.depthWriting);

  if (data.depthWriting && !data.depthTesting)
  {
    const GLenum depthFunction = GL_ALWAYS;
    glDepthFunc(depthFunction);
    cache.depthFunction = depthFunction;
  }
  else
    glDepthFunc(data.depthFunction);

  const GLboolean state = data.colorWriting ? GL_TRUE : GL_FALSE;
  glColorMask(state, state, state, state);

  if (data.program)
    data.program->apply();
  else
  {
    // TODO: Get default shader program.
  }

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeWarning("Error when forcing render pass: %s", gluErrorString(error));
#endif

  TextureStack::apply();
  
  cache.dirty = data.dirty = false;
}

void Pass::setBooleanState(GLenum state, bool value) const
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
  lineWidth = 1.f;
  cullMode = CULL_BACK;
  polygonMode = GL_FILL;
  srcFactor = GL_ONE;
  dstFactor = GL_ZERO;
  depthFunction = GL_LESS;
  alphaFunction = GL_ALWAYS;
  program = NULL;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

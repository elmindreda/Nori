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
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLTexture.h>
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
  
  if (data.cullMode != cache.cullMode)
  {
    if ((data.cullMode == CULL_NONE) != (cache.cullMode == CULL_NONE))
      setBooleanState(GL_CULL_FACE, data.cullMode != CULL_NONE);

    if (data.cullMode != CULL_NONE)
      glCullFace(data.cullMode);
      
    cache.cullMode = data.cullMode;
  }

  if (data.lighting != cache.lighting)
  {
    setBooleanState(GL_LIGHTING, data.lighting);
    cache.lighting = data.lighting;
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
  
  if (data.lighting)
  {
    // Set ambient material color.
    if (data.ambientColor != cache.ambientColor)
    {
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, data.ambientColor);
      cache.ambientColor = data.ambientColor;
    }

    // Set diffuse material color.
    if (data.diffuseColor != cache.diffuseColor)
    {
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, data.diffuseColor);
      cache.diffuseColor = data.diffuseColor;
    }

    // Set specular material color.
    if (data.specularColor != cache.specularColor)
    {
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, data.specularColor);
      cache.specularColor = data.specularColor;
    }

    if (data.shininess != cache.shininess)
    {
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, data.shininess);
      cache.shininess = data.shininess;
    }
  }
  else
  {
    // For compatibility reasons, we do not trust the cached color.  Since we
    // always overwrite this value, there is no need to check whether the cache
    // is dirty.
    
    glColor4fv(data.defaultColor);
    cache.defaultColor = data.defaultColor;
  }

  if (GLEW_ARB_shading_language_100)
  {
    // Since the GLSL program object cannot push the currently active
    // program in any resonable fashion, it must force the use when
    // changing uniforms in a program object.  Hence we cannot trust the
    // state cache's program name to be valid between calls.  Thus we
    // always force the use of the correct program.

    if (data.shaderProgramName.empty())
    {
      ShaderProgram::applyFixedFunction();
      cache.shaderProgramName.clear();
    }
    else
    {
      ShaderProgram* program = ShaderProgram::findInstance(data.shaderProgramName);
      if (program)
      {
	program->apply();
	cache.shaderProgramName = data.shaderProgramName;
      }
      else
	Log::writeError("Render pass uses non-existent GLSL program %s",
	                data.shaderProgramName.c_str());
    }
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
  if (!GLEW_ARB_shading_language_100)
  {
    if (!data.shaderProgramName.empty())
      return false;
  }

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

bool Pass::isLit(void) const
{
  return data.lighting;
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

float Pass::getShininess(void) const
{
  return data.shininess;
}

const ColorRGBA& Pass::getDefaultColor(void) const
{
  return data.defaultColor;
}

const ColorRGBA& Pass::getAmbientColor(void) const
{
  return data.ambientColor;
}

const ColorRGBA& Pass::getDiffuseColor(void) const
{
  return data.diffuseColor;
}

const ColorRGBA& Pass::getSpecularColor(void) const
{
  return data.specularColor;
}

const String& Pass::getShaderProgramName(void) const
{
  return data.shaderProgramName;
}

const String& Pass::getName(void) const
{
  return name;
}

void Pass::setLit(bool enable)
{
  data.lighting = enable;
  data.dirty = true;
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

void Pass::setShininess(float newValue)
{
  data.shininess = newValue;
  data.dirty = true;
}

void Pass::setDefaultColor(const ColorRGBA& color)
{
  data.defaultColor = color;
  data.dirty = true;
}

void Pass::setAmbientColor(const ColorRGBA& color)
{
  data.ambientColor = color;
  data.dirty = true;
}

void Pass::setDiffuseColor(const ColorRGBA& color)
{
  data.diffuseColor = color;
  data.dirty = true;
}

void Pass::setSpecularColor(const ColorRGBA& color)
{
  data.specularColor = color;
  data.dirty = true;
}

void Pass::setShaderProgramName(const String& newName)
{
  data.shaderProgramName = newName;
  data.dirty = true;
}

void Pass::setDefaults(void)
{
  data.setDefaults();
}

void Pass::force(void) const
{
  cache = data;

  setBooleanState(GL_CULL_FACE, data.cullMode != CULL_NONE);
  if (data.cullMode != CULL_NONE)
    glCullFace(data.cullMode);

  setBooleanState(GL_LIGHTING, data.lighting);

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

  glColor4fv(data.defaultColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, data.ambientColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, data.diffuseColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, data.specularColor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, data.shininess);

  if (GLEW_ARB_shader_objects)
  {
    if (data.shaderProgramName.empty())
      ShaderProgram::applyFixedFunction();
    else
    {
      ShaderProgram* program = ShaderProgram::findInstance(data.shaderProgramName);
      if (program)
	program->apply();
      else
	Log::writeError("Render pass uses non-existent shader program %s",
			data.shaderProgramName.c_str());
    }
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

///////////////////////////////////////////////////////////////////////

Pass::Data::Data(void)
{
  setDefaults();
}

void Pass::Data::setDefaults(void)
{
  dirty = true;
  lighting = false;
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
  shininess = 0.f;
  defaultColor.set(1.f, 1.f, 1.f, 1.f);
  ambientColor.set(0.f, 0.f, 0.f, 1.f);
  diffuseColor.set(1.f, 1.f, 1.f, 1.f);
  specularColor.set(1.f, 1.f, 1.f, 1.f);
  shaderProgramName.clear();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

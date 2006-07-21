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
#include <wendy/GLTexture.h>
#include <wendy/GLLight.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>

#include <algorithm>

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

inline unsigned int max(unsigned int x, unsigned int y)
{
  if (x > y)
    return x;
  else
    return y;
}

void onContextDestroy(void)
{
  Renderer::destroy();
}

}

///////////////////////////////////////////////////////////////////////

RenderPass::RenderPass(void)
{
  setDefaults();
}

void RenderPass::apply(void) const
{
  CullMode inverseCullMode;

  switch (data.cullMode)
  {
    case CULL_NONE:
      inverseCullMode = CULL_BOTH;
      break;
    case CULL_FRONT:
      inverseCullMode = CULL_BACK;
      break;
    case CULL_BACK:
      inverseCullMode = CULL_FRONT;
      break;
    case CULL_BOTH:
      inverseCullMode = CULL_NONE;
      break;
  }

  if (cache.dirty)
  {
    // Force all states to known values

    cache = data;

    setBooleanState(GL_CULL_FACE, data.cullMode != CULL_NONE);
    if (data.cullMode != CULL_NONE)
      glCullFace(data.cullMode);

    setBooleanState(GL_LIGHTING, data.lighting);

    setBooleanState(GL_BLEND, data.srcFactor != GL_ONE || data.dstFactor != GL_ZERO);
    glBlendFunc(data.srcFactor, data.dstFactor);
    
    glShadeModel(data.shadeMode);

    glPolygonMode(GL_FRONT_AND_BACK, data.polygonMode);

    glLineWidth(data.lineWidth);

    glDepthMask(data.depthWriting ? GL_TRUE : GL_FALSE);
    setBooleanState(GL_DEPTH_TEST, data.depthTesting || data.depthWriting);

    if (data.depthWriting && !data.depthTesting)
    {
      GLenum depthFunction = GL_ALWAYS;
      glDepthFunc(depthFunction);
      cache.depthFunction = depthFunction;
    }
    else
      glDepthFunc(data.depthFunction);

    setBooleanState(GL_STENCIL_TEST, data.stencilTesting);
    glStencilFunc(data.stencilFunction, data.stencilRef, data.stencilMask);
    glStencilOp(data.stencilFailed, data.depthFailed, data.depthPassed);

    glColor4fv(data.defaultColor);
    glMaterialfv(inverseCullMode, GL_AMBIENT, data.ambientColor);
    glMaterialfv(inverseCullMode, GL_DIFFUSE, data.diffuseColor);
    glMaterialfv(inverseCullMode, GL_SPECULAR, data.specularColor);
    glMaterialf(inverseCullMode, GL_SHININESS, data.shininess);

    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_3D);

    setBooleanState(GL_TEXTURE_GEN_S, data.sphereMapped);
    setBooleanState(GL_TEXTURE_GEN_T, data.sphereMapped);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, data.combineMode);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, data.combineColor);

    if (!data.textureName.empty())
    {
      Texture* texture = Texture::findInstance(data.textureName);
      if (texture)
      {
        GLenum textureTarget = texture->getTarget();

        glEnable(textureTarget);
        glBindTexture(textureTarget, texture->getGLID());

        cache.textureTarget = textureTarget;
      }
      else
	Log::writeError("Render pass uses non-existent texture %s", data.textureName.c_str());
    }

    if (GLEW_ARB_vertex_program)
    {
      if (data.vertexProgramName.empty())
	glDisable(GL_VERTEX_PROGRAM_ARB);
      else
      {
	VertexProgram* program = VertexProgram::findInstance(data.vertexProgramName);
	if (program)
	{
	  glEnable(GL_VERTEX_PROGRAM_ARB);
	  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, program->getGLID());
	}
	else
	  Log::writeError("Render pass uses non-existent vertex program %s",
	                  data.vertexProgramName.c_str());
      }
    }
    else
    {
      if (!data.vertexProgramName.empty())
	Log::writeError("Vertex programs are not supported by the current OpenGL context");
    }

    if (GLEW_ARB_fragment_program)
    {
      if (data.fragmentProgramName.empty())
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
      else
      {
	FragmentProgram* program = FragmentProgram::findInstance(data.fragmentProgramName);
	if (program)
	{
	  glEnable(GL_FRAGMENT_PROGRAM_ARB);
	  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program->getGLID());
	}
	else
	  Log::writeError("Render pass uses non-existent fragment program %s",
	                  data.fragmentProgramName.c_str());
      }
    }
    else
    {
      if (!data.fragmentProgramName.empty())
	Log::writeError("Fragment programs are not supported by the current OpenGL context");
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      Log::writeWarning("Error when forcing render pass: %s", gluErrorString(error));
    
    cache.dirty = data.dirty = false;
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

  if (data.shadeMode != cache.shadeMode)
  {
    glShadeModel(data.shadeMode);
    cache.shadeMode = data.shadeMode;
  }

  if (data.polygonMode != cache.polygonMode)
  {
    glPolygonMode(GL_FRONT_AND_BACK, data.polygonMode);
    cache.polygonMode = data.polygonMode;
  }

  if (data.lineWidth != cache.lineWidth)
  {
    glLineWidth(data.lineWidth);
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
      const unsigned int depthFunction = GL_ALWAYS;

      if (cache.depthFunction != depthFunction)
      {
        glDepthFunc(depthFunction);
        cache.depthFunction = depthFunction;
      }
    }
  }
  
  if (data.depthTesting || data.depthWriting)
  {
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

  if (data.stencilTesting)
  {
    if (!cache.stencilTesting)
    {
      glEnable(GL_STENCIL_TEST);
      cache.stencilTesting = data.stencilTesting;
    }

    if (data.stencilFunction != cache.stencilFunction ||
        data.stencilRef != cache.stencilRef ||
        data.stencilMask != cache.stencilMask)
    {
      glStencilFunc(data.stencilFunction, data.stencilRef, data.stencilMask);
      cache.stencilFunction = data.stencilFunction;
      cache.stencilRef = data.stencilRef;
      cache.stencilMask = data.stencilMask;
    }

    if (data.stencilFailed != cache.stencilFailed ||
        data.depthFailed != cache.depthFailed ||
        data.depthPassed != cache.depthPassed)
    {
      glStencilOp(data.stencilFailed, data.depthFailed, data.depthPassed);
      cache.stencilFailed = data.stencilFailed;
      cache.depthFailed = data.depthFailed;
      cache.depthPassed = data.depthPassed;
    }
  }
  else
  {
    if (cache.stencilTesting)
    {
      glDisable(GL_STENCIL_TEST);
      cache.stencilTesting = data.stencilTesting;
    }
  }
  
  if (data.lighting)
  {
    // Set ambient material color.
    if (data.ambientColor != cache.ambientColor)
    {
      glMaterialfv(inverseCullMode, GL_AMBIENT, data.ambientColor);
      cache.ambientColor = data.ambientColor;
    }

    // Set diffuse material color.
    if (data.diffuseColor != cache.diffuseColor)
    {
      glMaterialfv(inverseCullMode, GL_DIFFUSE, data.diffuseColor);
      cache.diffuseColor = data.diffuseColor;
    }

    // Set specular material color.
    if (data.specularColor != cache.specularColor)
    {
      glMaterialfv(inverseCullMode, GL_SPECULAR, data.specularColor);
      cache.specularColor = data.specularColor;
    }

    if (data.shininess != cache.shininess)
    {
      glMaterialf(inverseCullMode, GL_SHININESS, data.shininess);
      cache.shininess = data.shininess;
    }
  }

  if (!data.lighting)
  {
    // For compatibility reasons, we do not trust the cached color.
    // Since we always overwrite this value, there is no need to
    // check whether the cache is dirty.
    
    glColor4fv(data.defaultColor);
    cache.defaultColor = data.defaultColor;
  }

  if (data.textureName.empty())
  {
    if (!cache.textureName.empty())
      glDisable(cache.textureTarget);

    cache.textureTarget = 0;
    cache.textureName.clear();
  }
  else
  {
    // Retrieve texture object.
    Texture* texture = Texture::findInstance(data.textureName);
    if (texture)
    {
      GLenum textureTarget = texture->getTarget();

      if (textureTarget != cache.textureTarget)
      {
        if (cache.textureTarget)
          glDisable(cache.textureTarget);

        glEnable(textureTarget);
        cache.textureTarget = textureTarget;
      }
      
      if (data.textureName != cache.textureName)
      {
        glBindTexture(textureTarget, texture->getGLID());
        cache.textureName = data.textureName;
      }

      if (data.combineMode != cache.combineMode)
      {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, data.combineMode);
        cache.combineMode = data.combineMode;
      }

      // Set texture environment color.
      if (data.combineColor != cache.combineColor)
      {
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, data.combineColor);
        cache.combineColor = data.combineColor;
      }

      if (data.sphereMapped != cache.sphereMapped)
      {
        if (data.sphereMapped)
        {
          glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
          glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
          glEnable(GL_TEXTURE_GEN_S);
          glEnable(GL_TEXTURE_GEN_T);
        }
        else
        {
          glDisable(GL_TEXTURE_GEN_S);
          glDisable(GL_TEXTURE_GEN_T);
        }

        cache.sphereMapped = data.sphereMapped;
      }
    }
    else
      Log::writeError("Render pass uses non-existent texture %s", data.textureName.c_str());
  }

  if (GLEW_ARB_vertex_program)
  {
    if (data.vertexProgramName.empty())
    {
      if (!cache.vertexProgramName.empty())
	glDisable(GL_VERTEX_PROGRAM_ARB);

      cache.vertexProgramName.clear();
    }
    else
    {
      VertexProgram* program = VertexProgram::findInstance(data.vertexProgramName);
      if (program)
      {
	if (cache.vertexProgramName.empty())
	  glEnable(GL_VERTEX_PROGRAM_ARB);

	// Since the vertex program object cannot push its binding in any
	// resonable fashion, it must force the binding when creating and
	// changing parameters on a program object.  Hence we cannot trust
	// the state cache's program name to be valid between calls.  Thus
	// we always force the binding to the correct value.
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, program->getGLID());
	cache.vertexProgramName = data.vertexProgramName;
      }
      else
	Log::writeError("Render pass uses non-existent vertex program %s",
	                data.vertexProgramName.c_str());
    }
  }
  else
  {
    if (!data.vertexProgramName.empty())
      Log::writeError("Vertex programs are not supported by the current OpenGL context");
  }

  if (GLEW_ARB_fragment_program)
  {
    if (data.fragmentProgramName.empty())
    {
      if (!cache.fragmentProgramName.empty())
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

      cache.fragmentProgramName.clear();
    }
    else
    {
      FragmentProgram* program = FragmentProgram::findInstance(data.fragmentProgramName);
      if (program)
      {
	if (cache.fragmentProgramName.empty())
	  glEnable(GL_FRAGMENT_PROGRAM_ARB);

	// Since the fragment program object cannot push its binding in any
	// resonable fashion, it must force the binding when creating and
	// changing parameters on a program object.  Hence we cannot trust
	// the state cache's program name to be valid between calls.  Thus
	// we always force the binding to the correct value.
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program->getGLID());
	cache.fragmentProgramName = data.fragmentProgramName;
      }
      else
	Log::writeError("Render pass uses non-existent fragment program %s",
	                data.fragmentProgramName.c_str());
    }
  }
  else
  {
    if (!data.fragmentProgramName.empty())
      Log::writeError("Fragment programs are not supported by the current OpenGL context");
  }

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeWarning("Error when applying render pass: %s", gluErrorString(error));
  
  data.dirty = cache.dirty = false;
}

bool RenderPass::isDirty(void) const
{
  return data.dirty;
}

bool RenderPass::isCulling(void) const
{
  return data.cullMode != CULL_NONE;
}

bool RenderPass::isBlending(void) const
{
  return data.srcFactor != GL_ONE || data.dstFactor != GL_ZERO;
}

bool RenderPass::isDepthTesting(void) const
{
  return data.depthTesting;
}

bool RenderPass::isDepthWriting(void) const
{
  return data.depthWriting;
}

bool RenderPass::isStencilTesting(void) const
{
  return data.stencilTesting;
}

bool RenderPass::isSphereMapped(void) const
{
  return data.sphereMapped;
}

bool RenderPass::isLit(void) const
{
  return data.lighting;
}

float RenderPass::getLineWidth(void) const
{
  return data.lineWidth;
}

CullMode RenderPass::getCullMode(void) const
{
  return data.cullMode;
}

GLenum RenderPass::getCombineMode(void) const
{
  return data.combineMode;
}

GLenum RenderPass::getPolygonMode(void) const
{
  return data.polygonMode;
}

GLenum RenderPass::getShadeMode(void) const
{
  return data.shadeMode;
}

GLenum RenderPass::getSrcFactor(void) const
{
  return data.srcFactor;
}

GLenum RenderPass::getDstFactor(void) const
{
  return data.dstFactor;
}

GLenum RenderPass::getDepthFunction(void) const
{
  return data.depthFunction;
}

GLenum RenderPass::getAlphaFunction(void) const
{
  return data.alphaFunction;
}

GLenum RenderPass::getStencilFunction(void) const
{
  return data.stencilFunction;
}

GLenum RenderPass::getStencilFailOperation(void) const
{
  return data.stencilFailed;
}

GLenum RenderPass::getDepthFailOperation(void) const
{
  return data.depthFailed;
}

GLenum RenderPass::getDepthPassOperation(void) const
{
  return data.depthPassed;
}

unsigned int RenderPass::getStencilReference(void) const
{
  return data.stencilRef;
}

unsigned int RenderPass::getStencilMask(void) const
{
  return data.stencilMask;
}

float RenderPass::getShininess(void) const
{
  return data.shininess;
}

const ColorRGBA& RenderPass::getDefaultColor(void) const
{
  return data.defaultColor;
}

const ColorRGBA& RenderPass::getAmbientColor(void) const
{
  return data.ambientColor;
}

const ColorRGBA& RenderPass::getDiffuseColor(void) const
{
  return data.diffuseColor;
}

const ColorRGBA& RenderPass::getSpecularColor(void) const
{
  return data.specularColor;
}

const ColorRGBA& RenderPass::getCombineColor(void) const
{
  return data.combineColor;
}

const String& RenderPass::getTextureName(void) const
{
  return data.textureName;
}

const String& RenderPass::getVertexProgramName(void) const
{
  return data.vertexProgramName;
}

const String& RenderPass::getFragmentProgramName(void) const
{
  return data.fragmentProgramName;
}

void RenderPass::setSphereMapped(bool enabled)
{
  data.sphereMapped = enabled;
  data.dirty = true;
}

void RenderPass::setLit(bool enable)
{
  data.lighting = enable;
  data.dirty = true;
}

void RenderPass::setDepthTesting(bool enable)
{
  data.depthTesting = enable;
  data.dirty = true;
}

void RenderPass::setDepthWriting(bool enable)
{
  data.depthWriting = enable;
  data.dirty = true;
}

void RenderPass::setStencilTesting(bool enable)
{
  data.stencilTesting = enable;
  data.dirty = true;
}

void RenderPass::setLineWidth(float width)
{
  data.lineWidth = width;
  data.dirty = true;
}

void RenderPass::setCullMode(CullMode mode)
{
  data.cullMode = mode;
  data.dirty = true;
}

void RenderPass::setCombineMode(GLenum mode)
{
  data.combineMode = mode;
  data.dirty = true;
}

void RenderPass::setPolygonMode(GLenum mode)
{
  data.polygonMode = mode;
  data.dirty = true;
}

void RenderPass::setShadeMode(GLenum mode)
{
  data.shadeMode = mode;
  data.dirty = true;
}

void RenderPass::setBlendFactors(GLenum src, GLenum dst)
{
  data.srcFactor = src;
  data.dstFactor = dst;
  data.dirty = true;
}

void RenderPass::setDepthFunction(GLenum function)
{
  data.depthFunction = function;
  data.dirty = true;
}

void RenderPass::setAlphaFunction(GLenum function)
{
  data.alphaFunction = function;
  data.dirty = true;
}

void RenderPass::setStencilFunction(GLenum function)
{
  data.stencilFunction = function;
  data.dirty = true;
}

void RenderPass::setStencilValues(unsigned int reference, unsigned int mask)
{
  data.stencilRef = reference;
  data.stencilMask = mask;
  data.dirty = true;
}

void RenderPass::setStencilOperations(GLenum stencilFailed,
                                      GLenum depthFailed,
                                      GLenum depthPassed)
{
  data.stencilFailed = stencilFailed;
  data.depthFailed = depthFailed;
  data.depthPassed = depthPassed;
  data.dirty = true;
}

void RenderPass::setShininess(float newValue)
{
  data.shininess = newValue;
  data.dirty = true;
}

void RenderPass::setDefaultColor(const ColorRGBA& color)
{
  data.defaultColor = color;
  data.dirty = true;
}

void RenderPass::setAmbientColor(const ColorRGBA& color)
{
  data.ambientColor = color;
  data.dirty = true;
}

void RenderPass::setDiffuseColor(const ColorRGBA& color)
{
  data.diffuseColor = color;
  data.dirty = true;
}

void RenderPass::setSpecularColor(const ColorRGBA& color)
{
  data.specularColor = color;
  data.dirty = true;
}

void RenderPass::setCombineColor(const ColorRGBA& color)
{
  data.combineColor = color;
  data.dirty = true;
}

void RenderPass::setTextureName(const String& name)
{
  data.textureName = name;
  data.dirty = true;
}

void RenderPass::setVertexProgramName(const String& name)
{
  data.vertexProgramName = name;
  data.dirty = true;
}

void RenderPass::setFragmentProgramName(const String& name)
{
  data.fragmentProgramName = name;
  data.dirty = true;
}

void RenderPass::setDefaults(void)
{
  data.setDefaults();
}

void RenderPass::invalidateCache(void)
{
  cache.dirty = true;
}

void RenderPass::setBooleanState(GLenum state, bool value) const
{
  if (value)
    glEnable(state);
  else
    glDisable(state);
}

RenderPass::Data RenderPass::cache;

///////////////////////////////////////////////////////////////////////

RenderPass::Data::Data(void)
{
  setDefaults();
}

void RenderPass::Data::setDefaults(void)
{
  dirty = true;
  lighting = false;
  depthTesting = true;
  depthWriting = true;
  stencilTesting = false;
  sphereMapped = false;
  lineWidth = 1.f;
  cullMode = CULL_BACK;
  combineMode = GL_MODULATE;
  polygonMode = GL_FILL;
  shadeMode = GL_SMOOTH;
  srcFactor = GL_ONE;
  dstFactor = GL_ZERO;
  depthFunction = GL_LESS;
  alphaFunction = GL_ALWAYS;
  stencilFunction = GL_ALWAYS;
  stencilRef = 0;
  stencilMask = ~0;
  stencilFailed = GL_KEEP;
  depthFailed = GL_KEEP;
  depthPassed = GL_KEEP;
  shininess = 0.f;
  defaultColor.set(1.f, 1.f, 1.f, 1.f);
  ambientColor.set(0.f, 0.f, 0.f, 1.f);
  diffuseColor.set(1.f, 1.f, 1.f, 1.f);
  specularColor.set(1.f, 1.f, 1.f, 1.f);
  combineColor.set(1.f, 1.f, 1.f, 1.f);
  textureName.clear();
  vertexProgramName.clear();
  fragmentProgramName.clear();
  textureTarget = 0;
}

///////////////////////////////////////////////////////////////////////

RenderStyle::RenderStyle(const String& name):
  Managed<RenderStyle>(name)
{
}

RenderPass& RenderStyle::createPass(void)
{
  passes.push_back(RenderPass());
  return passes.back();
}

void RenderStyle::destroyPasses(void)
{
  passes.clear();
}

void RenderStyle::applyPass(unsigned int index) const
{
  getPass(index).apply();
}

bool RenderStyle::operator < (const RenderStyle& other) const
{
  // Styles with blending always go last

  if (!isBlending() && other.isBlending())
    return true;
 
  return getName().compare(other.getName()) < 0;
}

bool RenderStyle::isBlending(void) const
{
  for (PassList::const_iterator i = passes.begin();  i != passes.end();  i++)
    if ((*i).isBlending())
      return true;

  return false;
}

RenderPass& RenderStyle::getPass(unsigned int index)
{
  PassList::iterator pass = passes.begin();
  std::advance(pass, index);
  return *pass;
}

const RenderPass& RenderStyle::getPass(unsigned int index) const
{
  PassList::const_iterator pass = passes.begin();
  std::advance(pass, index);
  return *pass;
}

unsigned int RenderStyle::getPassCount(void) const
{
  return (unsigned int) passes.size();
}

///////////////////////////////////////////////////////////////////////

RenderOperation::RenderOperation(void):
  vertexBuffer(NULL),
  indexBuffer(NULL),
  style(NULL),
  start(0),
  count(0)
{
}

bool RenderOperation::operator < (const RenderOperation& other) const
{
  return (*style) < (*other.style);
}

///////////////////////////////////////////////////////////////////////

RenderQueue::RenderQueue(void):
  sorted(true)
{
}

void RenderQueue::addLight(Light& light)
{
  lights.push_back(&light);
}

void RenderQueue::addOperation(RenderOperation& operation)
{
  operations.push_back(operation);
  sorted = false;
}

void RenderQueue::removeOperations(void)
{
  operations.clear();
}

void RenderQueue::renderOperations(void)
{
  sortOperations();

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(true);

  for (OperationList::const_iterator i = operations.begin();  i != operations.end();  i++)
  {
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf((*i).transform);
    glPopAttrib();

    (*i).vertexBuffer->apply();
    if ((*i).indexBuffer)
      (*i).indexBuffer->apply();

    for (unsigned int pass = 0;  pass < (*i).style->getPassCount();  pass++)
    {
      (*i).style->applyPass(pass);

      if ((*i).indexBuffer)
        (*i).indexBuffer->render((*i).renderMode, (*i).start, (*i).count);
      else
        (*i).vertexBuffer->render((*i).renderMode, (*i).start, (*i).count);
    }

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
  }

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(false);
}

const RenderQueue::LightList& RenderQueue::getLights(void) const
{
  return lights;
}

const RenderQueue::OperationList& RenderQueue::getOperations(void) const
{
  return operations;
}

void RenderQueue::sortOperations(void)
{
  if (!sorted)
  {
    std::sort(operations.begin(), operations.end());
    sorted = true;
  }
}

///////////////////////////////////////////////////////////////////////

bool Renderer::allocateIndices(IndexBufferRange& range,
		               unsigned int count,
                               IndexBuffer::Type type)
{
  IndexBufferSlot* slot = NULL;

  for (IndexBufferList::iterator i = indexBuffers.begin();  i != indexBuffers.end();  i++)
  {
    if ((*i).indexBuffer->getType() == type && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    indexBuffers.push_back(IndexBufferSlot());
    slot = &(indexBuffers.back());

    slot->indexBuffer = IndexBuffer::createInstance(max(1024, count),
                                                    type,
						    IndexBuffer::DYNAMIC);
    if (!slot->indexBuffer)
    {
      indexBuffers.pop_back();
      return false;
    }

    slot->available = slot->indexBuffer->getCount();
  }

  range = IndexBufferRange(*(slot->indexBuffer), 
			   slot->indexBuffer->getCount() - slot->available,
                           count);

  slot->available -= count;
  return true;
}

bool Renderer::allocateVertices(VertexBufferRange& range,
				unsigned int count,
				const VertexFormat& format)
{
  VertexBufferSlot* slot = NULL;

  for (VertexBufferList::iterator i = vertexBuffers.begin();  i != vertexBuffers.end();  i++)
  {
    if ((*i).vertexBuffer->getFormat() == format && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    vertexBuffers.push_back(VertexBufferSlot());
    slot = &(vertexBuffers.back());

    slot->vertexBuffer = VertexBuffer::createInstance(max(1024, count),
                                                      format,
						      VertexBuffer::DYNAMIC);
    if (!slot->vertexBuffer)
    {
      vertexBuffers.pop_back();
      return false;
    }

    slot->available = slot->vertexBuffer->getCount();
  }

  range = VertexBufferRange(*(slot->vertexBuffer), 
			    slot->vertexBuffer->getCount() - slot->available,
                            count);

  slot->available -= count;
  return true;
}

bool Renderer::create(void)
{
  Ptr<Renderer> renderer = new Renderer();
  if (!renderer->init())
    return false;

  set(renderer.detachObject());
  return true;
}

Renderer::Renderer(void)
{
}

bool Renderer::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create renderer without OpenGL context");
    return false;
  }

  Context::get()->getFinishSignal().connect(*this, &Renderer::onFinish);
  Context::get()->getDestroySignal().connect(onContextDestroy);

  return true;
}

void Renderer::onFinish(void)
{
  for (IndexBufferList::iterator i = indexBuffers.begin();  i != indexBuffers.end();  i++)
    (*i).available = (*i).indexBuffer->getCount();

  for (VertexBufferList::iterator i = vertexBuffers.begin();  i != vertexBuffers.end();  i++)
    (*i).available = (*i).vertexBuffer->getCount();
}

///////////////////////////////////////////////////////////////////////

/*
void Renderer::begin(void)
{
  stack.push(Context());

  Context& context = getContext();
  context.strokePass.setPolygonMode(GL_LINE);
}

void Renderer::end(void)
{
  if (stack.empty())
    throw Exception("Renderer context stack is empty");

  stack.pop();
}

void Renderer::drawLine(const Vector2& start, const Vector2& end) const
{
  Context& context = getContext();

  if (context.stroking)
  {
    context.strokePass.apply();

    glBegin(GL_LINES);
    glVertex2fv(start);
    glVertex2fv(end);
    glEnd();
  }
}

void Renderer::drawCircle(const Vector2& center, float radius) const
{
}

void Renderer::drawBezier(const BezierCurve2& curve) const
{
  BezierCurve2::PointList points;
  curve.tesselate(points, 0.5f);

  if (context.stroking)
  {
    context.strokePass.apply();

    glBegin(GL_LINE_STRIP);
    for (BezierCurve2::PointList::const_iterator p = points.begin();  p != points.end();  p++)
      glVertex2fv(*p);
    glEnd();
  }
}

void Renderer::drawRectangle(const Rectangle& rectangle) const
{
  Context& context = getContext();
  
  if (context.filling)
  {
    context.fillPass.apply();
    glRectf(rectangle.position.x,
            rectangle.position.y,
	    rectangle.position.x + rectangle.size.x,
	    rectangle.position.y + rectangle.size.y);
  }

  if (context.stroking)
  {
    context.strokePass.apply();
    glRectf(rectangle.position.x,
            rectangle.position.y,
	    rectangle.position.x + rectangle.size.x,
	    rectangle.position.y + rectangle.size.y);
  }
}

bool Renderer::isStroking(void) const
{
  return getContext().stroking;
}

void Renderer::setStroking(bool newState)
{
  Context& context = getContext();
  context.stroking = newState;
}

bool Renderer::isFilling(void) const
{
  return getContext().filling;
}

void Renderer::setFilling(bool newState)
{
  Context& context = getContext();
  context.filling = newState;
}

const ColorRGBA& Renderer::getStrokeColor(void) const
{
  return getContext().strokePass.getDefaultColor();
}

void Renderer::setStrokeColor(const ColorRGBA& newColor)
{
  Context& context = getContext();
  context.strokePass.setDefaultColor(newColor);
}

const ColorRGBA& Renderer::getFillColor(void) const
{
  return getContext().fillPass.getDefaultColor();
}

void Renderer::setFillColor(const ColorRGBA& newColor)
{
  Context& context = getContext();
  context.fillPass.setDefaultColor(newColor);
}

float Renderer::getStrokeWidth(void) const
{
  return getContext().strokePass.getLineWidth() / getLineScale();
}

void Renderer::setStrokeWidth(float newWidth)
{
  Context& context = getContext();
  context.strokePass.setLineWidth(newWidth() * getLineScale());
}

bool Renderer::create(void)
{
  Ptr<Renderer> renderer = new Renderer();
  if (!renderer->init())
    return false;

  set(renderer.detachObject());
  return true;
}

Renderer::Renderer(void)
{
}

bool Renderer::init(void)
{
  return true;
}

float Renderer::getLineScale(void) const
{
  Canvas* Canvas::getCurrent();

  return (float) canvas->getPhysicalHeight() / (float) canvas->getHeight();
}

RenderPass& Renderer::getContext(void)
{
  if (stack.empty())
    throw Exception("Renderer context stack empty");

  return stack.top();
}

const RenderPass& Renderer::getContext(void) const
{
  if (stack.empty())
    throw Exception("Renderer context stack empty");

  return stack.top();
}

///////////////////////////////////////////////////////////////////////

Renderer::Context::Context(void):
  stroking(true),
  filling(false)
{
}
*/

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

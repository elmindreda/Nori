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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Resource.h>
#include <moira/Image.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLShader.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

ShaderPass::ShaderPass(void)
{
  setDefaults();
}

void ShaderPass::apply(void) const
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
      Log::writeWarning("Error when forcing shader pass: %s", gluErrorString(error));
    
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
    Log::writeWarning("Error when applying shader pass: %s", gluErrorString(error));
  
  data.dirty = cache.dirty = false;
}

bool ShaderPass::isDirty(void) const
{
  return data.dirty;
}

bool ShaderPass::isCulling(void) const
{
  return data.cullMode != CULL_NONE;
}

bool ShaderPass::isBlending(void) const
{
  return data.srcFactor != GL_ONE || data.dstFactor != GL_ZERO;
}

bool ShaderPass::isDepthTesting(void) const
{
  return data.depthTesting;
}

bool ShaderPass::isDepthWriting(void) const
{
  return data.depthWriting;
}

bool ShaderPass::isStencilTesting(void) const
{
  return data.stencilTesting;
}

bool ShaderPass::isSphereMapped(void) const
{
  return data.sphereMapped;
}

bool ShaderPass::isLit(void) const
{
  return data.lighting;
}

float ShaderPass::getLineWidth(void) const
{
  return data.lineWidth;
}

CullMode ShaderPass::getCullMode(void) const
{
  return data.cullMode;
}

GLenum ShaderPass::getCombineMode(void) const
{
  return data.combineMode;
}

GLenum ShaderPass::getPolygonMode(void) const
{
  return data.polygonMode;
}

GLenum ShaderPass::getShadeMode(void) const
{
  return data.shadeMode;
}

GLenum ShaderPass::getSrcFactor(void) const
{
  return data.srcFactor;
}

GLenum ShaderPass::getDstFactor(void) const
{
  return data.dstFactor;
}

GLenum ShaderPass::getDepthFunction(void) const
{
  return data.depthFunction;
}

GLenum ShaderPass::getAlphaFunction(void) const
{
  return data.alphaFunction;
}

GLenum ShaderPass::getStencilFunction(void) const
{
  return data.stencilFunction;
}

GLenum ShaderPass::getStencilFailOperation(void) const
{
  return data.stencilFailed;
}

GLenum ShaderPass::getDepthFailOperation(void) const
{
  return data.depthFailed;
}

GLenum ShaderPass::getDepthPassOperation(void) const
{
  return data.depthPassed;
}

unsigned int ShaderPass::getStencilReference(void) const
{
  return data.stencilRef;
}

unsigned int ShaderPass::getStencilMask(void) const
{
  return data.stencilMask;
}

float ShaderPass::getShininess(void) const
{
  return data.shininess;
}

const ColorRGBA& ShaderPass::getDefaultColor(void) const
{
  return data.defaultColor;
}

const ColorRGBA& ShaderPass::getAmbientColor(void) const
{
  return data.ambientColor;
}

const ColorRGBA& ShaderPass::getDiffuseColor(void) const
{
  return data.diffuseColor;
}

const ColorRGBA& ShaderPass::getSpecularColor(void) const
{
  return data.specularColor;
}

const ColorRGBA& ShaderPass::getCombineColor(void) const
{
  return data.combineColor;
}

const String& ShaderPass::getTextureName(void) const
{
  return data.textureName;
}

const String& ShaderPass::getVertexProgramName(void) const
{
  return data.vertexProgramName;
}

const String& ShaderPass::getFragmentProgramName(void) const
{
  return data.fragmentProgramName;
}

void ShaderPass::setSphereMapped(bool enabled)
{
  data.sphereMapped = enabled;
  data.dirty = true;
}

void ShaderPass::setLit(bool enable)
{
  data.lighting = enable;
  data.dirty = true;
}

void ShaderPass::setDepthTesting(bool enable)
{
  data.depthTesting = enable;
  data.dirty = true;
}

void ShaderPass::setDepthWriting(bool enable)
{
  data.depthWriting = enable;
  data.dirty = true;
}

void ShaderPass::setStencilTesting(bool enable)
{
  data.stencilTesting = enable;
  data.dirty = true;
}

void ShaderPass::setLineWidth(float width)
{
  data.lineWidth = width;
  data.dirty = true;
}

void ShaderPass::setCullMode(CullMode mode)
{
  data.cullMode = mode;
  data.dirty = true;
}

void ShaderPass::setCombineMode(GLenum mode)
{
  data.combineMode = mode;
  data.dirty = true;
}

void ShaderPass::setPolygonMode(GLenum mode)
{
  data.polygonMode = mode;
  data.dirty = true;
}

void ShaderPass::setShadeMode(GLenum mode)
{
  data.shadeMode = mode;
  data.dirty = true;
}

void ShaderPass::setBlendFactors(GLenum src, GLenum dst)
{
  data.srcFactor = src;
  data.dstFactor = dst;
  data.dirty = true;
}

void ShaderPass::setDepthFunction(GLenum function)
{
  data.depthFunction = function;
  data.dirty = true;
}

void ShaderPass::setAlphaFunction(GLenum function)
{
  data.alphaFunction = function;
  data.dirty = true;
}

void ShaderPass::setStencilFunction(GLenum function)
{
  data.stencilFunction = function;
  data.dirty = true;
}

void ShaderPass::setStencilValues(unsigned int reference, unsigned int mask)
{
  data.stencilRef = reference;
  data.stencilMask = mask;
  data.dirty = true;
}

void ShaderPass::setStencilOperations(GLenum stencilFailed,
                                      GLenum depthFailed,
                                      GLenum depthPassed)
{
  data.stencilFailed = stencilFailed;
  data.depthFailed = depthFailed;
  data.depthPassed = depthPassed;
  data.dirty = true;
}

void ShaderPass::setShininess(float newValue)
{
  data.shininess = newValue;
  data.dirty = true;
}

void ShaderPass::setDefaultColor(const ColorRGBA& color)
{
  data.defaultColor = color;
  data.dirty = true;
}

void ShaderPass::setAmbientColor(const ColorRGBA& color)
{
  data.ambientColor = color;
  data.dirty = true;
}

void ShaderPass::setDiffuseColor(const ColorRGBA& color)
{
  data.diffuseColor = color;
  data.dirty = true;
}

void ShaderPass::setSpecularColor(const ColorRGBA& color)
{
  data.specularColor = color;
  data.dirty = true;
}

void ShaderPass::setCombineColor(const ColorRGBA& color)
{
  data.combineColor = color;
  data.dirty = true;
}

void ShaderPass::setTextureName(const String& name)
{
  data.textureName = name;
  data.dirty = true;
}

void ShaderPass::setVertexProgramName(const String& name)
{
  data.vertexProgramName = name;
  data.dirty = true;
}

void ShaderPass::setFragmentProgramName(const String& name)
{
  data.fragmentProgramName = name;
  data.dirty = true;
}

void ShaderPass::setDefaults(void)
{
  data.setDefaults();
}

void ShaderPass::invalidateCache(void)
{
  cache.dirty = true;
}

void ShaderPass::setBooleanState(GLenum state, bool value) const
{
  if (value)
    glEnable(state);
  else
    glDisable(state);
}

ShaderPass::Data ShaderPass::cache;

///////////////////////////////////////////////////////////////////////

ShaderPass::Data::Data(void)
{
  setDefaults();
}

void ShaderPass::Data::setDefaults(void)
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

Shader::Shader(const String& name):
  Managed<Shader>(name)
{
}

ShaderPass& Shader::createPass(void)
{
  passes.push_back(ShaderPass());
  return passes.back();
}

void Shader::destroyPasses(void)
{
  passes.clear();
}

void Shader::applyPass(unsigned int index) const
{
  getPass(index).apply();
}

bool Shader::operator < (const Shader& other) const
{
  // Shaders with blending always go last

  if (!isBlending() && other.isBlending())
    return true;
 
  return getName().compare(other.getName()) < 0;
}

bool Shader::isBlending(void) const
{
  for (PassList::const_iterator i = passes.begin();  i != passes.end();  i++)
    if ((*i).isBlending())
      return true;

  return false;
}

ShaderPass& Shader::getPass(unsigned int index)
{
  PassList::iterator pass = passes.begin();
  std::advance(pass, index);
  return *pass;
}

const ShaderPass& Shader::getPass(unsigned int index) const
{
  PassList::const_iterator pass = passes.begin();
  std::advance(pass, index);
  return *pass;
}

unsigned int Shader::getPassCount(void) const
{
  return (unsigned int) passes.size();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

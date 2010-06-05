///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLenum convertToGL(CullMode mode)
{
  switch (mode)
  {
    case CULL_FRONT:
      return GL_FRONT;
    case CULL_BACK:
      return GL_BACK;
    case CULL_BOTH:
      return GL_FRONT_AND_BACK;
  }

  Log::writeError("Invalid cull mode %u", mode);
  return 0;
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
  }

  Log::writeError("Invalid cull mode %u", mode);
  return (CullMode) 0;
}

GLenum convertToGL(BlendFactor factor)
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
  }

  Log::writeError("Invalid blend factor %u", factor);
  return 0;
}

GLenum convertToGL(Function function)
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
  }

  Log::writeError("Invalid comparison function %u", function);
  return 0;
}

GLenum convertToGL(Operation operation)
{
  switch (operation)
  {
    case OP_KEEP:
      return GL_KEEP;
    case OP_ZERO:
      return GL_ZERO;
    case OP_REPLACE:
      return GL_REPLACE;
    case OP_INCREASE:
      return GL_INCR;
    case OP_DECREASE:
      return GL_DECR;
    case OP_INVERT:
      return GL_INVERT;
    case OP_INCREASE_WRAP:
      return GL_INCR_WRAP;
    case OP_DECREASE_WRAP:
      return GL_DECR_WRAP;
  }

  Log::writeError("Invalid stencil operation %u", operation);
  return 0;
}

template <typename T>
class NameComparator
{
public:
  inline NameComparator(const String& name);
  inline bool operator () (const T& object);
  inline bool operator () (const T* object);
private:
  const String& name;
};

template <typename T>
inline NameComparator<T>::NameComparator(const String& initName):
  name(initName)
{
}

template <typename T>
inline bool NameComparator<T>::operator () (const T& object)
{
  return name == object.getName();
}

template <typename T>
inline bool NameComparator<T>::operator () (const T* object)
{
  return name == object->getName();
}

template <>
inline bool NameComparator<UniformState>::operator () (const UniformState* object)
{
  return name == object->getUniform().getName();
}

template <>
inline bool NameComparator<SamplerState>::operator () (const SamplerState* object)
{
  return name == object->getSampler().getName();
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

void StencilState::apply(void) const
{
  if (cache.dirty)
  {
    force();
    return;
  }

  if (data.enabled)
  {
    if (!cache.enabled)
    {
      glEnable(GL_STENCIL_TEST);
      cache.enabled = data.enabled;
    }

    if (data.function != cache.function ||
        data.reference != cache.reference ||
        data.writeMask != cache.writeMask)
    {
      glStencilFunc(convertToGL(data.function), data.reference, data.writeMask);

      cache.function = data.function;
      cache.reference = data.reference;
      cache.writeMask = data.writeMask;
    }

    if (data.stencilFailed != cache.stencilFailed ||
        data.depthFailed != cache.depthFailed ||
        data.depthPassed != cache.depthPassed)
    {
      glStencilOp(convertToGL(data.stencilFailed),
                  convertToGL(data.depthFailed),
		  convertToGL(data.depthPassed));

      cache.stencilFailed = data.stencilFailed;
      cache.depthFailed = data.depthFailed;
      cache.depthPassed = data.depthPassed;
    }
  }
  else
  {
    if (cache.enabled)
    {
      glDisable(GL_STENCIL_TEST);
      cache.enabled = data.enabled;
    }
  }

#if WENDY_DEBUG
  checkGL("Error when applying stencil state");
#endif

  data.dirty = false;
}

bool StencilState::isEnabled(void) const
{
  return data.enabled;
}

Function StencilState::getFunction(void) const
{
  return data.function;
}

Operation StencilState::getStencilFailOperation(void) const
{
  return data.stencilFailed;
}

Operation StencilState::getDepthFailOperation(void) const
{
  return data.depthFailed;
}

Operation StencilState::getDepthPassOperation(void) const
{
  return data.depthPassed;
}

unsigned int StencilState::getReference(void) const
{
  return data.reference;
}

unsigned int StencilState::getWriteMask(void) const
{
  return data.writeMask;
}

void StencilState::setEnabled(bool newState)
{
  data.enabled = newState;
  data.dirty = true;
}

void StencilState::setFunction(Function newFunction)
{
  data.function = newFunction;
  data.dirty = true;
}

void StencilState::setReference(unsigned int newReference)
{
  data.reference = newReference;
  data.dirty = true;
}

void StencilState::setWriteMask(unsigned int newMask)
{
  data.writeMask = newMask;
  data.dirty = true;
}

void StencilState::setOperations(Operation stencilFailed,
                                 Operation depthFailed,
                                 Operation depthPassed)
{
  data.stencilFailed = stencilFailed;
  data.depthFailed = depthFailed;
  data.depthPassed = depthPassed;
  data.dirty = true;
}

void StencilState::setDefaults(void)
{
  data.setDefaults();
}

void StencilState::force(void) const
{
  cache = data;

  if (data.enabled)
    glEnable(GL_STENCIL_TEST);
  else
    glDisable(GL_STENCIL_TEST);

  glStencilFunc(data.function, data.reference, data.writeMask);
  glStencilOp(data.stencilFailed, data.depthFailed, data.depthPassed);

#if WENDY_DEBUG
  checkGL("Error when forcing stencil state");
#endif

  cache.dirty = data.dirty = false;
}

StencilState::Data StencilState::cache;

///////////////////////////////////////////////////////////////////////

StencilState::Data::Data(void)
{
  setDefaults();
}

void StencilState::Data::setDefaults(void)
{
  dirty = true;
  enabled = false;
  function = ALLOW_ALWAYS;
  reference = 0;
  writeMask = ~0;
  stencilFailed = OP_KEEP;
  depthFailed = OP_KEEP;
  depthPassed = OP_KEEP;
}

///////////////////////////////////////////////////////////////////////

UniformState::UniformState(Uniform& initUniform):
  uniform(initUniform)
{
  std::memset(data, 0, sizeof(data));
}

UniformState::UniformState(const UniformState& source):
  uniform(source.uniform)
{
  std::memcpy(data, source.data, sizeof(data));
}

void UniformState::getValue(float& result) const
{
  result = *data;
}

void UniformState::setValue(float newValue)
{
  *data = newValue;
}

void UniformState::getValue(Vec2& result) const
{
  result = *reinterpret_cast<const Vec2*>(data);
}

void UniformState::setValue(const Vec2& newValue)
{
  *reinterpret_cast<Vec2*>(data) = newValue;
}

void UniformState::getValue(Vec3& result) const
{
  result = *reinterpret_cast<const Vec3*>(data);
}

void UniformState::setValue(const Vec3& newValue)
{
  *reinterpret_cast<Vec3*>(data) = newValue;
}

void UniformState::getValue(Vec4& result) const
{
  result = *reinterpret_cast<const Vec4*>(data);
}

void UniformState::setValue(const Vec4& newValue)
{
  *reinterpret_cast<Vec4*>(data) = newValue;
}

void UniformState::getValue(ColorRGB& result) const
{
  result = *reinterpret_cast<const ColorRGB*>(data);
}

void UniformState::setValue(const ColorRGB& newValue)
{
  *reinterpret_cast<ColorRGB*>(data) = newValue;
}

void UniformState::getValue(ColorRGBA& result) const
{
  result = *reinterpret_cast<const ColorRGBA*>(data);
}

void UniformState::setValue(const ColorRGBA& newValue)
{
  *reinterpret_cast<ColorRGBA*>(data) = newValue;
}

void UniformState::getValue(Mat2& result) const
{
  result = *reinterpret_cast<const Mat2*>(data);
}

void UniformState::setValue(const Mat2& newValue)
{
  *reinterpret_cast<Mat2*>(data) = newValue;
}

void UniformState::getValue(Mat3& result) const
{
  result = *reinterpret_cast<const Mat3*>(data);
}

void UniformState::setValue(const Mat3& newValue)
{
  *reinterpret_cast<Mat3*>(data) = newValue;
}

void UniformState::getValue(Mat4& result) const
{
  result = *reinterpret_cast<const Mat4*>(data);
}

void UniformState::setValue(const Mat4& newValue)
{
  *reinterpret_cast<Mat4*>(data) = newValue;
}

Uniform& UniformState::getUniform(void) const
{
  return uniform;
}

void UniformState::apply(void) const
{
  switch (uniform.getType())
  {
    case Uniform::FLOAT:
      uniform.setValue(*data);
      break;
    case Uniform::FLOAT_VEC2:
      uniform.setValue(*reinterpret_cast<const Vec2*>(data));
      break;
    case Uniform::FLOAT_VEC3:
      uniform.setValue(*reinterpret_cast<const Vec3*>(data));
      break;
    case Uniform::FLOAT_VEC4:
      uniform.setValue(*reinterpret_cast<const Vec4*>(data));
      break;
    case Uniform::FLOAT_MAT2:
      uniform.setValue(*reinterpret_cast<const Mat2*>(data));
      break;
    case Uniform::FLOAT_MAT3:
      uniform.setValue(*reinterpret_cast<const Mat3*>(data));
      break;
    case Uniform::FLOAT_MAT4:
      uniform.setValue(*reinterpret_cast<const Mat4*>(data));
      break;
  }
}

UniformState& UniformState::operator = (const UniformState& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

SamplerState::SamplerState(Sampler& initSampler):
  sampler(initSampler)
{
}

SamplerState::SamplerState(const SamplerState& source):
  sampler(source.sampler)
{
  texture = source.texture;
}

void SamplerState::getTexture(Ref<Texture>& result) const
{
  result = texture;
}

void SamplerState::setTexture(Texture* newTexture)
{
  texture = newTexture;
}

Sampler& SamplerState::getSampler(void) const
{
  return sampler;
}

void SamplerState::apply(void) const
{
  if (texture)
    sampler.setTexture(*texture);
  else
  {
    // TODO: Wtf do we do here?
  }
}

SamplerState& SamplerState::operator = (const SamplerState& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

ProgramState::ProgramState(void)
{
}

ProgramState::ProgramState(const ProgramState& source)
{
  operator = (source);
}

ProgramState::~ProgramState(void)
{
  destroyProgramState();
}

void ProgramState::apply(void) const
{
  if (program)
  {
    for (UniformList::const_iterator i = uniforms.begin();  i != uniforms.end();  i++)
      (**i).apply();

    for (SamplerList::const_iterator i = samplers.begin();  i != samplers.end();  i++)
      (**i).apply();

    Context::get()->setCurrentProgram(program);
  }
}

ProgramState& ProgramState::operator = (const ProgramState& source)
{
  setProgram(source.program);

  for (SamplerList::const_iterator s = source.samplers.begin();  s != source.samplers.end();  s++)
    samplers.push_back(new SamplerState(**s));

  for (UniformList::const_iterator s = source.uniforms.begin();  s != source.uniforms.end();  s++)
    uniforms.push_back(new UniformState(**s));

  return *this;
}

unsigned int ProgramState::getUniformCount(void) const
{
  return uniforms.size();
}

UniformState& ProgramState::getUniformState(const String& name)
{
  UniformList::const_iterator i = std::find_if(uniforms.begin(), uniforms.end(), NameComparator<UniformState>(name));
  if (i == uniforms.end())
    throw Exception("Render pass uniform state unknown");

  return **i;
}

const UniformState& ProgramState::getUniformState(const String& name) const
{
  UniformList::const_iterator i = std::find_if(uniforms.begin(), uniforms.end(), NameComparator<UniformState>(name));
  if (i == uniforms.end())
    throw Exception("Render pass uniform state unknown");

  return **i;
}

UniformState& ProgramState::getUniformState(unsigned int index)
{
  if (index >= uniforms.size())
    throw Exception("Render pass uniform state access out of range");

  return *uniforms[index];
}

const UniformState& ProgramState::getUniformState(unsigned int index) const
{
  if (index >= uniforms.size())
    throw Exception("Render pass uniform state access out of range");

  return *uniforms[index];
}

unsigned int ProgramState::getSamplerCount(void) const
{
  return samplers.size();
}

SamplerState& ProgramState::getSamplerState(const String& name)
{
  SamplerList::const_iterator i = std::find_if(samplers.begin(), samplers.end(), NameComparator<SamplerState>(name));
  if (i == samplers.end())
    throw Exception("Render pass sampler state unknown");

  return **i;
}

const SamplerState& ProgramState::getSamplerState(const String& name) const
{
  SamplerList::const_iterator i = std::find_if(samplers.begin(), samplers.end(), NameComparator<SamplerState>(name));
  if (i == samplers.end())
    throw Exception("Render pass sampler state unknown");

  return **i;
}

SamplerState& ProgramState::getSamplerState(unsigned int index)
{
  if (index >= samplers.size())
    throw Exception("Render pass sampler state access out of range");

  return *samplers[index];
}

const SamplerState& ProgramState::getSamplerState(unsigned int index) const
{
  if (index >= samplers.size())
    throw Exception("Render pass sampler state access out of range");

  return *samplers[index];
}

Program* ProgramState::getProgram(void) const
{
  return program;
}

void ProgramState::setProgram(Program* newProgram)
{
  destroyProgramState();
  program = newProgram;

  if (program)
  {
    for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
    {
      Sampler& sampler = program->getSampler(i);

      if (Context::get()->isReservedSampler(sampler.getName()))
	continue;

      samplers.push_back(new SamplerState(sampler));
    }

    for (unsigned int i = 0;  i < program->getUniformCount();  i++)
    {
      Uniform& uniform = program->getUniform(i);

      if (Context::get()->isReservedUniform(uniform.getName()))
	continue;

      uniforms.push_back(new UniformState(uniform));
    }
  }
}

void ProgramState::setDefaults(void)
{
  setProgram(NULL);
}

void ProgramState::destroyProgramState(void)
{
  while (!uniforms.empty())
  {
    delete uniforms.back();
    uniforms.pop_back();
  }

  while (!samplers.empty())
  {
    delete samplers.back();
    samplers.pop_back();
  }
}

///////////////////////////////////////////////////////////////////////

void RenderState::apply(void) const
{
  if (Context* context = Context::get())
  {
    if (Stats* stats = context->getStats())
      stats->addPasses(1);
  }

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
      glCullFace(convertToGL(cullMode));

    cache.cullMode = cullMode;
  }

  if (data.srcFactor != cache.srcFactor || data.dstFactor != cache.dstFactor)
  {
    setBooleanState(GL_BLEND, data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO);

    if (data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO)
      glBlendFunc(convertToGL(data.srcFactor), convertToGL(data.dstFactor));

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
        glDepthFunc(convertToGL(data.depthFunction));
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
        glDepthFunc(convertToGL(depthFunction));
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

#if WENDY_DEBUG
  checkGL("Error when applying render state");
#endif

  ProgramState::apply();

  data.dirty = false;
}

bool RenderState::isCulling(void) const
{
  return data.cullMode != CULL_NONE;
}

bool RenderState::isBlending(void) const
{
  return data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO;
}

bool RenderState::isDepthTesting(void) const
{
  return data.depthTesting;
}

bool RenderState::isDepthWriting(void) const
{
  return data.depthWriting;
}

bool RenderState::isColorWriting(void) const
{
  return data.colorWriting;
}

bool RenderState::isWireframe(void) const
{
  return data.wireframe;
}

CullMode RenderState::getCullMode(void) const
{
  return data.cullMode;
}

BlendFactor RenderState::getSrcFactor(void) const
{
  return data.srcFactor;
}

BlendFactor RenderState::getDstFactor(void) const
{
  return data.dstFactor;
}

Function RenderState::getDepthFunction(void) const
{
  return data.depthFunction;
}

void RenderState::setDepthTesting(bool enable)
{
  data.depthTesting = enable;
  data.dirty = true;
}

void RenderState::setDepthWriting(bool enable)
{
  data.depthWriting = enable;
  data.dirty = true;
}

void RenderState::setCullMode(CullMode mode)
{
  data.cullMode = mode;
  data.dirty = true;
}

void RenderState::setBlendFactors(BlendFactor src, BlendFactor dst)
{
  data.srcFactor = src;
  data.dstFactor = dst;
  data.dirty = true;
}

void RenderState::setDepthFunction(Function function)
{
  data.depthFunction = function;
  data.dirty = true;
}

void RenderState::setColorWriting(bool enabled)
{
  data.colorWriting = enabled;
  data.dirty = true;
}

void RenderState::setWireframe(bool enabled)
{
  data.wireframe = enabled;
  data.dirty = true;
}

void RenderState::setDefaults(void)
{
  data.setDefaults();
}

bool RenderState::isCullingInverted(void)
{
  return cullingInverted;
}

void RenderState::setCullingInversion(bool newState)
{
  cullingInverted = newState;
}

void RenderState::force(void) const
{
  cache = data;

  CullMode cullMode = data.cullMode;
  if (cullingInverted)
    cullMode = invertCullMode(cullMode);

  setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);
  if (cullMode != CULL_NONE)
    glCullFace(convertToGL(cullMode));

  setBooleanState(GL_BLEND, data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO);
  glBlendFunc(convertToGL(data.srcFactor), convertToGL(data.dstFactor));

  glDepthMask(data.depthWriting ? GL_TRUE : GL_FALSE);
  setBooleanState(GL_DEPTH_TEST, data.depthTesting || data.depthWriting);

  if (data.depthWriting && !data.depthTesting)
  {
    const Function depthFunction = ALLOW_ALWAYS;
    glDepthFunc(convertToGL(depthFunction));
    cache.depthFunction = depthFunction;
  }
  else
    glDepthFunc(convertToGL(data.depthFunction));

  const GLboolean state = data.colorWriting ? GL_TRUE : GL_FALSE;
  glColorMask(state, state, state, state);

  const GLenum polygonMode = data.wireframe ? GL_LINE : GL_FILL;
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

#if WENDY_DEBUG
  checkGL("Error when forcing render state");
#endif

  ProgramState::apply();

  cache.dirty = data.dirty = false;
}

void RenderState::setBooleanState(unsigned int state, bool value) const
{
  if (value)
    glEnable(state);
  else
    glDisable(state);
}

RenderState::Data RenderState::cache;

bool RenderState::cullingInverted = false;

///////////////////////////////////////////////////////////////////////

RenderState::Data::Data(void)
{
  setDefaults();
}

void RenderState::Data::setDefaults(void)
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

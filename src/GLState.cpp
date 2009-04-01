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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

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

GLenum convertOperation(Operation operation)
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
    default:
      throw Exception("Invalid stencil operation");
  }
}

template <typename T>
class UniformStateTemplate : public UniformState
{
public:
  inline UniformStateTemplate(Uniform& uniform);
  inline bool getValue(T& result) const;
  inline void setValue(const T& newValue);
private:
  inline void apply(void) const;
  T value;
};

template <typename T>
inline UniformStateTemplate<T>::UniformStateTemplate(Uniform& uniform):
  UniformState(uniform)
{
}

template <typename T>
inline bool UniformStateTemplate<T>::getValue(T& result) const
{
  result = value;
  return true;
}

template <typename T>
inline void UniformStateTemplate<T>::setValue(const T& newValue)
{
  value = newValue;
}

template <typename T>
inline void UniformStateTemplate<T>::apply(void) const
{
  getUniform().setValue(value);
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

}

///////////////////////////////////////////////////////////////////////

void StencilState::apply(void) const
{
  // NOTE: Yes, I know this is huge.  You don't need to point it out.

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
      glStencilFunc(convertFunction(data.function), data.reference, data.writeMask);

      cache.function = data.function;
      cache.reference = data.reference;
      cache.writeMask = data.writeMask;
    }

    if (data.stencilFailed != cache.stencilFailed ||
        data.depthFailed != cache.depthFailed ||
        data.depthPassed != cache.depthPassed)
    {
      glStencilOp(convertOperation(data.stencilFailed),
                  convertOperation(data.depthFailed),
		  convertOperation(data.depthPassed));

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

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying stencil state: %s", gluErrorString(error));
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

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeWarning("Error when forcing stencil state: %s", gluErrorString(error));
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

bool UniformState::getValue(float& result) const
{
  return false;
}

void UniformState::setValue(float newValue)
{
}

bool UniformState::getValue(Vector2& result) const
{
  return false;
}

void UniformState::setValue(const Vector2& newValue)
{
}

bool UniformState::getValue(Vector3& result) const
{
  return false;
}

void UniformState::setValue(const Vector3& newValue)
{
}

bool UniformState::getValue(Vector4& result) const
{
  return false;
}

void UniformState::setValue(const Vector4& newValue)
{
}

bool UniformState::getValue(Matrix2& result) const
{
  return false;
}

void UniformState::setValue(const Matrix2& newValue)
{
}

bool UniformState::getValue(Matrix3& result) const
{
  return false;
}

void UniformState::setValue(const Matrix3& newValue)
{
}

bool UniformState::getValue(Matrix4& result) const
{
  return false;
}

void UniformState::setValue(const Matrix4& newValue)
{
}

Uniform& UniformState::getUniform(void) const
{
  return uniform;
}

UniformState::UniformState(Uniform& initUniform):
  uniform(initUniform)
{
}

UniformState::UniformState(const UniformState& source):
  uniform(source.uniform)
{
}

UniformState& UniformState::operator = (const UniformState& source)
{
  return *this;
}

void UniformState::apply(void) const
{
}

///////////////////////////////////////////////////////////////////////

bool SamplerState::getTexture(Ref<Texture>& result) const
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

SamplerState::SamplerState(Sampler& initSampler):
  sampler(initSampler)
{
}

void SamplerState::apply(void) const
{
  sampler.setTexture(texture);
}

SamplerState::SamplerState(const SamplerState& source):
  sampler(source.sampler)
{
}

SamplerState& SamplerState::operator = (const SamplerState& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

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

    GL::Renderer::get()->setProgram(program);
  }
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
      samplers.push_back(new SamplerState(program->getSampler(i)));

    for (unsigned int i = 0;  i < program->getUniformCount();  i++)
      uniforms.push_back(new UniformState(program->getUniform(i)));
  }
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

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

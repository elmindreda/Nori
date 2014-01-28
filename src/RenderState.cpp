///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/ID.hpp>

#include <wendy/Core.hpp>
#include <wendy/Primitive.hpp>

#include <wendy/RenderState.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

bool samplerTypeMatchesTextureType(SamplerType samplerType,
                                   TextureType textureType)
{
  return (int) samplerType == (int) textureType;
}

IDPool<StateID> stateIDs;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

UniformStateIndex::UniformStateIndex():
  index(0xffff),
  offset(0xffff)
{
}

UniformStateIndex::UniformStateIndex(uint16 initIndex, uint16 initOffset):
  index(initIndex),
  offset(initOffset)
{
}

///////////////////////////////////////////////////////////////////////

SamplerStateIndex::SamplerStateIndex():
  index(0xffff),
  unit(0xffff)
{
}

SamplerStateIndex::SamplerStateIndex(uint16 initIndex, uint16 initUnit):
  index(initIndex),
  unit(initUnit)
{
}

///////////////////////////////////////////////////////////////////////

ProgramState::ProgramState():
  m_ID(stateIDs.allocateID())
{
}

ProgramState::ProgramState(const ProgramState& source):
  m_ID(stateIDs.allocateID()),
  m_program(source.m_program),
  m_floats(source.m_floats),
  m_textures(source.m_textures)
{
}

ProgramState::~ProgramState()
{
  stateIDs.releaseID(m_ID);
}

void ProgramState::apply() const
{
  if (!m_program)
  {
    logError("Applying program state with no program set");
    return;
  }

  RenderContext& context = m_program->context();
  context.setCurrentProgram(m_program);

  SharedProgramState* state = context.currentSharedProgramState();

  uint textureIndex = 0, textureUnit = 0;

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    context.setActiveTextureUnit(textureUnit);

    Sampler& sampler = m_program->sampler(i);
    if (sampler.isShared())
    {
      if (state)
        state->updateTo(sampler);
      else
        logError("Program %s uses shared sampler %s without a current shared program state",
                 m_program->name().c_str(),
                 sampler.name().c_str());
    }
    else
    {
      context.setCurrentTexture(m_textures[textureIndex]);
      textureIndex++;
    }

    sampler.bind(textureUnit);
    textureUnit++;
  }

  size_t offset = 0;

  for (uint i = 0;  i < m_program->uniformCount();  i++)
  {
    Uniform& uniform = m_program->uniform(i);
    if (uniform.isShared())
    {
      if (state)
        state->updateTo(uniform);
      else
        logError("Program %s uses shared uniform %s without a current shared program state",
                 m_program->name().c_str(),
                 uniform.name().c_str());
    }
    else
    {
      uniform.copyFrom(&m_floats[0] + offset);
      offset += uniform.elementCount();
    }
  }
}

bool ProgramState::hasUniformState(const char* name) const
{
  if (!m_program)
    return false;

  Uniform* uniform = m_program->findUniform(name);
  if (!uniform)
    return false;

  return !uniform->isShared();
}

bool ProgramState::hasSamplerState(const char* name) const
{
  if (!m_program)
    return false;

  Sampler* sampler = m_program->findSampler(name);
  if (!sampler)
    return false;

  return !sampler->isShared();
}

Texture* ProgramState::samplerState(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return nullptr;
  }

  uint textureIndex = 0;

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    const Sampler& sampler = m_program->sampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.name() == name)
      return m_textures[textureIndex];

    textureIndex++;
  }

  logError("Program %s has no sampler named %s",
           m_program->name().c_str(),
           name);
  return nullptr;
}

Texture* ProgramState::samplerState(SamplerStateIndex index) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return nullptr;
  }

  return m_textures[index.unit];
}

void ProgramState::setSamplerState(const char* name, Texture* newTexture)
{
  if (!m_program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  uint textureIndex = 0;

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    Sampler& sampler = m_program->sampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.name() == name)
    {
      if (newTexture)
      {
        if (samplerTypeMatchesTextureType(sampler.type(), newTexture->type()))
          m_textures[textureIndex] = newTexture;
        else
          logError("Type mismatch between sampler %s and texture %s",
                   sampler.name().c_str(),
                   newTexture->name().c_str());
      }
      else
        m_textures[textureIndex] = nullptr;

      return;
    }

    textureIndex++;
  }
}

void ProgramState::setSamplerState(SamplerStateIndex index, Texture* newTexture)
{
  if (!m_program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  Sampler& sampler = m_program->sampler(index.index);

  if (newTexture)
  {
    if (samplerTypeMatchesTextureType(sampler.type(), newTexture->type()))
      m_textures[index.unit] = newTexture;
    else
      logError("Type mismatch between sampler %s and texture %s",
                sampler.name().c_str(),
                newTexture->name().c_str());
  }
  else
    m_textures[index.unit] = nullptr;
}

UniformStateIndex ProgramState::uniformStateIndex(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve uniform state indices with no program");
    return UniformStateIndex();
  }

  uint offset = 0;

  for (uint i = 0;  i < m_program->uniformCount();  i++)
  {
    Uniform& uniform = m_program->uniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.name() == name)
      return UniformStateIndex(i, offset);

    offset += uniform.elementCount();
  }

  return UniformStateIndex();
}

SamplerStateIndex ProgramState::samplerStateIndex(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state indices with no program");
    return SamplerStateIndex();
  }

  uint textureIndex = 0;

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    Sampler& sampler = m_program->sampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.name() == name)
      return SamplerStateIndex(i, textureIndex);

    textureIndex++;
  }

  return SamplerStateIndex();
}

void ProgramState::setProgram(Program* newProgram)
{
  m_floats.clear();
  m_textures.clear();

  m_program = newProgram;
  if (!m_program)
    return;

  uint floatCount = 0;
  uint textureCount = 0;

  for (uint i = 0;  i < m_program->uniformCount();  i++)
  {
    Uniform& uniform = m_program->uniform(i);
    if (!uniform.isShared())
      floatCount += uniform.elementCount();
  }

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    Sampler& sampler = m_program->sampler(i);
    if (!sampler.isShared())
      textureCount++;
  }

  m_floats.insert(m_floats.end(), floatCount, 0.f);
  m_textures.resize(textureCount);
}

void* ProgramState::data(const char* name, UniformType type)
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  uint offset = 0;

  for (uint i = 0;  i < m_program->uniformCount();  i++)
  {
    Uniform& uniform = m_program->uniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.name() == name)
    {
      if (uniform.type() == type)
        return &m_floats[0] + offset;

      logError("Uniform %s of program %s is not of type %s",
               uniform.name().c_str(),
               m_program->name().c_str(),
               Uniform::typeName(type));
      return nullptr;
    }

    offset += uniform.elementCount();
  }

  logError("Program %s has no uniform named %s",
           m_program->name().c_str(),
           name);
  return nullptr;
}

const void* ProgramState::data(const char* name, UniformType type) const
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  uint offset = 0;

  for (uint i = 0;  i < m_program->uniformCount();  i++)
  {
    Uniform& uniform = m_program->uniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.name() == name)
    {
      if (uniform.type() == type)
        return &m_floats[0] + offset;

      logError("Uniform %s of program %s is not of type %s",
               uniform.name().c_str(),
               m_program->name().c_str(),
               Uniform::typeName(type));
      return nullptr;
    }

    offset += uniform.elementCount();
  }

  logError("Program %s has no uniform named %s",
           m_program->name().c_str(),
           name);
  return nullptr;
}

void* ProgramState::data(UniformStateIndex index, UniformType type)
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  Uniform& uniform = m_program->uniform(index.index);

  if (uniform.type() != type)
  {
    logError("Uniform %u of program %s is not of type %s",
             index.index,
             m_program->name().c_str(),
             Uniform::typeName(type));
    return nullptr;
  }

  return &m_floats[0] + index.offset;
}

const void* ProgramState::data(UniformStateIndex index, UniformType type) const
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  Uniform& uniform = m_program->uniform(index.index);

  if (uniform.type() != type)
  {
    logError("Uniform %u of program %s is not of type %s",
             index.index,
             m_program->name().c_str(),
             Uniform::typeName(type));
    return nullptr;
  }

  return &m_floats[0] + index.offset;
}

template <>
UniformType ProgramState::uniformType<float>()
{
  return UNIFORM_FLOAT;
}

template <>
UniformType ProgramState::uniformType<vec2>()
{
  return UNIFORM_VEC2;
}

template <>
UniformType ProgramState::uniformType<vec3>()
{
  return UNIFORM_VEC3;
}

template <>
UniformType ProgramState::uniformType<vec4>()
{
  return UNIFORM_VEC4;
}

template <>
UniformType ProgramState::uniformType<mat2>()
{
  return UNIFORM_MAT2;
}

template <>
UniformType ProgramState::uniformType<mat3>()
{
  return UNIFORM_MAT3;
}

template <>
UniformType ProgramState::uniformType<mat4>()
{
  return UNIFORM_MAT4;
}

///////////////////////////////////////////////////////////////////////

void Pass::apply() const
{
  if (!program())
  {
    logError("Applying render state with no program set");
    return;
  }

  RenderContext& context = program()->context();
  context.setCurrentRenderState(m_data);

  ProgramState::apply();
}

bool Pass::isCulling() const
{
  return m_data.cullMode != CULL_NONE;
}

bool Pass::isBlending() const
{
  return m_data.srcFactor != BLEND_ONE || m_data.dstFactor != BLEND_ZERO;
}

bool Pass::isDepthTesting() const
{
  return m_data.depthTesting;
}

bool Pass::isDepthWriting() const
{
  return m_data.depthWriting;
}

bool Pass::isColorWriting() const
{
  return m_data.colorWriting;
}

bool Pass::isStencilTesting() const
{
  return m_data.stencilTesting;
}

bool Pass::isWireframe() const
{
  return m_data.wireframe;
}

bool Pass::isLineSmoothing() const
{
  return m_data.lineSmoothing;
}

bool Pass::isMultisampling() const
{
  return m_data.multisampling;
}

float Pass::lineWidth() const
{
  return m_data.lineWidth;
}

CullMode Pass::cullMode() const
{
  return m_data.cullMode;
}

BlendFactor Pass::srcFactor() const
{
  return m_data.srcFactor;
}

BlendFactor Pass::dstFactor() const
{
  return m_data.dstFactor;
}

Function Pass::depthFunction() const
{
  return m_data.depthFunction;
}

Function Pass::stencilFunction() const
{
  return m_data.stencilFunction;
}

StencilOp Pass::stencilFailOperation() const
{
  return m_data.stencilFailOp;
}

StencilOp Pass::depthFailOperation() const
{
  return m_data.depthFailOp;
}

StencilOp Pass::depthPassOperation() const
{
  return m_data.depthPassOp;
}

uint Pass::stencilReference() const
{
  return m_data.stencilRef;
}

uint Pass::stencilWriteMask() const
{
  return m_data.stencilMask;
}

void Pass::setDepthTesting(bool enable)
{
  m_data.depthTesting = enable;
}

void Pass::setDepthWriting(bool enable)
{
  m_data.depthWriting = enable;
}

void Pass::setStencilTesting(bool enable)
{
  m_data.stencilTesting = enable;
}

void Pass::setCullMode(CullMode mode)
{
  m_data.cullMode = mode;
}

void Pass::setBlendFactors(BlendFactor src, BlendFactor dst)
{
  m_data.srcFactor = src;
  m_data.dstFactor = dst;
}

void Pass::setDepthFunction(Function function)
{
  m_data.depthFunction = function;
}

void Pass::setStencilFunction(Function newFunction)
{
  m_data.stencilFunction = newFunction;
}

void Pass::setStencilReference(uint newReference)
{
  m_data.stencilRef = newReference;
}

void Pass::setStencilWriteMask(uint newMask)
{
  m_data.stencilMask = newMask;
}

void Pass::setStencilFailOperation(StencilOp newOperation)
{
  m_data.stencilFailOp = newOperation;
}

void Pass::setDepthFailOperation(StencilOp newOperation)
{
  m_data.depthFailOp = newOperation;
}

void Pass::setDepthPassOperation(StencilOp newOperation)
{
  m_data.depthPassOp = newOperation;
}

void Pass::setColorWriting(bool enabled)
{
  m_data.colorWriting = enabled;
}

void Pass::setWireframe(bool enabled)
{
  m_data.wireframe = enabled;
}

void Pass::setLineSmoothing(bool enabled)
{
  m_data.lineSmoothing = enabled;
}

void Pass::setMultisampling(bool enabled)
{
  m_data.multisampling = enabled;
}

void Pass::setLineWidth(float newWidth)
{
  m_data.lineWidth = newWidth;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

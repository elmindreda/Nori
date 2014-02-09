///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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
#include <wendy/Primitive.hpp>
#include <wendy/Pass.hpp>

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

IDPool<PassID> passIDs;

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

Pass::Pass():
  m_ID(passIDs.allocateID())
{
}

Pass::Pass(const Pass& source):
  m_ID(passIDs.allocateID()),
  m_program(source.m_program),
  m_floats(source.m_floats),
  m_textures(source.m_textures)
{
}

Pass::~Pass()
{
  passIDs.releaseID(m_ID);
}

void Pass::apply() const
{
  if (!m_program)
  {
    logError("Applying program state with no program set");
    return;
  }

  RenderContext& context = m_program->context();
  context.setCurrentProgram(m_program);
  context.setCurrentRenderState(m_state);

  SharedProgramState* state = context.currentSharedProgramState();

  uint textureIndex = 0, textureUnit = 0;

  for (auto& sampler : m_program->m_samplers)
  {
    context.setActiveTextureUnit(textureUnit);

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

    textureUnit++;
  }

  size_t offset = 0;

  for (auto& uniform : m_program->m_uniforms)
  {
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

bool Pass::isCulling() const
{
  return m_state.cullFace != FACE_NONE;
}

bool Pass::isBlending() const
{
  return m_state.srcFactor != BLEND_ONE || m_state.dstFactor != BLEND_ZERO;
}

bool Pass::isDepthTesting() const
{
  return m_state.depthTesting;
}

bool Pass::isDepthWriting() const
{
  return m_state.depthWriting;
}

bool Pass::isColorWriting() const
{
  return m_state.colorWriting;
}

bool Pass::isStencilTesting() const
{
  return m_state.stencilTesting;
}

bool Pass::isWireframe() const
{
  return m_state.wireframe;
}

bool Pass::isLineSmoothing() const
{
  return m_state.lineSmoothing;
}

bool Pass::isMultisampling() const
{
  return m_state.multisampling;
}

bool Pass::hasUniformState(const char* name) const
{
  if (!m_program)
    return false;

  Uniform* uniform = m_program->findUniform(name);
  if (!uniform)
    return false;

  return !uniform->isShared();
}

bool Pass::hasSamplerState(const char* name) const
{
  if (!m_program)
    return false;

  Sampler* sampler = m_program->findSampler(name);
  if (!sampler)
    return false;

  return !sampler->isShared();
}

float Pass::lineWidth() const
{
  return m_state.lineWidth;
}

PolygonFace Pass::cullFace() const
{
  return m_state.cullFace;
}

BlendFactor Pass::srcFactor() const
{
  return m_state.srcFactor;
}

BlendFactor Pass::dstFactor() const
{
  return m_state.dstFactor;
}

FragmentFunction Pass::depthFunction() const
{
  return m_state.depthFunction;
}

FragmentFunction Pass::stencilFunction(PolygonFace face) const
{
  assert(face == FACE_FRONT || face == FACE_BACK);
  return m_state.stencil[face].function;
}

StencilOp Pass::stencilFailOperation(PolygonFace face) const
{
  assert(face == FACE_FRONT || face == FACE_BACK);
  return m_state.stencil[face].stencilFailOp;
}

StencilOp Pass::depthFailOperation(PolygonFace face) const
{
  assert(face == FACE_FRONT || face == FACE_BACK);
  return m_state.stencil[face].depthFailOp;
}

StencilOp Pass::depthPassOperation(PolygonFace face) const
{
  assert(face == FACE_FRONT || face == FACE_BACK);
  return m_state.stencil[face].depthPassOp;
}

uint Pass::stencilReference(PolygonFace face) const
{
  assert(face == FACE_FRONT || face == FACE_BACK);
  return m_state.stencil[face].reference;
}

uint Pass::stencilWriteMask(PolygonFace face) const
{
  assert(face == FACE_FRONT || face == FACE_BACK);
  return m_state.stencil[face].mask;
}

void Pass::setDepthTesting(bool enable)
{
  m_state.depthTesting = enable;
}

void Pass::setDepthWriting(bool enable)
{
  m_state.depthWriting = enable;
}

void Pass::setStencilTesting(bool enable)
{
  m_state.stencilTesting = enable;
}

void Pass::setCullFace(PolygonFace face)
{
  m_state.cullFace = face;
}

void Pass::setBlendFactors(BlendFactor src, BlendFactor dst)
{
  m_state.srcFactor = src;
  m_state.dstFactor = dst;
}

void Pass::setDepthFunction(FragmentFunction function)
{
  m_state.depthFunction = function;
}

void Pass::setStencilFunction(PolygonFace face, FragmentFunction newFunction)
{
  if (face == FACE_BOTH)
  {
    m_state.stencil[0].function = newFunction;
    m_state.stencil[1].function = newFunction;
  }
  else
  {
    assert(face == FACE_FRONT || face == FACE_BACK);
    m_state.stencil[face].function = newFunction;
  }
}

void Pass::setStencilReference(PolygonFace face, uint newReference)
{
  if (face == FACE_BOTH)
  {
    m_state.stencil[0].reference = newReference;
    m_state.stencil[1].reference = newReference;
  }
  else
  {
    assert(face == FACE_FRONT || face == FACE_BACK);
    m_state.stencil[face].reference = newReference;
  }
}

void Pass::setStencilWriteMask(PolygonFace face, uint newMask)
{
  if (face == FACE_BOTH)
  {
    m_state.stencil[0].mask = newMask;
    m_state.stencil[1].mask = newMask;
  }
  else
  {
    assert(face == FACE_FRONT || face == FACE_BACK);
    m_state.stencil[face].mask = newMask;
  }
}

void Pass::setStencilFailOperation(PolygonFace face, StencilOp newOperation)
{
  if (face == FACE_BOTH)
  {
    m_state.stencil[0].stencilFailOp = newOperation;
    m_state.stencil[1].stencilFailOp = newOperation;
  }
  else
  {
    assert(face == FACE_FRONT || face == FACE_BACK);
    m_state.stencil[face].stencilFailOp = newOperation;
  }
}

void Pass::setDepthFailOperation(PolygonFace face, StencilOp newOperation)
{
  if (face == FACE_BOTH)
  {
    m_state.stencil[0].depthFailOp = newOperation;
    m_state.stencil[1].depthFailOp = newOperation;
  }
  else
  {
    assert(face == FACE_FRONT || face == FACE_BACK);
    m_state.stencil[face].depthFailOp = newOperation;
  }
}

void Pass::setDepthPassOperation(PolygonFace face, StencilOp newOperation)
{
  if (face == FACE_BOTH)
  {
    m_state.stencil[0].depthPassOp = newOperation;
    m_state.stencil[1].depthPassOp = newOperation;
  }
  else
  {
    assert(face == FACE_FRONT || face == FACE_BACK);
    m_state.stencil[face].depthPassOp = newOperation;
  }
}

void Pass::setColorWriting(bool enabled)
{
  m_state.colorWriting = enabled;
}

void Pass::setWireframe(bool enabled)
{
  m_state.wireframe = enabled;
}

void Pass::setLineSmoothing(bool enabled)
{
  m_state.lineSmoothing = enabled;
}

void Pass::setMultisampling(bool enabled)
{
  m_state.multisampling = enabled;
}

void Pass::setLineWidth(float newWidth)
{
  m_state.lineWidth = newWidth;
}

Texture* Pass::samplerState(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return nullptr;
  }

  uint textureIndex = 0;

  for (auto& sampler : m_program->m_samplers)
  {
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

Texture* Pass::samplerState(SamplerStateIndex index) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return nullptr;
  }

  return m_textures[index.unit];
}

void Pass::setSamplerState(const char* name, Texture* newTexture)
{
  if (!m_program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  uint textureIndex = 0;

  for (auto& sampler : m_program->m_samplers)
  {
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

void Pass::setSamplerState(SamplerStateIndex index, Texture* newTexture)
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

UniformStateIndex Pass::uniformStateIndex(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve uniform state indices with no program");
    return UniformStateIndex();
  }

  uint index = 0, offset = 0;

  for (auto& uniform : m_program->m_uniforms)
  {
    if (!uniform.isShared())
    {
      if (uniform.name() == name)
        return UniformStateIndex(index, offset);

      offset += uniform.elementCount();
    }

    index++;
  }

  return UniformStateIndex();
}

SamplerStateIndex Pass::samplerStateIndex(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state indices with no program");
    return SamplerStateIndex();
  }

  uint index = 0, textureIndex = 0;

  for (auto& sampler : m_program->m_samplers)
  {
    if (!sampler.isShared())
    {
      if (sampler.name() == name)
        return SamplerStateIndex(index, textureIndex);

      textureIndex++;
    }

    index++;
  }

  return SamplerStateIndex();
}

void Pass::setProgram(Program* newProgram)
{
  m_floats.clear();
  m_textures.clear();

  m_program = newProgram;
  if (!m_program)
    return;

  uint floatCount = 0;
  uint textureCount = 0;

  for (auto& uniform : m_program->m_uniforms)
  {
    if (!uniform.isShared())
      floatCount += uniform.elementCount();
  }

  for (auto& sampler : m_program->m_samplers)
  {
    if (!sampler.isShared())
      textureCount++;
  }

  m_floats.insert(m_floats.end(), floatCount, 0.f);
  m_textures.resize(textureCount);
}

void* Pass::data(const char* name, UniformType type)
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  uint offset = 0;

  for (auto& uniform : m_program->m_uniforms)
  {
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

const void* Pass::data(const char* name, UniformType type) const
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  uint offset = 0;

  for (auto& uniform : m_program->m_uniforms)
  {
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

void* Pass::data(UniformStateIndex index, UniformType type)
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

const void* Pass::data(UniformStateIndex index, UniformType type) const
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
UniformType Pass::uniformType<float>()
{
  return UNIFORM_FLOAT;
}

template <>
UniformType Pass::uniformType<vec2>()
{
  return UNIFORM_VEC2;
}

template <>
UniformType Pass::uniformType<vec3>()
{
  return UNIFORM_VEC3;
}

template <>
UniformType Pass::uniformType<vec4>()
{
  return UNIFORM_VEC4;
}

template <>
UniformType Pass::uniformType<mat2>()
{
  return UNIFORM_MAT2;
}

template <>
UniformType Pass::uniformType<mat3>()
{
  return UNIFORM_MAT3;
}

template <>
UniformType Pass::uniformType<mat4>()
{
  return UNIFORM_MAT4;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

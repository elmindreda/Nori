///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Core.hpp>
#include <nori/ID.hpp>
#include <nori/Primitive.hpp>
#include <nori/Pass.hpp>

namespace nori
{

namespace
{

const size_t uniformTypeSizes[] =
{
  sizeof(Texture*), sizeof(Texture*), sizeof(Texture*),
  sizeof(Texture*), sizeof(Texture*),
  sizeof(int), sizeof(uint), sizeof(float),
  sizeof(vec2), sizeof(vec3), sizeof(vec4),
  sizeof(mat2), sizeof(mat3), sizeof(mat4)
};

IDPool<PassID> passIDs;

} /*namespace*/

UniformStateIndex::UniformStateIndex():
  index(0xffff),
  offset(0xffff)
{
}

UniformStateIndex::UniformStateIndex(uint16 index, uint16 offset):
  index(index),
  offset(offset)
{
}

Pass::Pass():
  m_id(passIDs.allocateID())
{
}

Pass::Pass(const Pass& source):
  m_id(passIDs.allocateID())
{
  operator = (source);
}

Pass::~Pass()
{
  setProgram(nullptr);
  passIDs.releaseID(m_id);
}

void Pass::apply() const
{
  assert(m_program);

  RenderContext& context = m_program->context();
  context.setProgram(m_program);
  context.setRenderState(m_state);

  SharedProgramState* state = context.sharedProgramState();
  assert(state);

  uint textureUnit = 0;
  size_t offset = 0;

  for (Uniform& uniform : m_program->m_uniforms)
  {
    if (uniform.isSampler())
    {
      context.setTextureUnit(textureUnit);
      textureUnit++;

      if (uniform.isShared())
        state->updateTo(uniform);
      else
      {
        context.setTexture(*(Ref<Texture>*)(&m_uniformState[offset]));
        offset += uniformTypeSizes[uniform.type()];
      }
    }
    else
    {
      if (uniform.isShared())
        state->updateTo(uniform);
      else
      {
        uniform.copyFrom(&m_uniformState[offset]);
        offset += uniformTypeSizes[uniform.type()];
      }
    }
  }
}

Pass& Pass::operator = (const Pass& source)
{
  setProgram(source.m_program);
  m_state = source.m_state;

  if (m_program)
  {
    size_t offset = 0;

    for (const Uniform& uniform : m_program->m_uniforms)
    {
      if (uniform.isShared())
        continue;

      if (uniform.isSampler())
      {
        *(Ref<Texture>*)(&m_uniformState[offset]) =
          *(Ref<Texture>*)(&source.m_uniformState[offset]);
      }
      else
      {
        std::memcpy(&m_uniformState[offset],
                    &source.m_uniformState[offset],
                    uniformTypeSizes[uniform.type()]);
      }

      offset += uniformTypeSizes[uniform.type()];
    }
  }

  return *this;
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

bool Pass::hasUniformState(const char* name) const
{
  if (!m_program)
    return false;

  Uniform* uniform = m_program->findUniform(name);
  if (!uniform)
    return false;

  return !uniform->isShared();
}

Texture* Pass::uniformTexture(const char* name) const
{
  return uniformTexture(uniformStateIndex(name));
}

Texture* Pass::uniformTexture(UniformStateIndex index) const
{
  assert(m_program);
  return *(Ref<Texture>*)(&m_uniformState[index.offset]);
}

void Pass::setUniformTexture(const char* name, Texture* texture)
{
  setUniformTexture(uniformStateIndex(name), texture);
}

void Pass::setUniformTexture(UniformStateIndex index, Texture* texture)
{
  assert(m_program);
  assert(m_program->uniform(index.index).type() == UniformType(texture->type()));
  *(Ref<Texture>*)(&m_uniformState[index.offset]) = texture;
}

UniformStateIndex Pass::uniformStateIndex(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve uniform state indices with no program");
    return UniformStateIndex();
  }

  size_t index = 0, offset = 0;

  for (const Uniform& uniform : m_program->m_uniforms)
  {
    if (!uniform.isShared())
    {
      if (uniform.name() == name)
        return UniformStateIndex(uint16(index), uint16(offset));

      offset += uniformTypeSizes[uniform.type()];
    }

    index++;
  }

  return UniformStateIndex();
}

void Pass::setProgram(Program* program)
{
  if (m_program)
  {
    size_t offset = 0;

    for (const Uniform& uniform : m_program->m_uniforms)
    {
      if (uniform.isShared())
        continue;
      if (uniform.isSampler())
        *(Ref<Texture>*)(&m_uniformState[offset]) = nullptr;

      offset += uniformTypeSizes[uniform.type()];
    }
  }

  m_uniformState.clear();
  m_program = program;

  if (m_program)
  {
    size_t totalUniformSize = 0;

    for (const Uniform& uniform : m_program->m_uniforms)
    {
      if (uniform.isShared())
        continue;

      totalUniformSize += uniformTypeSizes[uniform.type()];
    }

    m_uniformState.insert(m_uniformState.end(), totalUniformSize, 0);
  }
}

void* Pass::data(UniformStateIndex index, UniformType type)
{
  assert(m_program);
  assert(m_program->uniform(index.index).type() == type);

  return &m_uniformState[index.offset];
}

const void* Pass::data(UniformStateIndex index, UniformType type) const
{
  assert(m_program);
  assert(m_program->uniform(index.index).type() == type);

  return &m_uniformState[index.offset];
}

template <>
UniformType Pass::uniformType<int>() { return UNIFORM_INT; }
template <>
UniformType Pass::uniformType<uint>() { return UNIFORM_UINT; }
template <>
UniformType Pass::uniformType<float>() { return UNIFORM_FLOAT; }
template <>
UniformType Pass::uniformType<vec2>() { return UNIFORM_VEC2; }
template <>
UniformType Pass::uniformType<vec3>() { return UNIFORM_VEC3; }
template <>
UniformType Pass::uniformType<vec4>() { return UNIFORM_VEC4; }
template <>
UniformType Pass::uniformType<mat2>() { return UNIFORM_MAT2; }
template <>
UniformType Pass::uniformType<mat3>() { return UNIFORM_MAT3; }
template <>
UniformType Pass::uniformType<mat4>() { return UNIFORM_MAT4; }

} /*namespace nori*/


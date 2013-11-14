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
#include <wendy/Primitive.hpp>

#include <wendy/RenderState.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

bool samplerTypeMatchesTextureType(GL::SamplerType samplerType,
                                   GL::TextureType textureType)
{
  return (int) samplerType == (int) textureType;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

SharedProgramState::SharedProgramState():
  m_dirtyModelView(true),
  m_dirtyViewProj(true),
  m_dirtyModelViewProj(true),
  m_dirtyInvModel(true),
  m_dirtyInvView(true),
  m_dirtyInvProj(true),
  m_dirtyInvModelView(true),
  m_dirtyInvViewProj(true),
  m_dirtyInvModelViewProj(true),
  m_cameraNearZ(0.f),
  m_cameraFarZ(0.f),
  m_cameraAspect(0.f),
  m_cameraFOV(0.f),
  m_viewportWidth(0.f),
  m_viewportHeight(0.f),
  m_time(0.f)
{
}

bool SharedProgramState::reserveSupported(GL::Context& context) const
{
  context.createSharedUniform("wyM", GL::UNIFORM_MAT4, SHARED_MODEL_MATRIX);
  context.createSharedUniform("wyV", GL::UNIFORM_MAT4, SHARED_VIEW_MATRIX);
  context.createSharedUniform("wyP", GL::UNIFORM_MAT4, SHARED_PROJECTION_MATRIX);
  context.createSharedUniform("wyMV", GL::UNIFORM_MAT4, SHARED_MODELVIEW_MATRIX);
  context.createSharedUniform("wyVP", GL::UNIFORM_MAT4, SHARED_VIEWPROJECTION_MATRIX);
  context.createSharedUniform("wyMVP", GL::UNIFORM_MAT4, SHARED_MODELVIEWPROJECTION_MATRIX);

  context.createSharedUniform("wyInvM", GL::UNIFORM_MAT4, SHARED_INVERSE_MODEL_MATRIX);
  context.createSharedUniform("wyInvV", GL::UNIFORM_MAT4, SHARED_INVERSE_VIEW_MATRIX);
  context.createSharedUniform("wyInvP", GL::UNIFORM_MAT4, SHARED_INVERSE_PROJECTION_MATRIX);
  context.createSharedUniform("wyInvMV", GL::UNIFORM_MAT4, SHARED_INVERSE_MODELVIEW_MATRIX);
  context.createSharedUniform("wyInvVP", GL::UNIFORM_MAT4, SHARED_INVERSE_VIEWPROJECTION_MATRIX);
  context.createSharedUniform("wyInvMVP", GL::UNIFORM_MAT4, SHARED_INVERSE_MODELVIEWPROJECTION_MATRIX);

  context.createSharedUniform("wyCameraNearZ", GL::UNIFORM_FLOAT, SHARED_CAMERA_NEAR_Z);
  context.createSharedUniform("wyCameraFarZ", GL::UNIFORM_FLOAT, SHARED_CAMERA_FAR_Z);
  context.createSharedUniform("wyCameraAspectRatio", GL::UNIFORM_FLOAT, SHARED_CAMERA_ASPECT_RATIO);
  context.createSharedUniform("wyCameraFOV", GL::UNIFORM_FLOAT, SHARED_CAMERA_FOV);
  context.createSharedUniform("wyCameraPosition", GL::UNIFORM_VEC3, SHARED_CAMERA_POSITION);

  context.createSharedUniform("wyViewportWidth", GL::UNIFORM_FLOAT, SHARED_VIEWPORT_WIDTH);
  context.createSharedUniform("wyViewportHeight", GL::UNIFORM_FLOAT, SHARED_VIEWPORT_HEIGHT);

  context.createSharedUniform("wyTime", GL::UNIFORM_FLOAT, SHARED_TIME);

  return true;
}

void SharedProgramState::cameraProperties(vec3& position,
                                          float& FOV,
                                          float& aspect,
                                          float& nearZ,
                                          float& farZ) const
{
  position = m_cameraPos;
  FOV = m_cameraFOV;
  aspect = m_cameraAspect;
  nearZ = m_cameraNearZ;
  farZ = m_cameraFarZ;
}

void SharedProgramState::setModelMatrix(const mat4& newMatrix)
{
  m_modelMatrix = newMatrix;
  m_dirtyModelView = m_dirtyModelViewProj = true;
  m_dirtyInvModel = m_dirtyInvModelView = m_dirtyInvModelViewProj = true;
}

void SharedProgramState::setViewMatrix(const mat4& newMatrix)
{
  m_viewMatrix = newMatrix;
  m_dirtyModelView = m_dirtyViewProj = m_dirtyModelViewProj = true;
  m_dirtyInvView = m_dirtyInvModelView = m_dirtyInvViewProj = m_dirtyInvModelViewProj = true;
}

void SharedProgramState::setProjectionMatrix(const mat4& newMatrix)
{
  m_projectionMatrix = newMatrix;
  m_dirtyViewProj = m_dirtyModelViewProj = true;
  m_dirtyInvProj = m_dirtyInvViewProj = m_dirtyInvModelViewProj = true;
}

void SharedProgramState::setOrthoProjectionMatrix(float width, float height)
{
  setProjectionMatrix(ortho(0.f, width, 0.f, height));
}

void SharedProgramState::setOrthoProjectionMatrix(const AABB& volume)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  volume.bounds(minX, minY, minZ, maxX, maxY, maxZ);

  setProjectionMatrix(ortho(minX, maxX, minY, maxY, minZ, maxZ));
}

void SharedProgramState::setPerspectiveProjectionMatrix(float FOV,
                                                        float aspect,
                                                        float nearZ,
                                                        float farZ)
{
  setProjectionMatrix(perspective(FOV, aspect, nearZ, farZ));
}

void SharedProgramState::setCameraProperties(const vec3& position,
                                             float FOV,
                                             float aspect,
                                             float nearZ,
                                             float farZ)
{
  m_cameraPos = position;
  m_cameraFOV = FOV;
  m_cameraAspect = aspect;
  m_cameraNearZ = nearZ;
  m_cameraFarZ = farZ;
}

void SharedProgramState::setViewportSize(float newWidth, float newHeight)
{
  m_viewportWidth = newWidth;
  m_viewportHeight = newHeight;
}

void SharedProgramState::setTime(float newTime)
{
  m_time = newTime;
}

void SharedProgramState::updateTo(GL::Sampler& sampler)
{
  logError("Unknown shared sampler uniform %s requested",
           sampler.name().c_str());
}

void SharedProgramState::updateTo(GL::Uniform& uniform)
{
  switch (uniform.sharedID())
  {
    case SHARED_MODEL_MATRIX:
    {
      uniform.copyFrom(value_ptr(m_modelMatrix));
      return;
    }

    case SHARED_VIEW_MATRIX:
    {
      uniform.copyFrom(value_ptr(m_viewMatrix));
      return;
    }

    case SHARED_PROJECTION_MATRIX:
    {
      uniform.copyFrom(value_ptr(m_projectionMatrix));
      return;
    }

    case SHARED_MODELVIEW_MATRIX:
    {
      if (m_dirtyModelView)
      {
        m_modelViewMatrix = m_viewMatrix;
        m_modelViewMatrix *= m_modelMatrix;
        m_dirtyModelView = false;
      }

      uniform.copyFrom(value_ptr(m_modelViewMatrix));
      return;
    }

    case SHARED_VIEWPROJECTION_MATRIX:
    {
      if (m_dirtyViewProj)
      {
        m_viewProjMatrix = m_projectionMatrix;
        m_viewProjMatrix *= m_viewMatrix;
        m_dirtyViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_viewProjMatrix));
      return;
    }

    case SHARED_MODELVIEWPROJECTION_MATRIX:
    {
      if (m_dirtyModelViewProj)
      {
        if (m_dirtyViewProj)
        {
          m_viewProjMatrix = m_projectionMatrix;
          m_viewProjMatrix *= m_viewMatrix;
          m_dirtyViewProj = false;
        }

        m_modelViewProjMatrix = m_viewProjMatrix;
        m_modelViewProjMatrix *= m_modelMatrix;
        m_dirtyModelViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_modelViewProjMatrix));
      return;
    }

    case SHARED_INVERSE_MODEL_MATRIX:
    {
      if (m_dirtyInvModel)
      {
        m_invModelMatrix = inverse(m_modelMatrix);
        m_dirtyInvModel = false;
      }

      uniform.copyFrom(value_ptr(m_invModelMatrix));
      return;
    }

    case SHARED_INVERSE_VIEW_MATRIX:
    {
      if (m_dirtyInvView)
      {
        m_invViewMatrix = inverse(m_viewMatrix);
        m_dirtyInvView = false;
      }

      uniform.copyFrom(value_ptr(m_invViewMatrix));
      return;
    }

    case SHARED_INVERSE_PROJECTION_MATRIX:
    {
      if (m_dirtyInvProj)
      {
        m_invProjMatrix = inverse(m_projectionMatrix);
        m_dirtyInvProj = false;
      }

      uniform.copyFrom(value_ptr(m_invProjMatrix));
      return;
    }

    case SHARED_INVERSE_MODELVIEW_MATRIX:
    {
      if (m_dirtyInvModelView)
      {
        if (m_dirtyModelView)
        {
          m_modelViewMatrix = m_viewMatrix;
          m_modelViewMatrix *= m_modelMatrix;
          m_dirtyModelView = false;
        }

        m_invModelViewMatrix = inverse(m_modelViewMatrix);
        m_dirtyInvModelView = false;
      }

      uniform.copyFrom(value_ptr(m_invModelViewMatrix));
      return;
    }

    case SHARED_INVERSE_VIEWPROJECTION_MATRIX:
    {
      if (m_dirtyInvViewProj)
      {
        if (m_dirtyViewProj)
        {
          m_viewProjMatrix = m_projectionMatrix;
          m_viewProjMatrix *= m_viewMatrix;
          m_dirtyViewProj = false;
        }

        m_invViewProjMatrix = inverse(m_viewProjMatrix);
        m_dirtyInvViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_invViewProjMatrix));
      return;
    }

    case SHARED_INVERSE_MODELVIEWPROJECTION_MATRIX:
    {
      if (m_dirtyInvModelViewProj)
      {
        if (m_dirtyModelViewProj)
        {
          if (m_dirtyViewProj)
          {
            m_viewProjMatrix = m_projectionMatrix;
            m_viewProjMatrix *= m_viewMatrix;
            m_dirtyViewProj = false;
          }

          m_modelViewProjMatrix = m_viewProjMatrix;
          m_modelViewProjMatrix *= m_modelMatrix;
          m_dirtyModelViewProj = false;
        }

        m_invModelViewProjMatrix = inverse(m_modelViewProjMatrix);
        m_dirtyInvModelViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_invModelViewProjMatrix));
      return;
    }

    case SHARED_CAMERA_POSITION:
    {
      uniform.copyFrom(value_ptr(m_cameraPos));
      return;
    }

    case SHARED_CAMERA_NEAR_Z:
    {
      uniform.copyFrom(&m_cameraNearZ);
      return;
    }

    case SHARED_CAMERA_FAR_Z:
    {
      uniform.copyFrom(&m_cameraFarZ);
      return;
    }

    case SHARED_CAMERA_ASPECT_RATIO:
    {
      uniform.copyFrom(&m_cameraAspect);
      return;
    }

    case SHARED_CAMERA_FOV:
    {
      uniform.copyFrom(&m_cameraFOV);
      return;
    }

    case SHARED_VIEWPORT_WIDTH:
    {
      uniform.copyFrom(&m_viewportWidth);
      return;
    }

    case SHARED_VIEWPORT_HEIGHT:
    {
      uniform.copyFrom(&m_viewportHeight);
      return;
    }

    case SHARED_TIME:
    {
      uniform.copyFrom(&m_time);
      return;
    }
  }

  logError("Unknown shared uniform %s requested",
           uniform.name().c_str());
}

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
  m_ID(allocateID())
{
}

ProgramState::ProgramState(const ProgramState& source):
  m_ID(allocateID()),
  m_program(source.m_program),
  m_floats(source.m_floats),
  m_textures(source.m_textures)
{
}

ProgramState::~ProgramState()
{
  releaseID(m_ID);
}

void ProgramState::apply() const
{
  if (!m_program)
  {
    logError("Applying program state with no program set");
    return;
  }

  GL::Context& context = m_program->context();
  context.setCurrentProgram(m_program);

  GL::SharedProgramState* state = context.currentSharedProgramState();

  uint textureIndex = 0, textureUnit = 0;

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    context.setActiveTextureUnit(textureUnit);

    GL::Sampler& sampler = m_program->sampler(i);
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
    GL::Uniform& uniform = m_program->uniform(i);
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

  GL::Uniform* uniform = m_program->findUniform(name);
  if (!uniform)
    return false;

  return !uniform->isShared();
}

bool ProgramState::hasSamplerState(const char* name) const
{
  if (!m_program)
    return false;

  GL::Sampler* sampler = m_program->findSampler(name);
  if (!sampler)
    return false;

  return !sampler->isShared();
}

GL::Texture* ProgramState::samplerState(const char* name) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return nullptr;
  }

  uint textureIndex = 0;

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    const GL::Sampler& sampler = m_program->sampler(i);
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

GL::Texture* ProgramState::samplerState(SamplerStateIndex index) const
{
  if (!m_program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return nullptr;
  }

  return m_textures[index.unit];
}

void ProgramState::setSamplerState(const char* name, GL::Texture* newTexture)
{
  if (!m_program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  uint textureIndex = 0;

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    GL::Sampler& sampler = m_program->sampler(i);
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

void ProgramState::setSamplerState(SamplerStateIndex index, GL::Texture* newTexture)
{
  if (!m_program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  GL::Sampler& sampler = m_program->sampler(index.index);

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
    GL::Uniform& uniform = m_program->uniform(i);
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
    GL::Sampler& sampler = m_program->sampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.name() == name)
      return SamplerStateIndex(i, textureIndex);

    textureIndex++;
  }

  return SamplerStateIndex();
}

void ProgramState::setProgram(GL::Program* newProgram)
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
    GL::Uniform& uniform = m_program->uniform(i);
    if (!uniform.isShared())
      floatCount += uniform.elementCount();
  }

  for (uint i = 0;  i < m_program->samplerCount();  i++)
  {
    GL::Sampler& sampler = m_program->sampler(i);
    if (!sampler.isShared())
      textureCount++;
  }

  m_floats.insert(m_floats.end(), floatCount, 0.f);
  m_textures.resize(textureCount);
}

StateID ProgramState::allocateID()
{
  if (m_usedIDs.empty())
    return m_nextID++;

  const StateID ID = m_usedIDs.back();
  m_usedIDs.pop_back();
  return ID;
}

void ProgramState::releaseID(StateID ID)
{
  m_usedIDs.push_front(ID);
}

void* ProgramState::data(const char* name, GL::UniformType type)
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  uint offset = 0;

  for (uint i = 0;  i < m_program->uniformCount();  i++)
  {
    GL::Uniform& uniform = m_program->uniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.name() == name)
    {
      if (uniform.type() == type)
        return &m_floats[0] + offset;

      logError("Uniform %s of program %s is not of type %s",
               uniform.name().c_str(),
               m_program->name().c_str(),
               GL::Uniform::typeName(type));
      return nullptr;
    }

    offset += uniform.elementCount();
  }

  logError("Program %s has no uniform named %s",
           m_program->name().c_str(),
           name);
  return nullptr;
}

const void* ProgramState::data(const char* name, GL::UniformType type) const
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  uint offset = 0;

  for (uint i = 0;  i < m_program->uniformCount();  i++)
  {
    GL::Uniform& uniform = m_program->uniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.name() == name)
    {
      if (uniform.type() == type)
        return &m_floats[0] + offset;

      logError("Uniform %s of program %s is not of type %s",
               uniform.name().c_str(),
               m_program->name().c_str(),
               GL::Uniform::typeName(type));
      return nullptr;
    }

    offset += uniform.elementCount();
  }

  logError("Program %s has no uniform named %s",
           m_program->name().c_str(),
           name);
  return nullptr;
}

void* ProgramState::data(UniformStateIndex index, GL::UniformType type)
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  GL::Uniform& uniform = m_program->uniform(index.index);

  if (uniform.type() != type)
  {
    logError("Uniform %u of program %s is not of type %s",
             index.index,
             m_program->name().c_str(),
             GL::Uniform::typeName(type));
    return nullptr;
  }

  return &m_floats[0] + index.offset;
}

const void* ProgramState::data(UniformStateIndex index, GL::UniformType type) const
{
  if (!m_program)
  {
    logError("Cannot set uniform state on program state with no program");
    return nullptr;
  }

  GL::Uniform& uniform = m_program->uniform(index.index);

  if (uniform.type() != type)
  {
    logError("Uniform %u of program %s is not of type %s",
             index.index,
             m_program->name().c_str(),
             GL::Uniform::typeName(type));
    return nullptr;
  }

  return &m_floats[0] + index.offset;
}

template <>
GL::UniformType ProgramState::uniformType<float>()
{
  return GL::UNIFORM_FLOAT;
}

template <>
GL::UniformType ProgramState::uniformType<vec2>()
{
  return GL::UNIFORM_VEC2;
}

template <>
GL::UniformType ProgramState::uniformType<vec3>()
{
  return GL::UNIFORM_VEC3;
}

template <>
GL::UniformType ProgramState::uniformType<vec4>()
{
  return GL::UNIFORM_VEC4;
}

template <>
GL::UniformType ProgramState::uniformType<mat2>()
{
  return GL::UNIFORM_MAT2;
}

template <>
GL::UniformType ProgramState::uniformType<mat3>()
{
  return GL::UNIFORM_MAT3;
}

template <>
GL::UniformType ProgramState::uniformType<mat4>()
{
  return GL::UNIFORM_MAT4;
}

ProgramState::IDQueue ProgramState::m_usedIDs;

StateID ProgramState::m_nextID = 0;

///////////////////////////////////////////////////////////////////////

void Pass::apply() const
{
  if (!program())
  {
    logError("Applying render state with no program set");
    return;
  }

  GL::Context& context = program()->context();
  context.setCurrentRenderState(m_data);

  ProgramState::apply();
}

bool Pass::isCulling() const
{
  return m_data.cullMode != GL::CULL_NONE;
}

bool Pass::isBlending() const
{
  return m_data.srcFactor != GL::BLEND_ONE || m_data.dstFactor != GL::BLEND_ZERO;
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

GL::CullMode Pass::cullMode() const
{
  return m_data.cullMode;
}

GL::BlendFactor Pass::srcFactor() const
{
  return m_data.srcFactor;
}

GL::BlendFactor Pass::dstFactor() const
{
  return m_data.dstFactor;
}

GL::Function Pass::depthFunction() const
{
  return m_data.depthFunction;
}

GL::Function Pass::stencilFunction() const
{
  return m_data.stencilFunction;
}

GL::StencilOp Pass::stencilFailOperation() const
{
  return m_data.stencilFailOp;
}

GL::StencilOp Pass::depthFailOperation() const
{
  return m_data.depthFailOp;
}

GL::StencilOp Pass::depthPassOperation() const
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

void Pass::setCullMode(GL::CullMode mode)
{
  m_data.cullMode = mode;
}

void Pass::setBlendFactors(GL::BlendFactor src, GL::BlendFactor dst)
{
  m_data.srcFactor = src;
  m_data.dstFactor = dst;
}

void Pass::setDepthFunction(GL::Function function)
{
  m_data.depthFunction = function;
}

void Pass::setStencilFunction(GL::Function newFunction)
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

void Pass::setStencilFailOperation(GL::StencilOp newOperation)
{
  m_data.stencilFailOp = newOperation;
}

void Pass::setDepthFailOperation(GL::StencilOp newOperation)
{
  m_data.depthFailOp = newOperation;
}

void Pass::setDepthPassOperation(GL::StencilOp newOperation)
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

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

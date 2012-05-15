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

#include <wendy/Config.h>

#include <wendy/RenderState.h>

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
  dirtyModelView(true),
  dirtyViewProj(true),
  dirtyModelViewProj(true),
  cameraNearZ(0.f),
  cameraFarZ(0.f),
  cameraAspect(0.f),
  cameraFOV(0.f),
  viewportWidth(0.f),
  viewportHeight(0.f),
  time(0.f)
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

const mat4& SharedProgramState::getModelMatrix() const
{
  return modelMatrix;
}

const mat4& SharedProgramState::getViewMatrix() const
{
  return viewMatrix;
}

const mat4& SharedProgramState::getProjectionMatrix() const
{
  return projectionMatrix;
}

void SharedProgramState::getCameraProperties(vec3& position,
                                             float& FOV,
                                             float& aspect,
                                             float& nearZ,
                                             float& farZ) const
{
  position = cameraPos;
  FOV = cameraFOV;
  aspect = cameraAspect;
  nearZ = cameraNearZ;
  farZ = cameraFarZ;
}

float SharedProgramState::getViewportWidth() const
{
  return viewportWidth;
}

float SharedProgramState::getViewportHeight() const
{
  return viewportHeight;
}

float SharedProgramState::getTime() const
{
  return time;
}

void SharedProgramState::setModelMatrix(const mat4& newMatrix)
{
  modelMatrix = newMatrix;
  dirtyModelView = dirtyModelViewProj = true;
}

void SharedProgramState::setViewMatrix(const mat4& newMatrix)
{
  viewMatrix = newMatrix;
  dirtyModelView = dirtyViewProj = dirtyModelViewProj = true;
}

void SharedProgramState::setProjectionMatrix(const mat4& newMatrix)
{
  projectionMatrix = newMatrix;
  dirtyViewProj = dirtyModelViewProj = true;
}

void SharedProgramState::setOrthoProjectionMatrix(float width, float height)
{
  projectionMatrix = ortho(0.f, width, 0.f, height);
  dirtyViewProj = dirtyModelViewProj = true;
}

void SharedProgramState::setOrthoProjectionMatrix(const AABB& volume)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  volume.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  projectionMatrix = ortho(minX, maxX, minY, maxY, minZ, maxZ);
  dirtyViewProj = dirtyModelViewProj = true;
}

void SharedProgramState::setPerspectiveProjectionMatrix(float FOV,
                                                        float aspect,
                                                        float nearZ,
                                                        float farZ)
{
  projectionMatrix = perspective(FOV, aspect, nearZ, farZ);
  dirtyViewProj = dirtyModelViewProj = true;
}

void SharedProgramState::setCameraProperties(const vec3& position,
                                             float FOV,
                                             float aspect,
                                             float nearZ,
                                             float farZ)
{
  cameraPos = position;
  cameraFOV = FOV;
  cameraAspect = aspect;
  cameraNearZ = nearZ;
  cameraFarZ = farZ;
}

void SharedProgramState::setViewportSize(float newWidth, float newHeight)
{
  viewportWidth = newWidth;
  viewportHeight = newHeight;
}

void SharedProgramState::setTime(float newTime)
{
  time = newTime;
}

void SharedProgramState::updateTo(GL::Sampler& sampler)
{
  logError("Unknown shared sampler uniform \'%s\' requested",
           sampler.getName().c_str());
}

void SharedProgramState::updateTo(GL::Uniform& uniform)
{
  switch (uniform.getSharedID())
  {
    case SHARED_MODEL_MATRIX:
    {
      uniform.copyFrom(value_ptr(modelMatrix));
      return;
    }

    case SHARED_VIEW_MATRIX:
    {
      uniform.copyFrom(value_ptr(viewMatrix));
      return;
    }

    case SHARED_PROJECTION_MATRIX:
    {
      uniform.copyFrom(value_ptr(projectionMatrix));
      return;
    }

    case SHARED_MODELVIEW_MATRIX:
    {
      if (dirtyModelView)
      {
        modelViewMatrix = viewMatrix;
        modelViewMatrix *= modelMatrix;
        dirtyModelView = false;
      }

      uniform.copyFrom(value_ptr(modelViewMatrix));
      return;
    }

    case SHARED_VIEWPROJECTION_MATRIX:
    {
      if (dirtyViewProj)
      {
        viewProjMatrix = projectionMatrix;
        viewProjMatrix *= viewMatrix;
        dirtyViewProj = false;
      }

      uniform.copyFrom(value_ptr(viewProjMatrix));
      return;
    }

    case SHARED_MODELVIEWPROJECTION_MATRIX:
    {
      if (dirtyModelViewProj)
      {
        if (dirtyViewProj)
        {
          viewProjMatrix = projectionMatrix;
          viewProjMatrix *= viewMatrix;
          dirtyViewProj = false;
        }

        modelViewProjMatrix = viewProjMatrix;
        modelViewProjMatrix *= modelMatrix;
        dirtyModelViewProj = false;
      }

      uniform.copyFrom(value_ptr(modelViewProjMatrix));
      return;
    }

    case SHARED_CAMERA_POSITION:
    {
      uniform.copyFrom(value_ptr(cameraPos));
      return;
    }

    case SHARED_CAMERA_NEAR_Z:
    {
      uniform.copyFrom(&cameraNearZ);
      return;
    }

    case SHARED_CAMERA_FAR_Z:
    {
      uniform.copyFrom(&cameraFarZ);
      return;
    }

    case SHARED_CAMERA_ASPECT_RATIO:
    {
      uniform.copyFrom(&cameraAspect);
      return;
    }

    case SHARED_CAMERA_FOV:
    {
      uniform.copyFrom(&cameraFOV);
      return;
    }

    case SHARED_VIEWPORT_WIDTH:
    {
      uniform.copyFrom(&viewportWidth);
      return;
    }

    case SHARED_VIEWPORT_HEIGHT:
    {
      uniform.copyFrom(&viewportHeight);
      return;
    }

    case SHARED_TIME:
    {
      uniform.copyFrom(&time);
      return;
    }
  }

  logError("Unknown shared uniform \'%s\' requested",
           uniform.getName().c_str());
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
  ID(allocateID())
{
}

ProgramState::ProgramState(const ProgramState& source):
  ID(allocateID()),
  program(source.program),
  floats(source.floats),
  textures(source.textures)
{
}

ProgramState::~ProgramState()
{
  releaseID(ID);
}

void ProgramState::apply() const
{
  if (!program)
  {
    logError("Applying program state with no program set");
    return;
  }

  GL::Context& context = program->getContext();
  context.setCurrentProgram(program);

  GL::SharedProgramState* state = context.getCurrentSharedProgramState();

  unsigned int textureIndex = 0, textureUnit = 0;

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    context.setActiveTextureUnit(textureUnit);

    GL::Sampler& sampler = program->getSampler(i);
    if (sampler.isShared())
    {
      if (state)
        state->updateTo(sampler);
      else
        logError("Program \'%s\' uses shared sampler \'%s\' without a current shared program state",
                 program->getName().c_str(),
                 sampler.getName().c_str());
    }
    else
    {
      context.setCurrentTexture(textures[textureIndex]);
      textureIndex++;
    }

    sampler.bind(textureUnit);
    textureUnit++;
  }

  size_t offset = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    GL::Uniform& uniform = program->getUniform(i);
    if (uniform.isShared())
    {
      if (state)
        state->updateTo(uniform);
      else
        logError("Program \'%s\' uses shared uniform \'%s\' without a current shared program state",
                 program->getName().c_str(),
                 uniform.getName().c_str());
    }
    else
    {
      uniform.copyFrom(&floats[0] + offset);
      offset += uniform.getElementCount();
    }
  }
}

bool ProgramState::hasUniformState(const char* name) const
{
  if (!program)
    return false;

  GL::Uniform* uniform = program->findUniform(name);
  if (!uniform)
    return false;

  return !uniform->isShared();
}

bool ProgramState::hasSamplerState(const char* name) const
{
  if (!program)
    return false;

  GL::Sampler* sampler = program->findSampler(name);
  if (!sampler)
    return false;

  return !sampler->isShared();
}

GL::Texture* ProgramState::getSamplerState(const char* name) const
{
  if (!program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return NULL;
  }

  unsigned int textureIndex = 0;

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    const GL::Sampler& sampler = program->getSampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.getName() == name)
      return textures[textureIndex];

    textureIndex++;
  }

  logError("Program \'%s\' has no sampler named \'%s\'",
           program->getName().c_str(),
           name);
  return NULL;
}

GL::Texture* ProgramState::getSamplerState(SamplerStateIndex index) const
{
  if (!program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return NULL;
  }

  return textures[index.unit];
}

void ProgramState::setSamplerState(const char* name, GL::Texture* newTexture)
{
  if (!program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  unsigned int textureIndex = 0;

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    GL::Sampler& sampler = program->getSampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.getName() == name)
    {
      if (newTexture)
      {
        if (samplerTypeMatchesTextureType(sampler.getType(), newTexture->getType()))
          textures[textureIndex] = newTexture;
        else
          logError("Type mismatch between sampler \'%s\' and texture \'%s\'",
                   sampler.getName().c_str(),
                   newTexture->getName().c_str());
      }
      else
        textures[textureIndex] = NULL;

      return;
    }

    textureIndex++;
  }
}

void ProgramState::setSamplerState(SamplerStateIndex index, GL::Texture* newTexture)
{
  if (!program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  GL::Sampler& sampler = program->getSampler(index.index);

  if (newTexture)
  {
    if (samplerTypeMatchesTextureType(sampler.getType(), newTexture->getType()))
      textures[index.unit] = newTexture;
    else
      logError("Type mismatch between sampler \'%s\' and texture \'%s\'",
                sampler.getName().c_str(),
                newTexture->getName().c_str());
  }
  else
    textures[index.unit] = NULL;
}

UniformStateIndex ProgramState::getUniformStateIndex(const char* name) const
{
  if (!program)
  {
    logError("Cannot retrieve uniform state indices with no program");
    return UniformStateIndex();
  }

  unsigned int offset = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    GL::Uniform& uniform = program->getUniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.getName() == name)
      return UniformStateIndex(i, offset);

    offset += uniform.getElementCount();
  }

  return UniformStateIndex();
}

SamplerStateIndex ProgramState::getSamplerStateIndex(const char* name) const
{
  if (!program)
  {
    logError("Cannot retrieve sampler state indices with no program");
    return SamplerStateIndex();
  }

  unsigned int textureIndex = 0;

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    GL::Sampler& sampler = program->getSampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.getName() == name)
      return SamplerStateIndex(i, textureIndex);

    textureIndex++;
  }

  return SamplerStateIndex();
}

GL::Program* ProgramState::getProgram() const
{
  return program;
}

void ProgramState::setProgram(GL::Program* newProgram)
{
  floats.clear();
  textures.clear();

  program = newProgram;
  if (!program)
    return;

  unsigned int floatCount = 0;
  unsigned int textureCount = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    GL::Uniform& uniform = program->getUniform(i);
    if (!uniform.isShared())
      floatCount += uniform.getElementCount();
  }

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    GL::Sampler& sampler = program->getSampler(i);
    if (!sampler.isShared())
      textureCount++;
  }

  floats.insert(floats.end(), floatCount, 0.f);
  textures.resize(textureCount);
}

StateID ProgramState::getID() const
{
  return ID;
}

StateID ProgramState::allocateID()
{
  if (usedIDs.empty())
    return nextID++;

  const StateID ID = usedIDs.back();
  usedIDs.pop_back();
  return ID;
}

void ProgramState::releaseID(StateID ID)
{
  usedIDs.push_front(ID);
}

void* ProgramState::getData(const char* name, GL::UniformType type)
{
  if (!program)
  {
    logError("Cannot set uniform state on program state with no program");
    return NULL;
  }

  unsigned int offset = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    GL::Uniform& uniform = program->getUniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.getName() == name)
    {
      if (uniform.getType() == type)
        return &floats[0] + offset;

      logError("Uniform \'%s\' of program \'%s\' is not of type \'%s\'",
               uniform.getName().c_str(),
               program->getName().c_str(),
               GL::Uniform::getTypeName(type));
      return NULL;
    }

    offset += uniform.getElementCount();
  }

  logError("Program \'%s\' has no uniform named \'%s\'",
           program->getName().c_str(),
           name);
  return NULL;
}

const void* ProgramState::getData(const char* name, GL::UniformType type) const
{
  if (!program)
  {
    logError("Cannot set uniform state on program state with no program");
    return NULL;
  }

  unsigned int offset = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    GL::Uniform& uniform = program->getUniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.getName() == name)
    {
      if (uniform.getType() == type)
        return &floats[0] + offset;

      logError("Uniform \'%s\' of program \'%s\' is not of type \'%s\'",
               uniform.getName().c_str(),
               program->getName().c_str(),
               GL::Uniform::getTypeName(type));
      return NULL;
    }

    offset += uniform.getElementCount();
  }

  logError("Program \'%s\' has no uniform named \'%s\'",
           program->getName().c_str(),
           name);
  return NULL;
}

void* ProgramState::getData(UniformStateIndex index, GL::UniformType type)
{
  if (!program)
  {
    logError("Cannot set uniform state on program state with no program");
    return NULL;
  }

  GL::Uniform& uniform = program->getUniform(index.index);

  if (uniform.getType() != type)
  {
    logError("Uniform %u of program \'%s\' is not of type \'%s\'",
             index.index,
             program->getName().c_str(),
             GL::Uniform::getTypeName(type));
    return NULL;
  }

  return &floats[0] + index.offset;
}

const void* ProgramState::getData(UniformStateIndex index, GL::UniformType type) const
{
  if (!program)
  {
    logError("Cannot set uniform state on program state with no program");
    return NULL;
  }

  GL::Uniform& uniform = program->getUniform(index.index);

  if (uniform.getType() != type)
  {
    logError("Uniform %u of program \'%s\' is not of type \'%s\'",
             index.index,
             program->getName().c_str(),
             GL::Uniform::getTypeName(type));
    return NULL;
  }

  return &floats[0] + index.offset;
}

template <>
GL::UniformType ProgramState::getUniformType<float>()
{
  return GL::UNIFORM_FLOAT;
}

template <>
GL::UniformType ProgramState::getUniformType<vec2>()
{
  return GL::UNIFORM_VEC2;
}

template <>
GL::UniformType ProgramState::getUniformType<vec3>()
{
  return GL::UNIFORM_VEC3;
}

template <>
GL::UniformType ProgramState::getUniformType<vec4>()
{
  return GL::UNIFORM_VEC4;
}

template <>
GL::UniformType ProgramState::getUniformType<mat2>()
{
  return GL::UNIFORM_MAT2;
}

template <>
GL::UniformType ProgramState::getUniformType<mat3>()
{
  return GL::UNIFORM_MAT3;
}

template <>
GL::UniformType ProgramState::getUniformType<mat4>()
{
  return GL::UNIFORM_MAT4;
}

ProgramState::IDQueue ProgramState::usedIDs;

StateID ProgramState::nextID = 0;

///////////////////////////////////////////////////////////////////////

void Pass::apply() const
{
  GL::Program* program = getProgram();
  if (!program)
  {
    logError("Applying render state with no program set");
    return;
  }

  GL::Context& context = program->getContext();
  context.setCurrentRenderState(data);

  ProgramState::apply();
}

bool Pass::isCulling() const
{
  return data.cullMode != GL::CULL_NONE;
}

bool Pass::isBlending() const
{
  return data.srcFactor != GL::BLEND_ONE || data.dstFactor != GL::BLEND_ZERO;
}

bool Pass::isDepthTesting() const
{
  return data.depthTesting;
}

bool Pass::isDepthWriting() const
{
  return data.depthWriting;
}

bool Pass::isColorWriting() const
{
  return data.colorWriting;
}

bool Pass::isStencilTesting() const
{
  return data.stencilTesting;
}

bool Pass::isWireframe() const
{
  return data.wireframe;
}

bool Pass::isLineSmoothing() const
{
  return data.lineSmoothing;
}

bool Pass::isMultisampling() const
{
  return data.multisampling;
}

float Pass::getLineWidth() const
{
  return data.lineWidth;
}

GL::CullMode Pass::getCullMode() const
{
  return data.cullMode;
}

GL::BlendFactor Pass::getSrcFactor() const
{
  return data.srcFactor;
}

GL::BlendFactor Pass::getDstFactor() const
{
  return data.dstFactor;
}

GL::Function Pass::getDepthFunction() const
{
  return data.depthFunction;
}

GL::Function Pass::getStencilFunction() const
{
  return data.stencilFunction;
}

GL::Operation Pass::getStencilFailOperation() const
{
  return data.stencilFailOp;
}

GL::Operation Pass::getDepthFailOperation() const
{
  return data.depthFailOp;
}

GL::Operation Pass::getDepthPassOperation() const
{
  return data.depthPassOp;
}

unsigned int Pass::getStencilReference() const
{
  return data.stencilRef;
}

unsigned int Pass::getStencilWriteMask() const
{
  return data.stencilMask;
}

void Pass::setDepthTesting(bool enable)
{
  data.depthTesting = enable;
}

void Pass::setDepthWriting(bool enable)
{
  data.depthWriting = enable;
}

void Pass::setStencilTesting(bool enable)
{
  data.stencilTesting = enable;
}

void Pass::setCullMode(GL::CullMode mode)
{
  data.cullMode = mode;
}

void Pass::setBlendFactors(GL::BlendFactor src, GL::BlendFactor dst)
{
  data.srcFactor = src;
  data.dstFactor = dst;
}

void Pass::setDepthFunction(GL::Function function)
{
  data.depthFunction = function;
}

void Pass::setStencilFunction(GL::Function newFunction)
{
  data.stencilFunction = newFunction;
}

void Pass::setStencilReference(unsigned int newReference)
{
  data.stencilRef = newReference;
}

void Pass::setStencilWriteMask(unsigned int newMask)
{
  data.stencilMask = newMask;
}

void Pass::setStencilFailOperation(GL::Operation newOperation)
{
  data.stencilFailOp = newOperation;
}

void Pass::setDepthFailOperation(GL::Operation newOperation)
{
  data.depthFailOp = newOperation;
}

void Pass::setDepthPassOperation(GL::Operation newOperation)
{
  data.depthPassOp = newOperation;
}

void Pass::setColorWriting(bool enabled)
{
  data.colorWriting = enabled;
}

void Pass::setWireframe(bool enabled)
{
  data.wireframe = enabled;
}

void Pass::setLineSmoothing(bool enabled)
{
  data.lineSmoothing = enabled;
}

void Pass::setMultisampling(bool enabled)
{
  data.multisampling = enabled;
}

void Pass::setLineWidth(float newWidth)
{
  data.lineWidth = newWidth;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

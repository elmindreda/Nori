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

SharedProgramState::SharedProgramState():
  dirtyModelView(true),
  dirtyViewProj(true),
  dirtyModelViewProj(true),
  viewportWidth(0.f),
  viewportHeight(0.f),
  time(0.f)
{
}

bool SharedProgramState::reserveSupported(GL::Context& context) const
{
  context.createSharedUniform("wyM", GL::Uniform::MAT4, SHARED_MODEL_MATRIX);
  context.createSharedUniform("wyV", GL::Uniform::MAT4, SHARED_VIEW_MATRIX);
  context.createSharedUniform("wyP", GL::Uniform::MAT4, SHARED_PROJECTION_MATRIX);
  context.createSharedUniform("wyMV", GL::Uniform::MAT4, SHARED_MODELVIEW_MATRIX);
  context.createSharedUniform("wyVP", GL::Uniform::MAT4, SHARED_VIEWPROJECTION_MATRIX);
  context.createSharedUniform("wyMVP", GL::Uniform::MAT4, SHARED_MODELVIEWPROJECTION_MATRIX);

  context.createSharedUniform("wyCameraNearZ", GL::Uniform::FLOAT, SHARED_CAMERA_NEAR_Z);
  context.createSharedUniform("wyCameraFarZ", GL::Uniform::FLOAT, SHARED_CAMERA_FAR_Z);
  context.createSharedUniform("wyCameraAspectRatio", GL::Uniform::FLOAT, SHARED_CAMERA_ASPECT_RATIO);
  context.createSharedUniform("wyCameraFOV", GL::Uniform::FLOAT, SHARED_CAMERA_FOV);
  context.createSharedUniform("wyCameraPosition", GL::Uniform::VEC3, SHARED_CAMERA_POSITION);

  context.createSharedUniform("wyViewportWidth", GL::Uniform::FLOAT, SHARED_VIEWPORT_WIDTH);
  context.createSharedUniform("wyViewportHeight", GL::Uniform::FLOAT, SHARED_VIEWPORT_HEIGHT);

  context.createSharedUniform("wyTime", GL::Uniform::FLOAT, SHARED_TIME);

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
  position = camera.position;
  FOV = camera.FOV;
  aspect = camera.aspect;
  nearZ = camera.nearZ;
  farZ = camera.farZ;
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
  camera.position = position;
  camera.FOV = FOV;
  camera.aspect = aspect;
  camera.nearZ = nearZ;
  camera.farZ = farZ;
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
      uniform.copyFrom(value_ptr(camera.position));
      return;
    }

    case SHARED_CAMERA_NEAR_Z:
    {
      uniform.copyFrom(&camera.nearZ);
      return;
    }

    case SHARED_CAMERA_FAR_Z:
    {
      uniform.copyFrom(&camera.farZ);
      return;
    }

    case SHARED_CAMERA_ASPECT_RATIO:
    {
      uniform.copyFrom(&camera.aspect);
      return;
    }

    case SHARED_CAMERA_FOV:
    {
      uniform.copyFrom(&camera.FOV);
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

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

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

SharedProgramState::SharedProgramState(void):
  dirtyModelView(true),
  dirtyViewProj(true),
  dirtyModelViewProj(true)
{
}

bool SharedProgramState::reserveSupported(GL::Context& context) const
{
  context.createSharedUniform("wyM", GL::Uniform::MAT4, render::SHARED_MODEL_MATRIX);
  context.createSharedUniform("wyV", GL::Uniform::MAT4, render::SHARED_VIEW_MATRIX);
  context.createSharedUniform("wyP", GL::Uniform::MAT4, render::SHARED_PROJECTION_MATRIX);
  context.createSharedUniform("wyMV", GL::Uniform::MAT4, render::SHARED_MODELVIEW_MATRIX);
  context.createSharedUniform("wyVP", GL::Uniform::MAT4, render::SHARED_VIEWPROJECTION_MATRIX);
  context.createSharedUniform("wyMVP", GL::Uniform::MAT4, render::SHARED_MODELVIEWPROJECTION_MATRIX);

  context.createSharedUniform("wyNearZ", GL::Uniform::FLOAT, render::SHARED_NEAR_Z);
  context.createSharedUniform("wyFarZ", GL::Uniform::FLOAT, render::SHARED_FAR_Z);
  context.createSharedUniform("wyAspectRatio", GL::Uniform::FLOAT, render::SHARED_ASPECT_RATIO);
  context.createSharedUniform("wyFOV", GL::Uniform::FLOAT, render::SHARED_FOV);

  //context.createSharedUniform("wyViewportWidth", GL::Uniform::INT, render::SHARED_VIEWPORT_WIDTH);
  //context.createSharedUniform("wyViewportHeight", GL::Uniform::INT, render::SHARED_VIEWPORT_HEIGHT);

  context.createSharedUniform("wyTime", GL::Uniform::FLOAT, render::SHARED_TIME);

  return true;
}

const mat4& SharedProgramState::getModelMatrix(void) const
{
  return modelMatrix;
}

void SharedProgramState::setModelMatrix(const mat4& newMatrix)
{
  modelMatrix = newMatrix;
  dirtyModelView = dirtyModelViewProj = true;
}

const mat4& SharedProgramState::getViewMatrix(void) const
{
  return viewMatrix;
}

void SharedProgramState::setViewMatrix(const mat4& newMatrix)
{
  viewMatrix = newMatrix;
  dirtyModelView = dirtyViewProj = dirtyModelViewProj = true;
}

const mat4& SharedProgramState::getProjectionMatrix(void) const
{
  return projectionMatrix;
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
  }

  logError("Unknown shared uniform \'%s\' requested",
           uniform.getName().c_str());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Matrix.h>
#include <moira/Quaternion.h>
#include <moira/Transform.h>
#include <moira/Image.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLCamera.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Camera::Camera(const std::string& name):
  Managed<Camera>(name),
  FOV(90.f),
  aspectRatio(0.f)
{
}

void Camera::begin(void) const
{
  if (current != NULL)
    throw Exception("Cannot nest cameras");

  Canvas::getCurrent()->begin3D(FOV, aspectRatio);

  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  Transform3 reverseTransform = transform;
  reverseTransform.invert();
  Matrix4 matrix = reverseTransform;
  glLoadMatrixf(matrix);

  glPopAttrib();

  current = const_cast<Camera*>(this);
}

void Camera::end(void) const
{
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  Canvas::getCurrent()->end();

  current = NULL;
}

float Camera::getFOV(void) const
{
  return FOV;
}

float Camera::getAspectRatio(void) const
{
  return aspectRatio;
}

void Camera::setFOV(float newFOV)
{
  FOV = newFOV;
}

void Camera::setAspectRatio(float newAspectRatio)
{
  aspectRatio = newAspectRatio;
}

Transform3& Camera::getTransform(void)
{
  return transform;
}

const Transform3& Camera::getTransform(void) const
{
  return transform;
}

Camera* Camera::getCurrent(void)
{
  return current;
}

Camera* Camera::current = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

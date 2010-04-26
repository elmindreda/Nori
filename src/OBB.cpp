///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Core.h>
#include <wendy/Vector.h>
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>
#include <wendy/OBB.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

OBB2::OBB2(void)
{
}

OBB2::OBB2(const Vec2& initSize, const Transform2& initOrientation):
  size(initSize),
  orientation(initOrientation)
{
}

OBB2::OBB2(float width, float height):
  size(width, height)
{
}

bool OBB2::contains(const Vec2& point) const
{
  Vec2 x, y;
  getAxes(x, y);

  const Vec2 local = point - orientation.position;

  // NOTE: Assumes positive size.

  if (fabsf(local.dot(x)) > size.x / 2.f)
    return false;

  if (fabsf(local.dot(y)) > size.y / 2.f)
    return false;

  return true;
}

bool OBB2::intersects(const OBB2& other) const
{
  // TODO: Implement.

  return false;
}

void OBB2::getAxes(Vec2& x, Vec2& y) const
{
  const float sina = sinf(orientation.angle);
  const float cosa = cosf(orientation.angle);

  x.set(cosa, sina);
  y.set(-sina, cosa);
}

void OBB2::set(const Vec2& newSize, const Transform2& newOrientation)
{
  size = newSize;
  orientation = newOrientation;
}

void OBB2::set(float newWidth, float newHeight)
{
  size.set(newWidth, newHeight);
  orientation.setIdentity();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

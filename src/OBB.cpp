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
#include <wendy/Transform.h>
#include <wendy/OBB.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

OBB2::OBB2()
{
}

OBB2::OBB2(const vec2& initSize, const vec2& initCenter, float initAngle):
  size(initSize),
  center(initCenter),
  angle(initAngle)
{
}

bool OBB2::contains(const vec2& point) const
{
  vec2 x, y;
  getAxes(x, y);

  const vec2 local = point - center;

  // NOTE: Assumes positive size.

  if (abs(dot(local, x)) > size.x / 2.f)
    return false;

  if (abs(dot(local, y)) > size.y / 2.f)
    return false;

  return true;
}

bool OBB2::intersects(const OBB2& other) const
{
  // TODO: Implement.

  return false;
}

void OBB2::transformBy(const Transform2& transform)
{
  size *= transform.scale;
  angle += transform.angle;
  center += transform.position;
}

void OBB2::getAxes(vec2& x, vec2& y) const
{
  const float sina = sin(angle);
  const float cosa = cos(angle);

  x = vec2(cosa, sina);
  y = vec2(-sina, cosa);
}

void OBB2::set(const vec2& newSize, const vec2& newCenter, float newAngle)
{
  size = newSize;
  center = newCenter;
  angle = newAngle;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

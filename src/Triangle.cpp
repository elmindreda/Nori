///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Plane.h>
#include <wendy/Sphere.h>
#include <wendy/Triangle.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Triangle2::Triangle2()
{
}

Triangle2::Triangle2(const vec2& P0, const vec2& P1, const vec2& P2)
{
  set(P0, P1, P2);
}

vec2 Triangle2::center() const
{
  return (P[0] + P[1] + P[2]) / 3.f;
}

bool Triangle2::contains(const vec2& point) const
{
  // TODO: Implement.

  return false;
}

void Triangle2::set(const vec2& P0, const vec2& P1, const vec2& P2)
{
  P[0] = P0;
  P[1] = P1;
  P[2] = P2;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

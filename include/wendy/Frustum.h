///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#ifndef WENDY_FRUSTUM_H
#define WENDY_FRUSTUM_H
///////////////////////////////////////////////////////////////////////

#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class AABB;
class Sphere;

///////////////////////////////////////////////////////////////////////

class Frustum
{
public:
  Frustum(void);
  Frustum(float FOV, float aspectRatio, float farZ = 1000.f);
  bool contains(const vec3& point) const;
  bool contains(const Sphere& sphere) const;
  bool contains(const AABB& box) const;
  bool intersects(const Sphere& sphere) const;
  bool intersects(const AABB& box) const;
  void transformBy(const Transform3& transform);
  void set(float FOV, float aspectRatio, float farZ = 1000.f);
  typedef std::vector<Plane> PlaneList;
  PlaneList planes;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_FRUSTUM_H*/
///////////////////////////////////////////////////////////////////////

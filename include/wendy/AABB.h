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
#ifndef WENDY_AABB_H
#define WENDY_AABB_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class AABB
{
public:
  AABB(void);
  AABB(const vec3& center, const vec3& size);
  AABB(float width, float height, float depth);
  bool contains(const vec3& point) const;
  bool contains(const AABB& other) const;
  bool intersects(const AABB& other) const;
  void envelop(const vec3& point);
  void envelop(const AABB& other);
  void normalize(void);
  void getBounds(float& minX, float& minY, float& minZ,
                 float& maxX, float& maxY, float& maxZ) const;
  void setBounds(float minX, float minY, float minZ,
                 float maxX, float maxY, float maxZ);
  void set(const vec3& newCenter, const vec3& newSize);
  void set(float newWidth, float newHeight, float newDepth);
  vec3 center;
  vec3 size;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_AABB_H*/
///////////////////////////////////////////////////////////////////////

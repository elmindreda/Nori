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
#ifndef WENDY_OBB_H
#define WENDY_OBB_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief 2D oriented bounding box.
 */
class OBB2
{
public:
  OBB2();
  OBB2(const vec2& center, const vec2& size, float angle = 0.f);
  bool contains(const vec2& point) const;
  bool intersects(const OBB2& other) const;
  void transformBy(const Transform2& transform);
  void getAxes(vec2& x, vec2& y) const;
  void set(const vec2& newSize, const vec2& newCenter, float newAngle = 0.f);
  vec2 size;
  vec2 center;
  float angle;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_OBB_H*/
///////////////////////////////////////////////////////////////////////

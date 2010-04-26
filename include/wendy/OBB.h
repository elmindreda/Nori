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

class OBB2
{
public:
  OBB2(void);
  OBB2(const Vec2& size, const Transform2& orientation);
  OBB2(float width, float height);
  bool contains(const Vec2& point) const;
  bool intersects(const OBB2& other) const;
  void getAxes(Vec2& x, Vec2& y) const;
  void set(const Vec2& newSize, const Transform2& newOrientation);
  void set(float newWidth, float newHeight);
  Vec2 size;
  Transform2 orientation;
};

///////////////////////////////////////////////////////////////////////

class OBB3
{
public:
  OBB3(void);
  OBB3(const Vec3& size, const Transform3& orientation);
  OBB3(float width, float height, float depth);
  bool contains(const Vec3& point) const;
  bool intersects(const OBB3& other) const;
  void set(const Vec3& newSize, const Transform3& orientation);
  void set(float newWidth, float newHeight, float newDepth);
  Vec3 size;
  Transform3 orientation;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_OBB_H*/
///////////////////////////////////////////////////////////////////////

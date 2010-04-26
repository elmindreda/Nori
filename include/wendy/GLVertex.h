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
#ifndef WENDY_GLVERTEX_H
#define WENDY_GLVERTEX_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Vertex.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex3fv
{
public:
  Vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex3fn3fv
{
public:
  Vec3 normal;
  Vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2fv
{
public:
  Vec2 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2ft2fv
{
public:
  Vec2 mapping;
  Vec2 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2ft3fv
{
public:
  Vec2 mapping;
  Vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex4fc2ft3fv
{
public:
  ColorRGBA color;
  Vec2 mapping;
  Vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLVERTEX_H*/
///////////////////////////////////////////////////////////////////////

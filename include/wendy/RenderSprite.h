///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERSPRITE_H
#define WENDY_RENDERSPRITE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class RenderStyle;

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class Sprite2
{
public:
  Sprite2(void);
  void render(void) const;
  void render(const Style& style) const;
  void realizeVertices(GL::Vertex2ft2fv* vertices) const;
  void setDefaults(void);
  Vector2 position;
  Vector2 size;
  float angle;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class Sprite3 : public Renderable
{
public:
  Sprite3(void);
  void enqueue(Queue& queue, const Transform3& transform) const;
  void render(void) const;
  void realizeVertices(GL::Vertex2ft3fv* vertices) const;
  void setDefaults(void);
  Vector3 position;
  Vector2 size;
  float angle;
  String styleName;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSPRITE_H*/
///////////////////////////////////////////////////////////////////////

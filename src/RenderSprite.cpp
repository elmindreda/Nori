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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLVertex.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderStyle.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderSprite.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

void rotateVector2(Vector2& vector, float angle)
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Vector2 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  vector = result;
}

void rotateVector3(Vector3& vector, float angle)
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Vector3 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  result.z = vector.z;
  vector = result;
}

}

///////////////////////////////////////////////////////////////////////

Sprite2::Sprite2(void)
{
  setDefaults();
}

void Sprite2::render(void) const
{
  GL::Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  glBegin(GL_QUADS);
  for (unsigned int i = 0;  i < 4;  i++)
    vertices[i].send();
  glEnd();
}

void Sprite2::render(const Style& style) const
{
  GL::Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  const Technique* technique = style.getActiveTechnique();

  for (unsigned int pass = 0;  pass < technique->getPassCount();  pass++)
  {
    technique->applyPass(pass);

    glBegin(GL_QUADS);
    for (unsigned int i = 0;  i < 4;  i++)
      vertices[i].send();
    glEnd();
  }
}

void Sprite2::realizeVertices(GL::Vertex2ft2fv* vertices) const
{
  const Vector2 offset(size.x / 2.f, size.y / 2.f);

  vertices[0].mapping.set(1.f, 1.f);
  vertices[0].position.set( offset.x,  offset.y);
  vertices[1].mapping.set(0.f, 1.f);
  vertices[1].position.set(-offset.x,  offset.y);
  vertices[2].mapping.set(0.f, 0.f);
  vertices[2].position.set(-offset.x, -offset.y);
  vertices[3].mapping.set(1.f, 0.f);
  vertices[3].position.set( offset.x, -offset.y);

  for (unsigned int i = 0;  i < 4;  i++)
  {
    rotateVector2(vertices[i].position, angle);
    vertices[i].position += position;
  }
}

void Sprite2::setDefaults(void)
{
  position.set(0.f, 0.f);
  size.set(1.f, 1.f);
  angle = 0.f;
}

///////////////////////////////////////////////////////////////////////

Sprite3::Sprite3(void)
{
  setDefaults();
}

void Sprite3::enqueue(Queue& queue, const Transform3& transform) const
{
  if (!GL::Renderer::get())
  {
    Log::writeError("Cannot enqueue sprites without a renderer");
    return;
  }

  if (styleName.empty())
  {
    Log::writeError("Cannot enqueue with no render style name set");
    return;
  }

  Style* style = Style::findInstance(styleName);
  if (!style)
  {
    Log::writeError("Render style %s not found", styleName.c_str());
    return;
  }

  const Technique* technique = style->getActiveTechnique();
  if (!technique)
  {
    Log::writeError("Render style %s has no active technique", styleName.c_str());
    return;
  }

  GL::VertexRange range;
  if (!GL::Renderer::get()->allocateVertices(range, 4, GL::Vertex2ft3fv::format))
    return;

  GL::Vertex2ft3fv* vertices = (GL::Vertex2ft3fv*) range.lock();
  if (!vertices)
    return;

  realizeVertices(vertices);
  range.unlock();

  Operation& operation = queue.createOperation();
  operation.vertexBuffer = range.getVertexBuffer();
  operation.start = range.getStart();
  operation.count = range.getCount();
  operation.renderMode = GL_QUADS;
  operation.transform = transform;
  operation.technique = technique;
}

void Sprite3::render(void) const
{
  GL::Vertex2ft3fv vertices[4];
  realizeVertices(vertices);

  if (styleName.empty())
  {
    glBegin(GL_QUADS);
    for (unsigned int i = 0;  i < 4;  i++)
      vertices[i].send();
    glEnd();
  }
  else
  {
    Style* style = Style::findInstance(styleName);
    if (!style)
    {
      Log::writeError("Render style %s not found", styleName.c_str());
      return;
    }

    const Technique* technique = style->getActiveTechnique();
    if (!technique)
    {
      Log::writeError("Render style %s has no active technique", styleName.c_str());
      return;
    }

    for (unsigned int pass = 0;  pass < technique->getPassCount();  pass++)
    {
      technique->applyPass(pass);

      glBegin(GL_QUADS);
      for (unsigned int i = 0;  i < 4;  i++)
	vertices[i].send();
      glEnd();
    }
  }
}

void Sprite3::realizeVertices(GL::Vertex2ft3fv* vertices) const
{
  const Vector2 offset(size.x / 2.f, size.y / 2.f);

  vertices[0].mapping.set(0.f, 0.f);
  vertices[0].position.set(-offset.x, -offset.y, 0.f);
  vertices[1].mapping.set(1.f, 0.f);
  vertices[1].position.set( offset.x, -offset.y, 0.f);
  vertices[2].mapping.set(1.f, 1.f);
  vertices[2].position.set( offset.x,  offset.y, 0.f);
  vertices[3].mapping.set(0.f, 1.f);
  vertices[3].position.set(-offset.x,  offset.y, 0.f);

  for (unsigned int i = 0;  i < 4;  i++)
  {
    rotateVector3(vertices[i].position, angle);
    vertices[i].position += position;
  }
}

void Sprite3::setDefaults(void)
{
  position.set(0.f, 0.f, 0.f);
  size.set(1.f, 1.f);
  angle = 0.f;
  styleName.clear();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

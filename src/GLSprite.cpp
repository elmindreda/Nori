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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Vector.h>
#include <moira/Color.h>
#include <moira/Matrix.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>
#include <wendy/GLSprite.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
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
  Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  glBegin(GL_QUADS);
  for (unsigned int i = 0;  i < 4;  i++)
    vertices[i].send();
  glEnd();
}

void Sprite2::render(const Shader& shader) const
{
  Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  for (unsigned int pass = 0;  pass < shader.getPassCount();  pass++)
  {
    shader.applyPass(pass);

    glBegin(GL_QUADS);
    for (unsigned int i = 0;  i < 4;  i++)
      vertices[i].send();
    glEnd();
  }
}

void Sprite2::realizeVertices(Vertex2ft2fv* vertices) const
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

void Sprite3::enqueue(RenderQueue& queue,
                      const Matrix4& transform,
		      const Shader& shader) const
{
  if (!Renderer::get())
  {
    Log::writeError("Cannot enqueue sprites without a renderer");
    return;
  }

  VertexBufferRange range;
  if (!Renderer::get()->allocateVertices(range, 4, Vertex2ft3fv::format))
    return;

  Vertex2ft3fv* vertices = (Vertex2ft3fv*) range.lock();
  if (!vertices)
    return;

  realizeVertices(vertices);
  range.unlock();

  RenderOperation operation;
  operation.vertexBuffer = range.getVertexBuffer();
  operation.start = range.getStart();
  operation.count = range.getCount();
  operation.renderMode = GL_QUADS;
  operation.transform = transform;
  operation.shader = &shader;
  queue.addOperation(operation);
}

void Sprite3::render(void) const
{
  Vertex2ft3fv vertices[4];
  realizeVertices(vertices);

  glBegin(GL_QUADS);
  for (unsigned int i = 0;  i < 4;  i++)
    vertices[i].send();
  glEnd();
}

void Sprite3::render(const Shader& shader) const
{
  Vertex2ft3fv vertices[4];
  realizeVertices(vertices);

  for (unsigned int pass = 0;  pass < shader.getPassCount();  pass++)
  {
    shader.applyPass(pass);

    glBegin(GL_QUADS);
    for (unsigned int i = 0;  i < 4;  i++)
      vertices[i].send();
    glEnd();
  }
}

void Sprite3::realizeVertices(Vertex2ft3fv* vertices) const
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
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

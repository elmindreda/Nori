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

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
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

void rotateVec2(Vec2& vector, float angle)
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Vec2 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  vector = result;
}

void rotateVec3(Vec3& vector, float angle)
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Vec3 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  result.z = vector.z;
  vector = result;
}

void realizeSpriteVertices(GL::Vertex2ft3fv* vertices,
			   const Vec3& camera,
                           const Vec3& position,
		           const Vec2& size,
		           float angle,
			   SpriteType3 type)
{
  const Vec2 offset(size.x / 2.f, size.y / 2.f);

  if (type == STATIC_SPRITE)
  {
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
      rotateVec3(vertices[i].position, angle);
      vertices[i].position += position;
    }
  }
  else if (type == CYLINDRIC_SPRITE)
  {
  }
  else if (type == SPHERICAL_SPRITE)
  {
    Vec3 direction = (camera - position).normalized();

    Quat final;
    final.setVectorRotation(direction);

    if (angle != 0.f)
    {
      Quat local;
      local.setAxisRotation(Vec3(0.f, 0.f, 1.f), angle);

      final = final * local;
    }

    Vec3 positions[4];

    positions[0].set(-offset.x, -offset.y, 0.f);
    positions[1].set( offset.x, -offset.y, 0.f);
    positions[2].set( offset.x,  offset.y, 0.f);
    positions[3].set(-offset.x,  offset.y, 0.f);

    for (unsigned int i = 0;  i < 4;  i++)
    {
      final.rotateVector(positions[i]);
      positions[i] += position;
    }

    vertices[0].mapping.set(0.f, 0.f);
    vertices[0].position = positions[0];
    vertices[1].mapping.set(1.f, 0.f);
    vertices[1].position = positions[1];
    vertices[2].mapping.set(1.f, 1.f);
    vertices[2].position = positions[2];
    vertices[3].mapping.set(0.f, 1.f);
    vertices[3].position = positions[3];
  }
  else
    Log::writeError("Unknown sprite type %u", type);
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

  GL::Renderer* renderer = GL::Renderer::get();

  GL::VertexRange range;
  if (!renderer->allocateVertices(range, 4, GL::Vertex2ft2fv::format))
    return;

  range.copyFrom(vertices);

  renderer->setCurrentPrimitiveRange(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
  renderer->render();
}

void Sprite2::render(const Material& material) const
{
  GL::Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  GL::Renderer* renderer = GL::Renderer::get();

  GL::VertexRange range;
  if (!renderer->allocateVertices(range, 4, GL::Vertex2ft2fv::format))
    return;

  range.copyFrom(vertices);

  const Technique* technique = material.getActiveTechnique();

  renderer->setCurrentPrimitiveRange(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));

  for (unsigned int pass = 0;  pass < technique->getPassCount();  pass++)
  {
    technique->applyPass(pass);
    renderer->render();
  }
}

void Sprite2::realizeVertices(GL::Vertex2ft2fv* vertices) const
{
  const Vec2 offset(size.x / 2.f, size.y / 2.f);

  vertices[0].mapping.set(mapping.position.x + mapping.size.x, mapping.position.y + mapping.size.y);
  vertices[1].mapping.set(mapping.position.x, mapping.position.y + mapping.size.y);
  vertices[2].mapping.set(mapping.position.x, mapping.position.y);
  vertices[3].mapping.set(mapping.position.x + mapping.size.x, mapping.position.y);

  vertices[0].position.set( offset.x,  offset.y);
  vertices[1].position.set(-offset.x,  offset.y);
  vertices[2].position.set(-offset.x, -offset.y);
  vertices[3].position.set( offset.x, -offset.y);

  for (unsigned int i = 0;  i < 4;  i++)
  {
    rotateVec2(vertices[i].position, angle);
    vertices[i].position += position;
  }
}

void Sprite2::setDefaults(void)
{
  mapping.set(Vec2::ZERO, Vec2::ONE);
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
    Log::writeError("Cannot enqueue sprite without a renderer");
    return;
  }

  if (!material)
  {
    Log::writeError("Cannot enqueue sprite without a material");
    return;
  }

  const Technique* technique = material->getActiveTechnique();
  if (!technique)
  {
    Log::writeError("Material %s has no active technique",
                    material->getName().c_str());
    return;
  }

  GL::VertexRange range;
  if (!GL::Renderer::get()->allocateVertices(range, 4, GL::Vertex2ft3fv::format))
    return;

  const Vec3 camera = queue.getCamera().getTransform().position;

  GL::Vertex2ft3fv vertices[4];
  realizeVertices(vertices, transform, camera);
  range.copyFrom(vertices);

  Vec3 position = Vec3::ZERO;
  transform.transformVector(position);

  Operation& operation = queue.createOperation();
  operation.range = GL::PrimitiveRange(GL::TRIANGLE_FAN, range);
  operation.transform = transform;
  operation.technique = technique;
  operation.distance = (position - camera).length();
}

void Sprite3::realizeVertices(GL::Vertex2ft3fv* vertices,
                              const Transform3& transform,
                              const Vec3& camera) const
{
  Transform3 inverse = transform;
  inverse.invert();

  Vec3 localCamera = camera;
  inverse.transformVector(localCamera);

  realizeSpriteVertices(vertices, localCamera, position, size, angle, type);
}

void Sprite3::setDefaults(void)
{
  position.set(0.f, 0.f, 0.f);
  size.set(1.f, 1.f);
  angle = 0.f;
  type = STATIC_SPRITE;
  material = NULL;
}

///////////////////////////////////////////////////////////////////////

SpriteCloud3::SpriteCloud3(void):
  type(STATIC_SPRITE)
{
}

void SpriteCloud3::enqueue(Queue& queue, const Transform3& transform) const
{
  if (slots.empty())
    return;

  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
  {
    Log::writeError("Cannot enqueue sprite cloud without a renderer");
    return;
  }

  if (!material)
  {
    Log::writeError("Cannot enqueue sprite cloud without a material");
    return;
  }

  const Technique* technique = material->getActiveTechnique();
  if (!technique)
  {
    Log::writeError("Material %s has no active technique",
                    material->getName().c_str());
    return;
  }

  GL::VertexRange vertexRange;
  if (!renderer->allocateVertices(vertexRange, 4 * slots.size(), GL::Vertex2ft3fv::format))
    return;

  GL::IndexRange indexRange;
  if (!renderer->allocateIndices(indexRange, 6 * slots.size(), GL::IndexBuffer::USHORT))
    return;

  GL::VertexRangeLock<GL::Vertex2ft3fv> vertices(vertexRange);
  if (!vertices)
    return;

  GL::IndexRangeLock<unsigned short> indices(indexRange);
  if (!indices)
    return;

  const Vec3 camera = queue.getCamera().getTransform().position;

  /* TODO: Make this not use quads.
  realizeVertices(vertices, transform, camera);
  range.unlock();

  Vec3 position = Vec3::ZERO;
  transform.transformVector(position);

  Operation& operation = queue.createOperation();
  operation.vertexBuffer = range.getVertexBuffer();
  operation.start = range.getStart();
  operation.count = range.getCount();
  operation.renderMode = GL_QUADS;
  operation.transform = transform;
  operation.technique = technique;
  operation.distance = (position - camera).length();
  */
}

void SpriteCloud3::realizeVertices(GL::Vertex2ft3fv* vertices,
                                   const Transform3& transform,
                                   const Vec3& camera) const
{
  Transform3 inverse = transform;
  inverse.invert();

  Vec3 localCamera = camera;
  inverse.transformVector(localCamera);

  for (unsigned int i = 0;  i < slots.size();  i++)
  {
    realizeSpriteVertices(vertices + i * 4,
                          localCamera,
			  slots[i].position,
			  slots[i].size,
			  slots[i].angle,
			  type);
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

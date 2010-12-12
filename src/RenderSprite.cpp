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

#include <wendy/Config.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderPool.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderSprite.h>

#include <stdint.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

void rotateByZ(Vec2& vector, float angle)
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Vec2 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  vector = result;
}

void rotateByZ(Vec3& vector, float angle)
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Vec3 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  result.z = vector.z;
  vector = result;
}

void realizeSpriteVertices(Vertex2ft3fv* vertices,
			   const Vec3& cameraPosition,
                           const Vec3& spritePosition,
		           const Vec2& size,
		           float angle,
			   SpriteType3 type)
{
  Vec3 axisX, axisY;
  const Vec2 offset(size.x / 2.f, size.y / 2.f);

  if (type == STATIC_SPRITE)
  {
    axisX.set(offset.x, 0.f, 0.f);
    axisY.set(0.f, offset.y, 0.f);

    if (angle != 0.f)
    {
      rotateByZ(axisX, angle);
      rotateByZ(axisY, angle);
    }
  }
  else if (type == CYLINDRIC_SPRITE || type == SPHERICAL_SPRITE)
  {
    Vec3 axisZ(cameraPosition.x - spritePosition.x,
               0.f,
               cameraPosition.z - spritePosition.z);

    if (axisZ.lengthSquared() < 0.001f)
      axisZ.set(0.f, 0.f, 1.f);
    else
      axisZ.normalize();

    axisY.set(0.f, offset.y, 0.f);

    axisX = axisY.cross(axisZ);
    axisX.scaleTo(offset.x);

    if (type == SPHERICAL_SPRITE)
    {
      axisZ = (cameraPosition - spritePosition).normalized();

      axisY = axisZ.cross(axisX);
      axisY.scaleTo(offset.y);
    }

    if (angle != 0.f)
    {
      Quat rotZ;
      rotZ.setAxisRotation(axisZ, angle);
      rotZ.rotateVector(axisX);
      rotZ.rotateVector(axisY);
    }
  }
  else
    Log::writeError("Unknown sprite type %u", type);

  vertices[0].mapping.set(0.f, 0.f);
  vertices[0].position = spritePosition - axisX - axisY;
  vertices[1].mapping.set(1.f, 0.f);
  vertices[1].position = spritePosition + axisX - axisY;
  vertices[2].mapping.set(1.f, 1.f);
  vertices[2].position = spritePosition + axisX + axisY;
  vertices[3].mapping.set(0.f, 1.f);
  vertices[3].position = spritePosition - axisX + axisY;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Sprite2::Sprite2(void)
{
  setDefaults();
}

void Sprite2::render(GeometryPool& pool) const
{
  Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 4, Vertex2ft2fv::format))
    return;

  range.copyFrom(vertices);

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
}

void Sprite2::render(GeometryPool& pool, const Material& material) const
{
  Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 4, Vertex2ft2fv::format))
    return;

  range.copyFrom(vertices);

  const Technique* technique = material.getActiveTechnique();

  for (unsigned int pass = 0;  pass < technique->getPassCount();  pass++)
  {
    technique->applyPass(pass);
    pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
  }
}

void Sprite2::realizeVertices(Vertex2ft2fv* vertices) const
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
    rotateByZ(vertices[i].position, angle);
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
  if (!GeometryPool::getSingleton())
  {
    Log::writeError("Cannot enqueue sprite without a geometry pool");
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
    Log::writeError("Material \'%s\' has no active technique",
                    material->getPath().asString().c_str());
    return;
  }

  GL::VertexRange range;
  if (!GeometryPool::getSingleton()->allocateVertices(range, 4, Vertex2ft3fv::format))
    return;

  const Vec3 cameraPosition = queue.getCamera().getTransform().position;

  Vertex2ft3fv vertices[4];
  realizeVertices(vertices, transform, cameraPosition);
  range.copyFrom(vertices);

  Vec3 position = Vec3::ZERO;
  transform.transformVector(position);

  Operation operation;
  operation.range = GL::PrimitiveRange(GL::TRIANGLE_FAN, range);
  operation.transform = transform;
  operation.technique = technique;
  operation.distance = (position - cameraPosition).length();
  queue.addOperation(operation);
}

void Sprite3::realizeVertices(Vertex2ft3fv* vertices,
                              const Transform3& transform,
                              const Vec3& cameraPosition) const
{
  Transform3 inverse = transform;
  inverse.invert();

  Vec3 localCamera = cameraPosition;
  inverse.transformVector(localCamera);

  realizeSpriteVertices(vertices, cameraPosition, position, size, angle, type);
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

  GeometryPool* pool = GeometryPool::getSingleton();
  if (!pool)
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
    Log::writeError("Material \'%s\' has no active technique",
                    material->getPath().asString().c_str());
    return;
  }

  GL::VertexRange vertexRange;
  if (!pool->allocateVertices(vertexRange, 4 * slots.size(), Vertex2ft3fv::format))
    return;

  GL::IndexRange indexRange;
  if (!pool->allocateIndices(indexRange, 6 * slots.size(), GL::IndexBuffer::UINT16))
    return;

  const Vec3 cameraPosition = queue.getCamera().getTransform().position;

  // Realize sprite vertices
  {
    GL::VertexRangeLock<Vertex2ft3fv> vertices(vertexRange);
    if (!vertices)
      return;

    realizeVertices(vertices, transform, cameraPosition);
  }

  // Realize sprite indices
  {
    GL::IndexRangeLock<uint16_t> indices(indexRange);
    if (!indices)
      return;

    unsigned int base = vertexRange.getStart();

    for (unsigned int i = 0;  i < slots.size();  i++)
    {
      indices[i * 6 + 0] = base + i * 4 + 0;
      indices[i * 6 + 1] = base + i * 4 + 1;
      indices[i * 6 + 2] = base + i * 4 + 2;

      indices[i * 6 + 3] = base + i * 4 + 2;
      indices[i * 6 + 4] = base + i * 4 + 3;
      indices[i * 6 + 5] = base + i * 4 + 0;
    }
  }

  Vec3 position = Vec3::ZERO;
  transform.transformVector(position);

  Operation operation;
  operation.technique = technique;
  operation.distance = (position - cameraPosition).length();
  operation.transform = transform;
  operation.range = GL::PrimitiveRange(GL::TRIANGLE_LIST,
                                       *vertexRange.getVertexBuffer(),
                                       indexRange);

  queue.addOperation(operation);
}

void SpriteCloud3::realizeVertices(Vertex2ft3fv* vertices,
                                   const Transform3& transform,
                                   const Vec3& cameraPosition) const
{
  Transform3 inverse = transform;
  inverse.invert();

  Vec3 localCamera = cameraPosition;
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

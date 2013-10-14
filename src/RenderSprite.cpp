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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Transform.hpp>
#include <wendy/AABB.hpp>
#include <wendy/Plane.hpp>
#include <wendy/Sphere.hpp>
#include <wendy/Frustum.hpp>
#include <wendy/Camera.hpp>

#include <wendy/RenderPool.hpp>
#include <wendy/RenderState.hpp>
#include <wendy/RenderMaterial.hpp>
#include <wendy/RenderScene.hpp>
#include <wendy/RenderSprite.hpp>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

void realizeSpriteVertices(Vertex2ft3fv* vertices,
                           const vec3& cameraPosition,
                           const vec3& spritePosition,
                           const vec2& size,
                           float angle,
                           SpriteType3 type)
{
  vec3 axisX, axisY;
  const vec2 offset(size.x / 2.f, size.y / 2.f);

  if (type == STATIC_SPRITE)
  {
    axisX = vec3(offset.x, 0.f, 0.f);
    axisY = vec3(0.f, offset.y, 0.f);

    if (angle != 0.f)
    {
      axisX = rotateZ(axisX, degrees(angle));
      axisY = rotateZ(axisY, degrees(angle));
    }
  }
  else if (type == CYLINDRIC_SPRITE || type == SPHERICAL_SPRITE)
  {
    vec3 axisZ(cameraPosition.x - spritePosition.x,
               0.f,
               cameraPosition.z - spritePosition.z);

    if (length2(axisZ) < 0.001f)
      axisZ = vec3(0.f, 0.f, 1.f);
    else
      axisZ = normalize(axisZ);

    axisY = vec3(0.f, offset.y, 0.f);
    axisX = normalize(cross(axisY, axisZ)) * offset.x;

    if (type == SPHERICAL_SPRITE)
    {
      axisZ = normalize(cameraPosition - spritePosition);
      axisY = normalize(cross(axisZ, axisX)) * offset.y;
    }

    if (angle != 0.f)
    {
      quat rotZ = angleAxis(degrees(angle), axisZ);
      axisX = rotate(rotZ, axisX);
      axisY = rotate(rotZ, axisY);
    }
  }
  else
    logError("Unknown sprite type %u", type);

  vertices[0].texcoord = vec2(0.f, 0.f);
  vertices[0].position = spritePosition - axisX - axisY;
  vertices[1].texcoord = vec2(1.f, 0.f);
  vertices[1].position = spritePosition + axisX - axisY;
  vertices[2].texcoord = vec2(1.f, 1.f);
  vertices[2].position = spritePosition + axisX + axisY;
  vertices[3].texcoord = vec2(0.f, 1.f);
  vertices[3].position = spritePosition - axisX + axisY;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Sprite2::Sprite2():
  texArea(vec2(0.f), vec2(1.f)),
  position(0.f),
  size(1.f),
  angle(0.f)
{
}

void Sprite2::render(VertexPool& pool) const
{
  Vertex2ft2fv vertices[4];
  realizeVertices(vertices);

  GL::VertexRange range = pool.allocate(4, Vertex2ft2fv::format);
  if (range.isEmpty())
    return;

  range.copyFrom(vertices);

  pool.context().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
}

void Sprite2::realizeVertices(Vertex2ft2fv* vertices) const
{
  const vec2 offset(size.x / 2.f, size.y / 2.f);

  vertices[0].texcoord = vec2(texArea.position.x + texArea.size.x, texArea.position.y + texArea.size.y);
  vertices[1].texcoord = vec2(texArea.position.x, texArea.position.y + texArea.size.y);
  vertices[2].texcoord = vec2(texArea.position.x, texArea.position.y);
  vertices[3].texcoord = vec2(texArea.position.x + texArea.size.x, texArea.position.y);

  vertices[0].position = vec2( offset.x,  offset.y);
  vertices[1].position = vec2(-offset.x,  offset.y);
  vertices[2].position = vec2(-offset.x, -offset.y);
  vertices[3].position = vec2( offset.x, -offset.y);

  for (uint i = 0;  i < 4;  i++)
  {
    vertices[i].position = rotate(vertices[i].position, degrees(angle));
    vertices[i].position += position;
  }
}

///////////////////////////////////////////////////////////////////////

Sprite3::Sprite3():
  size(1.f),
  angle(0.f),
  type(STATIC_SPRITE),
  material(nullptr)
{
}

void Sprite3::enqueue(Scene& scene,
                      const Camera& camera,
                      const Transform3& transform) const
{
  if (!material)
  {
    logError("Cannot enqueue sprite without a material");
    return;
  }

  GL::VertexRange range = scene.vertexPool().allocate(4, Vertex2ft3fv::format);
  if (range.isEmpty())
    return;

  const vec3 cameraPos = camera.transform().position;
  const vec3 spritePos = transform.position;

  Vertex2ft3fv vertices[4];
  realizeSpriteVertices(vertices, cameraPos, spritePos, size, angle, type);
  range.copyFrom(vertices);

  scene.createOperations(Transform3::IDENTITY,
                         GL::PrimitiveRange(GL::TRIANGLE_FAN, range),
                         *material,
                         camera.normalizedDepth(spritePos));
}

Sphere Sprite3::bounds() const
{
  return Sphere(vec3(0.f), length(size) / 2.f);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

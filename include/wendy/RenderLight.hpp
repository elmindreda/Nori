///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2008 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_RENDERLIGHT_HPP
#define WENDY_RENDERLIGHT_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class Light : public RefObject
{
public:
  enum Type
  {
    DIRECTIONAL,
    POINT,
    SPOTLIGHT
  };
  Light();
  Type type() const { return m_type; }
  void setType(Type newType);
  float radius() const { return m_radius; }
  void setRadius(float newRadius);
  const vec3& color() const { return m_color; }
  void setColor(const vec3& newColor);
  const vec3& position() const { return m_position; }
  void setPosition(const vec3& newPosition);
  const vec3& direction() const { return m_direction; }
  void setDirection(const vec3& newDirection);
private:
  Type m_type;
  float m_radius;
  vec3 m_color;
  vec3 m_position;
  vec3 m_direction;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
typedef std::vector<Ref<Light>> LightList;

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERLIGHT_HPP*/
///////////////////////////////////////////////////////////////////////

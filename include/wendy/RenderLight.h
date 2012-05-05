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
#ifndef WENDY_RENDERLIGHT_H
#define WENDY_RENDERLIGHT_H
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
  Type getType() const;
  void setType(Type newType);
  float getRadius() const;
  void setRadius(float newRadius);
  const vec3& getColor() const;
  void setColor(const vec3& newColor);
  const vec3& getPosition() const;
  void setPosition(const vec3& newPosition);
  const vec3& getDirection() const;
  void setDirection(const vec3& newDirection);
private:
  Type type;
  float radius;
  vec3 color;
  vec3 position;
  vec3 direction;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
typedef std::vector<Ref<Light>> LightList;

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERLIGHT_H*/
///////////////////////////////////////////////////////////////////////

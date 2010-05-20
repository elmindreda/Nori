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

#include <wendy/Sphere.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

class Light : public RefObject
{
public:
  enum Type
  {
    DIRECTIONAL,
    POINT,
    SPOTLIGHT,
  };
  Light(void);
  Type getType(void) const;
  void setType(Type newType);
  const ColorRGB& getColor(void) const;
  void setColor(const ColorRGB& newColor);
  const Vec3& getPosition(void) const;
  void setPosition(const Vec3& newPosition);
  const Vec3& getDirection(void) const;
  void setDirection(const Vec3& newDirection);
  float getConstantAttenuation(void) const;
  void setConstantAttenuation(float newValue);
  float getLinearAttenuation(void) const;
  void setLinearAttenuation(float newValue);
  float getQuadraticAttenuation(void) const;
  void setQuadraticAttenuation(float newValue);
  const Sphere& getBounds(void) const;
  void setBounds(const Sphere& newBounds);
private:
  Type type;
  ColorRGB color;
  Vec3 position;
  Vec3 direction;
  float constant;
  float linear;
  float quadratic;
  Sphere bounds;
};

///////////////////////////////////////////////////////////////////////

typedef Ref<Light> LightRef;

///////////////////////////////////////////////////////////////////////

class LightState
{
public:
  void attachLight(Light& light);
  void detachLight(Light& light);
  void detachLights(void);
  unsigned int getLightCount(void) const;
  Light& getLight(unsigned int index) const;
private:
  typedef std::vector<LightRef> List;
  List lights;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERLIGHT_H*/
///////////////////////////////////////////////////////////////////////

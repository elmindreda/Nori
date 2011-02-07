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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Color.h>

#include <wendy/RenderLight.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

Light::Light(void):
  type(DIRECTIONAL),
  radius(10.f),
  color(ColorRGB::WHITE),
  position(Vec3::ZERO),
  direction(0.f, 0.f, -1.f)
{
}

Light::Type Light::getType(void) const
{
  return type;
}

void Light::setType(Type newType)
{
  type = newType;
}

float Light::getRadius(void) const
{
  return radius;
}

void Light::setRadius(float newRadius)
{
  radius = newRadius;
}

const ColorRGB& Light::getColor(void) const
{
  return color;
}

void Light::setColor(const ColorRGB& newColor)
{
  color = newColor;
}

const Vec3& Light::getPosition(void) const
{
  return position;
}

void Light::setPosition(const Vec3& newPosition)
{
  position = newPosition;
}

const Vec3& Light::getDirection(void) const
{
  return direction;
}

void Light::setDirection(const Vec3& newDirection)
{
  direction = newDirection;
}

///////////////////////////////////////////////////////////////////////

LightState::LightState(void):
  ambient(ColorRGB::BLACK)
{
}

void LightState::attachLight(Light& light)
{
  List::const_iterator i = std::find(lights.begin(), lights.end(), &light);
  if (i != lights.end())
    return;

  lights.push_back(&light);
}

void LightState::detachLight(Light& light)
{
  List::iterator i = std::find(lights.begin(), lights.end(), &light);
  if (i != lights.end())
    lights.erase(i);
}

void LightState::detachLights(void)
{
  lights.clear();
}

unsigned int LightState::getLightCount(void) const
{
  return lights.size();
}

Light& LightState::getLight(unsigned int index) const
{
  return *lights[index];
}

const ColorRGB& LightState::getAmbientIntensity(void) const
{
  return ambient;
}

void LightState::setAmbientIntensity(const ColorRGB& newIntensity)
{
  ambient = newIntensity;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

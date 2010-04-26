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
#include <wendy/Vector.h>
#include <wendy/Rectangle.h>
#include <wendy/Sphere.h>
#include <wendy/Pixel.h>
#include <wendy/Signal.h>

#include <wendy/GLContext.h>

#include <wendy/RenderLight.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

Light::Light(void):
  intensity(ColorRGB::WHITE),
  position(Vec3::ZERO),
  bounds(Vec3::ZERO, 1.f)
{
}

const ColorRGB& Light::getIntensity(void) const
{
  return intensity;
}

void Light::setIntensity(const ColorRGB& newIntensity)
{
  intensity = newIntensity;
}

const Vec3& Light::getPosition(void) const
{
  return position;
}

void Light::setPosition(const Vec3& newPosition)
{
  position = newPosition;
}

const Sphere& Light::getBounds(void) const
{
  return bounds;
}

void Light::setBounds(const Sphere& newBounds)
{
  bounds = newBounds;
}

///////////////////////////////////////////////////////////////////////

LightState::LightState(void)
{
  static bool initialized = false;

  if (!initialized)
  {
    GL::Context* context = GL::Context::get();
    if (context)
    {
      context->getDestroySignal().connect(onContextDestroy);
      initialized = true;
    }
  }
}

void LightState::apply(void) const
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

const LightState& LightState::getCurrent(void)
{
  return current;
}

void LightState::onContextDestroy(void)
{
  current.detachLights();
}

LightState LightState::current;

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

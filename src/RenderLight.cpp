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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>

#include <wendy/RenderLight.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

Light::Light():
  m_type(DIRECTIONAL),
  m_radius(10.f),
  m_color(1.f),
  m_direction(0.f, 0.f, -1.f)
{
}

void Light::setType(Type newType)
{
  m_type = newType;
}

void Light::setRadius(float newRadius)
{
  m_radius = newRadius;
}

void Light::setColor(const vec3& newColor)
{
  m_color = newColor;
}

void Light::setPosition(const vec3& newPosition)
{
  m_position = newPosition;
}

void Light::setDirection(const vec3& newDirection)
{
  m_direction = newDirection;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

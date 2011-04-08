///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Random.h>

#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

template <>
Random<float>::Random(void):
  min(0.f),
  max(1.f)
{
}

template <>
Random<float>::Random(const float& initMin, const float& initMax):
  min(initMin),
  max(initMax)
{
}

template <>
float Random<float>::operator () (void) const
{
  return min + (max - min) * normalizedRandom();
}

template <>
void Random<float>::set(const float& newMin, const float& newMax)
{
  min = newMin;
  max = newMax;
}

///////////////////////////////////////////////////////////////////////

template <>
Random<vec2>::Random(void):
  min(0.f, 0.f),
  max(1.f, 1.f)
{
}

template <>
Random<vec2>::Random(const vec2& initMin, const vec2& initMax):
  min(initMin),
  max(initMax)
{
}

template <>
vec2 Random<vec2>::operator () (void) const
{
  vec2 result;
  result.x = min.x + (max.x - min.x) * normalizedRandom();
  result.y = min.y + (max.y - min.y) * normalizedRandom();
  return result;
}

template <>
void Random<vec2>::set(const vec2& newMin, const vec2& newMax)
{
  min = newMin;
  max = newMax;
}

///////////////////////////////////////////////////////////////////////

template <>
Random<vec3>::Random(void):
  min(0.f, 0.f, 0.f),
  max(1.f, 1.f, 1.f)
{
}

template <>
Random<vec3>::Random(const vec3& initMin, const vec3& initMax):
  min(initMin),
  max(initMax)
{
}

template <>
vec3 Random<vec3>::operator () (void) const
{
  vec3 result;
  result.x = min.x + (max.x - min.x) * normalizedRandom();
  result.y = min.y + (max.y - min.y) * normalizedRandom();
  result.z = min.z + (max.z - min.z) * normalizedRandom();
  return result;
}

template <>
void Random<vec3>::set(const vec3& newMin, const vec3& newMax)
{
  min = newMin;
  max = newMax;
}

///////////////////////////////////////////////////////////////////////

float normalizedRandom(void)
{
  return std::rand() / (float) RAND_MAX;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

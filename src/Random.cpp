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
#include <wendy/Color.h>
#include <wendy/Vector.h>
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
float Random<float>::generate(void) const
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
Random<Vec2>::Random(void):
  min(0.f, 0.f),
  max(1.f, 1.f)
{
}

template <>
Random<Vec2>::Random(const Vec2& initMin, const Vec2& initMax):
  min(initMin),
  max(initMax)
{
}

template <>
Vec2 Random<Vec2>::generate(void) const
{
  Vec2 result;
  result.x = min.x + (max.x - min.x) * normalizedRandom();
  result.y = min.y + (max.y - min.y) * normalizedRandom();
  return result;
}

template <>
void Random<Vec2>::set(const Vec2& newMin, const Vec2& newMax)
{
  min = newMin;
  max = newMax;
}

///////////////////////////////////////////////////////////////////////

template <>
Random<Vec3>::Random(void):
  min(0.f, 0.f, 0.f),
  max(1.f, 1.f, 1.f)
{
}

template <>
Random<Vec3>::Random(const Vec3& initMin, const Vec3& initMax):
  min(initMin),
  max(initMax)
{
}

template <>
Vec3 Random<Vec3>::generate(void) const
{
  Vec3 result;
  result.x = min.x + (max.x - min.x) * normalizedRandom();
  result.y = min.y + (max.y - min.y) * normalizedRandom();
  result.z = min.z + (max.z - min.z) * normalizedRandom();
  return result;
}

template <>
void Random<Vec3>::set(const Vec3& newMin, const Vec3& newMax)
{
  min = newMin;
  max = newMax;
}

///////////////////////////////////////////////////////////////////////

template <>
Random<ColorRGB>::Random(void):
  min(0.f, 0.f, 0.f),
  max(1.f, 1.f, 1.f)
{
}

template <>
Random<ColorRGB>::Random(const ColorRGB& initMin, const ColorRGB& initMax):
  min(initMin),
  max(initMax)
{
}

template <>
ColorRGB Random<ColorRGB>::generate(void) const
{
  ColorRGB result;
  result.r = min.r + (max.r - min.r) * normalizedRandom();
  result.g = min.g + (max.g - min.g) * normalizedRandom();
  result.b = min.b + (max.b - min.b) * normalizedRandom();
  return result;
}

template <>
void Random<ColorRGB>::set(const ColorRGB& newMin, const ColorRGB& newMax)
{
  min = newMin;
  max = newMax;
}

///////////////////////////////////////////////////////////////////////

template <>
Random<ColorRGBA>::Random(void):
  min(0.f, 0.f, 0.f, 0.f),
  max(1.f, 1.f, 1.f, 1.f)
{
}

template <>
Random<ColorRGBA>::Random(const ColorRGBA& initMin, const ColorRGBA& initMax):
  min(initMin),
  max(initMax)
{
}

template <>
ColorRGBA Random<ColorRGBA>::generate(void) const
{
  ColorRGBA result;
  result.r = min.r + (max.r - min.r) * normalizedRandom();
  result.g = min.g + (max.g - min.g) * normalizedRandom();
  result.b = min.b + (max.b - min.b) * normalizedRandom();
  result.a = min.a + (max.a - min.a) * normalizedRandom();
  return result;
}

template <>
void Random<ColorRGBA>::set(const ColorRGBA& newMin, const ColorRGBA& newMax)
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

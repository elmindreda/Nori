///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>
#include <nori/Core.hpp>
#include <nori/Rect.hpp>

#include <sstream>
#include <utility>

namespace nori
{

Rect::Rect(vec2 position, vec2 size):
  position(position),
  size(size)
{
}

Rect::Rect(float x, float y, float width, float height):
  position(x, y),
  size(width, height)
{
}

Rect::Rect(const Recti& source):
  position(source.position),
  size(source.size)
{
}

bool Rect::contains(vec2 point) const
{
  float minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  if (point.x < minX || point.x > maxX)
    return false;

  if (point.y < minY || point.y > maxY)
    return false;

  return true;
}

bool Rect::contains(const Rect& other) const
{
  float minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMinX || maxX < otherMaxX)
    return false;

  if (minY > otherMinY || maxY < otherMaxY)
    return false;

  return true;
}

bool Rect::intersects(const Rect& other) const
{
  float minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  return true;
}

bool Rect::clipBy(const Rect& other)
{
  float minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  setBounds(max(minY, otherMinX), max(minY, otherMinY),
            min(maxX, otherMaxX), min(maxY, otherMaxY));

  return true;
}

void Rect::envelop(vec2 other)
{
  float minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  setBounds(min(minX, other.x), min(minY, other.y),
            max(maxX, other.x), max(maxY, other.y));
}

void Rect::envelop(const Rect& other)
{
  float minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  setBounds(min(minX, otherMinX), min(minY, otherMinY),
            max(maxX, otherMaxX), min(maxY, otherMaxY));
}

void Rect::normalize()
{
  position += min(size, vec2(0.f));
  size = abs(size);
}

bool Rect::operator == (const Rect& other) const
{
  return position == other.position && size == other.size;
}

bool Rect::operator != (const Rect& other) const
{
  return position != other.position || size != other.size;
}

void Rect::setCenter(vec2 newCenter)
{
  position = newCenter - size / 2.f;
}

void Rect::bounds(float& minX, float& minY, float& maxX, float& maxY) const
{
  minX = position.x + min(size.x, 0.f);
  minY = position.y + min(size.y, 0.f);
  maxX = minX + abs(size.x);
  maxY = minY + abs(size.y);
}

void Rect::setBounds(float minX, float minY, float maxX, float maxY)
{
  position = vec2(minX, minY);
  size = vec2(maxX - minX, maxY - minY);
}

void Rect::set(vec2 newPosition, vec2 newSize)
{
  position = newPosition;
  size = newSize;
}

void Rect::set(float x, float y, float width, float height)
{
  position = vec2(x, y);
  size = vec2(width, height);
}

Recti::Recti(ivec2 position, ivec2 size):
  position(position),
  size(size)
{
}

Recti::Recti(int x, int y, int width, int height):
  position(x, y),
  size(width, height)
{
}

Recti::Recti(const Rect& source):
  position(source.position),
  size(source.size)
{
}

bool Recti::contains(ivec2 point) const
{
  int minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  if (point.x < minX || point.x > maxX)
    return false;

  if (point.y < minY || point.y > maxY)
    return false;

  return true;
}

bool Recti::contains(const Recti& other) const
{
  int minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMinX || maxX < otherMaxX)
    return false;

  if (minY > otherMinY || maxY < otherMaxY)
    return false;

  return true;
}

bool Recti::intersects(const Recti& other) const
{
  int minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  return true;
}

bool Recti::clipBy(const Recti& other)
{
  int minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  setBounds(max(minY, otherMinX), max(minY, otherMinY),
            min(maxX, otherMaxX), min(maxY, otherMaxY));

  return true;
}

void Recti::envelop(const Recti& other)
{
  int minX, minY, maxX, maxY;
  bounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.bounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  setBounds(min(minX, otherMinX), min(minY, otherMinY),
            max(maxX, otherMaxX), min(maxY, otherMaxY));
}

void Recti::normalize()
{
  position += min(size, ivec2(0));
  size = abs(size);
}

bool Recti::operator == (const Recti& other) const
{
  return position == other.position && size == other.size;
}

bool Recti::operator != (const Recti& other) const
{
  return position != other.position || size != other.size;
}

void Recti::setCenter(ivec2 newCenter)
{
  position = newCenter - size / 2;
}

void Recti::bounds(int& minX, int& minY, int& maxX, int& maxY) const
{
  minX = position.x + min(size.x, 0);
  minY = position.y + min(size.y, 0);
  maxX = minX + abs(size.x);
  maxY = minY + abs(size.y);
}

void Recti::setBounds(int minX, int minY, int maxX, int maxY)
{
  position = ivec2(minX, minY);
  size = ivec2(maxX - minX, maxY - minY);
}

void Recti::set(ivec2 newPosition, ivec2 newSize)
{
  position = newPosition;
  size = newSize;
}

void Recti::set(int x, int y, int width, int height)
{
  position = ivec2(x, y);
  size = ivec2(width, height);
}

Rect rectCast(const std::string& string)
{
  std::istringstream stream(string, std::ios::in);

  Rect result;
  stream >> result.position.x >> result.position.y;
  stream >> result.size.x >> result.size.y;
  return result;
}

Recti rectiCast(const std::string& string)
{
  std::istringstream stream(string, std::ios::in);

  Recti result;
  stream >> result.position.x >> result.position.y;
  stream >> result.size.x >> result.size.y;
  return result;
}

std::string stringCast(const Rect& v)
{
  std::ostringstream stream;

  stream << v.position.x << ' ' << v.position.y << ' '
         << v.size.x << ' ' << v.size.y;

  return stream.str();
}

std::string stringCast(const Recti& v)
{
  std::ostringstream stream;

  stream << v.position.x << ' ' << v.position.y << ' '
         << v.size.x << ' ' << v.size.y;

  return stream.str();
}

} /*namespace nori*/


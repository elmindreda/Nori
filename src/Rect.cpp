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
#include <wendy/Rect.h>

#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Rect::Rect()
{
}

Rect::Rect(const vec2& initPosition, const vec2& initSize):
  position(initPosition),
  size(initSize)
{
}

Rect::Rect(float x, float y, float width, float height):
  position(x, y),
  size(width, height)
{
}

bool Rect::contains(const vec2& point) const
{
  float minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  if (point.x < minX || point.x > maxX)
    return false;

  if (point.y < minY || point.y > maxY)
    return false;

  return true;
}

bool Rect::contains(const Rect& other) const
{
  float minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMinX || maxX < otherMaxX)
    return false;

  if (minY > otherMinY || maxY < otherMaxY)
    return false;

  return true;
}

bool Rect::intersects(const Rect& other) const
{
  float minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  return true;
}

bool Rect::clipBy(const Rect& other)
{
  float minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  if (minX < otherMinX)
    minX = otherMinX;
  if (minY < otherMinY)
    minY = otherMinY;
  if (maxX > otherMaxX)
    maxX = otherMaxX;
  if (maxY > otherMaxY)
    maxY = otherMaxY;

  setBounds(minX, minY, maxX, maxY);
  return true;
}

void Rect::envelop(const vec2& other)
{
  float minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  minX = min(minX, other.x);
  minY = min(minY, other.y);
  maxX = max(maxX, other.x);
  maxY = max(maxY, other.y);

  setBounds(minX, minY, maxX, maxY);
}

void Rect::envelop(const Rect& other)
{
  float minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  float otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMinX)
    minX = otherMinX;
  if (minY > otherMinY)
    minY = otherMinY;
  if (maxX < otherMaxX)
    maxX = otherMaxX;
  if (maxY < otherMaxY)
    maxY = otherMaxY;

  setBounds(minX, minY, maxX, maxY);
}

void Rect::normalize()
{
  if (size.x < 0.f)
  {
    position.x += size.x;
    size.x = -size.x;
  }

  if (size.y < 0.f)
  {
    position.y += size.y;
    size.y = -size.y;
  }
}

bool Rect::operator == (const Rect& other) const
{
  return position == other.position && size == other.size;
}

bool Rect::operator != (const Rect& other) const
{
  return position != other.position || size != other.size;
}

Rect Rect::operator + (const vec2& offset) const
{
  return Rect(position + offset, size);
}

Rect Rect::operator - (const vec2& offset) const
{
  return Rect(position - offset, size);
}

Rect Rect::operator * (const vec2& scale) const
{
  return Rect(position * scale, size * scale);
}

Rect& Rect::operator += (const vec2& offset)
{
  position += offset;
  return *this;
}

Rect& Rect::operator -= (const vec2& offset)
{
  position -= offset;
  return *this;
}

Rect& Rect::operator *= (const vec2& scale)
{
  position *= scale;
  size *= scale;
  return *this;
}

vec2 Rect::getCenter() const
{
  return position + size / 2.f;
}

void Rect::setCenter(const vec2& newCenter)
{
  position = newCenter - size / 2.f;
}

void Rect::getBounds(float& minX, float& minY, float& maxX, float& maxY) const
{
  minX = position.x;
  minY = position.y;

  maxX = position.x + size.x;
  maxY = position.y + size.y;

  if (minX > maxX)
  {
    float quux = minX;
    minX = maxX;
    maxX = quux;
  }

  if (minY > maxY)
  {
    float quux = minY;
    minY = maxY;
    maxY = quux;
  }
}

void Rect::setBounds(float minX, float minY, float maxX, float maxY)
{
  position = vec2(minX, minY);
  size = vec2(maxX - minX, maxY - minY);
}

void Rect::set(const vec2& newPosition, const vec2& newSize)
{
  position = newPosition;
  size = newSize;
}

void Rect::set(float x, float y, float width, float height)
{
  position = vec2(x, y);
  size = vec2(width, height);
}

///////////////////////////////////////////////////////////////////////

Recti::Recti()
{
}

Recti::Recti(const ivec2& initPosition, const ivec2& initSize):
  position(initPosition),
  size(initSize)
{
}

Recti::Recti(int x, int y, int width, int height):
  position(x, y),
  size(width, height)
{
}

bool Recti::contains(const ivec2& point) const
{
  int minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  if (point.x < minX || point.x > maxX)
    return false;

  if (point.y < minY || point.y > maxY)
    return false;

  return true;
}

bool Recti::contains(const Recti& other) const
{
  int minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMinX || maxX < otherMaxX)
    return false;

  if (minY > otherMinY || maxY < otherMaxY)
    return false;

  return true;
}

bool Recti::intersects(const Recti& other) const
{
  int minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  return true;
}

bool Recti::clipBy(const Recti& other)
{
  int minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  if (minX < otherMinX)
    minX = otherMinX;
  if (minY < otherMinY)
    minY = otherMinY;
  if (maxX > otherMaxX)
    maxX = otherMaxX;
  if (maxY > otherMaxY)
    maxY = otherMaxY;

  setBounds(minX, minY, maxX, maxY);
  return true;
}

void Recti::envelop(const Recti& other)
{
  int minX, minY, maxX, maxY;
  getBounds(minX, minY, maxX, maxY);

  int otherMinX, otherMinY, otherMaxX, otherMaxY;
  other.getBounds(otherMinX, otherMinY, otherMaxX, otherMaxY);

  if (minX > otherMinX)
    minX = otherMinX;
  if (minY > otherMinY)
    minY = otherMinY;
  if (maxX < otherMaxX)
    maxX = otherMaxX;
  if (maxY < otherMaxY)
    maxY = otherMaxY;

  setBounds(minX, minY, maxX, maxY);
}

void Recti::normalize()
{
  if (size.x < 0)
  {
    position.x += size.x;
    size.x = -size.x;
  }

  if (size.y < 0)
  {
    position.y += size.y;
    size.y = -size.y;
  }
}

bool Recti::operator == (const Recti& other) const
{
  return position == other.position && size == other.size;
}

bool Recti::operator != (const Recti& other) const
{
  return position != other.position || size != other.size;
}

Recti Recti::operator + (const ivec2& offset) const
{
  return Recti(position + offset, size);
}

Recti Recti::operator - (const ivec2& offset) const
{
  return Recti(position - offset, size);
}

Recti Recti::operator * (const ivec2& scale) const
{
  return Recti(position * scale, size * scale);
}

Recti& Recti::operator += (const ivec2& offset)
{
  position += offset;
  return *this;
}

Recti& Recti::operator -= (const ivec2& offset)
{
  position -= offset;
  return *this;
}

Recti& Recti::operator *= (const ivec2& scale)
{
  position *= scale;
  size *= scale;
  return *this;
}

ivec2 Recti::getCenter() const
{
  return position + size / 2;
}

void Recti::setCenter(const ivec2& newCenter)
{
  position = newCenter - size / 2;
}

void Recti::getBounds(int& minX, int& minY, int& maxX, int& maxY) const
{
  minX = position.x;
  minY = position.y;

  maxX = position.x + size.x;
  maxY = position.y + size.y;

  if (minX > maxX)
  {
    int quux = minX;
    minX = maxX;
    maxX = quux;
  }

  if (minY > maxY)
  {
    int quux = minY;
    minY = maxY;
    maxY = quux;
  }
}

void Recti::setBounds(int minX, int minY, int maxX, int maxY)
{
  position = ivec2(minX, minY);
  size = ivec2(maxX - minX, maxY - minY);
}

void Recti::set(const ivec2& newPosition, const ivec2& newSize)
{
  position = newPosition;
  size = newSize;
}

void Recti::set(int x, int y, int width, int height)
{
  position = ivec2(x, y);
  size = ivec2(width, height);
}

///////////////////////////////////////////////////////////////////////

Rect rectCast(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  Rect result;
  stream >> result.position.x >> result.position.y;
  stream >> result.size.x >> result.size.y;
  return result;
}

Recti rectiCast(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  Recti result;
  stream >> result.position.x >> result.position.y;
  stream >> result.size.x >> result.size.y;
  return result;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

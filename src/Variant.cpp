///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Variant.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Variant::Variant(void)
{
}

Variant::Variant(const String& initValue):
  value(initValue)
{
}

float Variant::asFloat(void) const
{
  return convertToFloat(value);
}

void Variant::setFloatValue(float newValue)
{
  convertToString(value, newValue);
}

int Variant::asInteger(void) const
{
  return convertToInteger(value);
}

void Variant::setIntegerValue(int newValue)
{
  convertToString(value, newValue);
}

bool Variant::asBoolean(void) const
{
  return convertToBoolean(value);
}

void Variant::setBooleanValue(bool newValue)
{
  convertToString(value, newValue);
}

const String& Variant::asString(void) const
{
  return value;
}

void Variant::setStringValue(const String& newValue)
{
  value = newValue;
}

float Variant::convertToFloat(const String& value)
{
  return strtof(value.c_str(), NULL);
}

int Variant::convertToInteger(const String& value)
{
  return std::strtol(value.c_str(), NULL, 0);
}

bool Variant::convertToBoolean(const String& value)
{
  if (value == "true")
    return true;

  return convertToInteger(value) ? true : false;
}

void Variant::convertToString(String& result, float value)
{
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%f", value);
  buffer[sizeof(buffer) - 1] = '\0';
  result = buffer;
}

void Variant::convertToString(String& result, int value)
{
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%i", value);
  buffer[sizeof(buffer) - 1] = '\0';
  result = buffer;
}

void Variant::convertToString(String& result, bool value)
{
  if (value)
    result = "true";
  else
    result = "false";
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

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
#ifndef WENDY_VARIANT_H
#define WENDY_VARIANT_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Variant
{
public:
  Variant(void);
  Variant(const String& value);
  float asFloat(void) const;
  void setFloatValue(float newValue);
  int asInteger(void) const;
  void setIntegerValue(int newValue);
  bool asBoolean(void) const;
  void setBooleanValue(bool newValue);
  const String& asString(void) const;
  void setStringValue(const String& newValue);
  static float convertToFloat(const String& value);
  static int convertToInteger(const String& value);
  static bool convertToBoolean(const String& value);
  static void convertToString(String& result, float value);
  static void convertToString(String& result, int value);
  static void convertToString(String& result, bool value);
private:
  String value;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_VARIANT_H*/
///////////////////////////////////////////////////////////////////////

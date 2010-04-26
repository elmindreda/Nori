///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_COLOR_H
#define WENDY_COLOR_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class ColorHSL;
class ColorHSV;
class ColorRGBA;

///////////////////////////////////////////////////////////////////////

/*! @brief RGB color value.
 */
class ColorRGB
{
public:
  /*! Default constructor.
   *  @remarks This does not initialize the object's fields.
   */
  ColorRGB(void);
  /*! Creates a color object with the specified values.
   */
  ColorRGB(float sr, float sg, float sb);
  ColorRGB(const ColorRGBA& rgba);
  ColorRGB(const ColorHSL& hsl);
  ColorRGB(const ColorHSV& hsv);
  explicit ColorRGB(const String& string);
  /*! Clamps each component to the range [0, 1].
   */
  void clamp(void);
  ColorRGB min(const ColorRGB& color) const;
  ColorRGB max(const ColorRGB& color) const;
  String asString(void) const;
  operator float* (void);
  operator const float* (void) const;
  ColorRGB operator - (void) const;
  ColorRGB operator + (float value) const;
  ColorRGB operator - (float value) const;
  ColorRGB operator * (float value) const;
  ColorRGB operator / (float value) const;
  ColorRGB operator += (float value);
  ColorRGB& operator -= (float value);
  ColorRGB& operator *= (float value);
  ColorRGB& operator /= (float value);
  ColorRGB operator + (const ColorRGB& color) const;
  ColorRGB operator - (const ColorRGB& color) const;
  ColorRGB operator * (const ColorRGB& color) const;
  ColorRGB operator / (const ColorRGB& color) const;
  ColorRGB operator += (const ColorRGB& color);
  ColorRGB& operator -= (const ColorRGB& color);
  ColorRGB& operator *= (const ColorRGB& color);
  ColorRGB& operator /= (const ColorRGB& color);
  bool operator == (const ColorRGB& color) const;
  bool operator != (const ColorRGB& color) const;
  float getLength(void) const;
  float getSquaredLength(void) const;
  void setDefaults(void);
  void set(float sr, float sg, float sb);
  float r;
  float g;
  float b;
  static const ColorRGB WHITE;
  static const ColorRGB BLACK;
};

///////////////////////////////////////////////////////////////////////

/*! @brief RGB color value with alpha (opacity) channel.
 */
class ColorRGBA
{
public:
  ColorRGBA(void);
  ColorRGBA(const ColorRGB& rgb, float a = 1.f);
  ColorRGBA(const ColorHSL& hsl, float a = 1.f);
  ColorRGBA(const ColorHSV& hsv, float a = 1.f);
  ColorRGBA(float r, float g, float b, float a);
  explicit ColorRGBA(const String& string);
  /*! Clamps each component to the range [0, 1].
   */
  void clamp(void);
  ColorRGBA min(const ColorRGBA& color) const;
  ColorRGBA max(const ColorRGBA& color) const;
  String asString(void) const;
  operator float* (void);
  operator const float* (void) const;
  ColorRGBA operator - (void) const;
  ColorRGBA operator + (float value) const;
  ColorRGBA operator - (float value) const;
  ColorRGBA operator * (float value) const;
  ColorRGBA operator / (float value) const;
  ColorRGBA operator += (float value);
  ColorRGBA& operator -= (float value);
  ColorRGBA& operator *= (float value);
  ColorRGBA& operator /= (float value);
  ColorRGBA operator + (const ColorRGBA& color) const;
  ColorRGBA operator - (const ColorRGBA& color) const;
  ColorRGBA operator * (const ColorRGBA& color) const;
  ColorRGBA operator / (const ColorRGBA& color) const;
  ColorRGBA operator += (const ColorRGBA& color);
  ColorRGBA& operator -= (const ColorRGBA& color);
  ColorRGBA& operator *= (const ColorRGBA& color);
  ColorRGBA& operator /= (const ColorRGBA& color);
  bool operator == (const ColorRGBA& color) const;
  bool operator != (const ColorRGBA& color) const;
  float getLength(void) const;
  float getSquaredLength(void) const;
  void setDefaults(void);
  void set(const ColorRGB& rgb, float a);
  void set(float r, float g, float b, float a);
  float r;
  float g;
  float b;
  float a;
  static const ColorRGBA WHITE;
  static const ColorRGBA BLACK;
};

///////////////////////////////////////////////////////////////////////

/*! @brief HSL (hue, saturation, luminance) color value.
 */
class ColorHSL
{
public:
  /*! Default constructor.
   *  @remarks This does not initialize the object's fields.
   */
  ColorHSL(void);
  /*! Creates a color object with the specified values.
   */
  ColorHSL(float h, float s, float l);
  ColorHSL(const ColorRGB& rgb);
  ColorHSL(const ColorRGBA& rgba);
  ColorHSL(const ColorHSV& hsv);
  /*! Clamps each component to the range [0, 1].
   */
  void clamp(void);
  operator float* (void);
  operator const float* (void) const;
  ColorHSL operator - (void) const;
  ColorHSL operator + (float value) const;
  ColorHSL operator - (float value) const;
  ColorHSL operator * (float value) const;
  ColorHSL operator / (float value) const;
  ColorHSL operator += (float value);
  ColorHSL& operator -= (float value);
  ColorHSL& operator *= (float value);
  ColorHSL& operator /= (float value);
  ColorHSL operator + (const ColorHSL& color) const;
  ColorHSL operator - (const ColorHSL& color) const;
  ColorHSL operator * (const ColorHSL& color) const;
  ColorHSL operator / (const ColorHSL& color) const;
  ColorHSL operator += (const ColorHSL& color);
  ColorHSL& operator -= (const ColorHSL& color);
  ColorHSL& operator *= (const ColorHSL& color);
  ColorHSL& operator /= (const ColorHSL& color);
  bool operator == (const ColorHSL& color) const;
  bool operator != (const ColorHSL& color) const;
  void setDefaults(void);
  void set(float newH, float newS, float newL);
  float h;
  float s;
  float l;
  static const ColorHSL WHITE;
  static const ColorHSL BLACK;
};

///////////////////////////////////////////////////////////////////////

/*! @brief HSV (hue, saturation, value) color value.
 */
class ColorHSV
{
public:
  /*! Default constructor.
   *  @remarks This does not initialize the object's fields.
   */
  ColorHSV(void);
  /*! Creates a color object with the specified values.
   */
  ColorHSV(float h, float s, float v);
  ColorHSV(const ColorRGB& rgb);
  ColorHSV(const ColorRGBA& rgba);
  ColorHSV(const ColorHSL& hsv);
  /*! Clamps each component to the range [0, 1].
   */
  void clamp(void);
  operator float* (void);
  operator const float* (void) const;
  ColorHSV operator - (void) const;
  ColorHSV operator + (float value) const;
  ColorHSV operator - (float value) const;
  ColorHSV operator * (float value) const;
  ColorHSV operator / (float value) const;
  ColorHSV operator += (float value);
  ColorHSV& operator -= (float value);
  ColorHSV& operator *= (float value);
  ColorHSV& operator /= (float value);
  ColorHSV operator + (const ColorHSV& color) const;
  ColorHSV operator - (const ColorHSV& color) const;
  ColorHSV operator * (const ColorHSV& color) const;
  ColorHSV operator / (const ColorHSV& color) const;
  ColorHSV operator += (const ColorHSV& color);
  ColorHSV& operator -= (const ColorHSV& color);
  ColorHSV& operator *= (const ColorHSV& color);
  ColorHSV& operator /= (const ColorHSV& color);
  bool operator == (const ColorHSV& color) const;
  bool operator != (const ColorHSV& color) const;
  void setDefaults(void);
  void set(float h, float s, float v);
  float h;
  float s;
  float v;
  static const ColorHSV WHITE;
  static const ColorHSV BLACK;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_COLOR_H*/
///////////////////////////////////////////////////////////////////////

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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Color.h>

#include <cmath>
#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

ColorRGB::ColorRGB(void)
{
}

ColorRGB::ColorRGB(float sr, float sg, float sb):
  r(sr),
  g(sg),
  b(sb)
{
}

ColorRGB::ColorRGB(const ColorRGBA& rgba):
  r(rgba.r),
  g(rgba.g),
  b(rgba.b)
{
}

ColorRGB::ColorRGB(const ColorHSL& hsl)
{
  if (hsl.s == 0.f)
  {
    r = hsl.l;
    g = hsl.l;
    b = hsl.l;
  }
  else
  {
    ColorRGB temp;

    if (hsl.h < 1.f / 3.f)
    {
      temp.r = (1.f / 3.f - hsl.h) * 6.f;
      temp.g = hsl.h * 6.f;
      temp.b = 0.f;
    }
    else if (hsl.h < 2.f / 3.f)
    {
      temp.r = 0.f;
      temp.g = (2.f / 3.f - hsl.h) * 6.f;
      temp.b = (hsl.h - 1.f / 3.f) * 6.f;
    }
    else
    {
      temp.r = (hsl.h - 2.f / 3.f) * 6.f;
      temp.g = 0.f;
      temp.b = (1.f - hsl.h) * 6.f;
    }

    temp.r = fminf(temp.r, 1.f);
    temp.g = fminf(temp.g, 1.f);
    temp.b = fminf(temp.b, 1.f);

    temp.r = 2.f * hsl.s * temp.r + (1.f - hsl.s);
    temp.g = 2.f * hsl.s * temp.g + (1.f - hsl.s);
    temp.b = 2.f * hsl.s * temp.b + (1.f - hsl.s);

    if (hsl.l < 0.5)
    {
      r = hsl.l * temp.r;
      g = hsl.l * temp.g;
      b = hsl.l * temp.b;
    }
    else
    {
      r = (1 - hsl.l) * temp.r + 2 * hsl.l - 1;
      g = (1 - hsl.l) * temp.g + 2 * hsl.l - 1;
      b = (1 - hsl.l) * temp.b + 2 * hsl.l - 1;
    }
  }
}

ColorRGB::ColorRGB(const ColorHSV& hsv)
{
  if (hsv.s == 0.f)
  {
    r = hsv.v;
    g = hsv.v;
    b = hsv.v;
  }
  else
  {
    const float h = hsv.h * 6.f;
    const unsigned int H = (unsigned int) h;
    const float f = h - H;

    const float p = hsv.v * (1.f - hsv.s);
    const float q = hsv.v * (1.f - hsv.s * f);
    const float t = hsv.v * (1.f - hsv.s * (1.f - f));
    const float v = hsv.v;

    switch (H)
    {
      case 0:
	r = v;  g = t;  b = p;
	break;

      case 1:
	r = q;  g = v;  b = p;
	break;

      case 2:
	r = p;  g = v;  b = t;
	break;

      case 3:
	r = p;  g = q;  b = v;
	break;

      case 4:
	r = t;  g = p;  b = v;
	break;

      case 5:
	r = v;  g = p;  b = q;
	break;
    }
  }
}

ColorRGB::ColorRGB(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  stream >> r >> g >> b;
}

void ColorRGB::clamp(void)
{
  if (r > 1.f)
    r = 1.f;
  else if (r < 0.f)
    r = 0.f;

  if (g > 1.f)
    g = 1.f;
  else if (g < 0.f)
    g = 0.f;

  if (b > 1.f)
    b = 1.f;
  else if (b < 0.f)
    b = 0.f;
}

ColorRGB ColorRGB::min(const ColorRGB& color) const
{
  return ColorRGB(fminf(r, color.r), fminf(g, color.g), fminf(b, color.b));
}

ColorRGB ColorRGB::max(const ColorRGB& color) const
{
  return ColorRGB(fmaxf(r, color.r), fmaxf(g, color.g), fmaxf(b, color.b));
}

String ColorRGB::asString(void) const
{
  std::ostringstream stream;

  stream << r << ' ' << g << ' ' << b;
  return stream.str();
}

ColorRGB::operator float* (void)
{
  return &(r);
}

ColorRGB::operator const float* (void) const
{
  return &(r);
}

ColorRGB ColorRGB::operator - (void) const
{
  return ColorRGB(-r, -g, -b);
}

ColorRGB ColorRGB::operator + (float value) const
{
  return ColorRGB(r + value, g + value, b + value);
}

ColorRGB ColorRGB::operator - (float value) const
{
  return ColorRGB(r - value, g - value, b - value);
}

ColorRGB ColorRGB::operator * (float value) const
{
  return ColorRGB(r * value, g * value, b * value);
}

ColorRGB ColorRGB::operator / (float value) const
{
  return ColorRGB(r / value, g / value, b / value);
}

ColorRGB ColorRGB::operator += (float value)
{
  r += value;
  g += value;
  b += value;
  return *this;
}

ColorRGB& ColorRGB::operator -= (float value)
{
  r -= value;
  g -= value;
  b -= value;
  return *this;
}

ColorRGB& ColorRGB::operator *= (float value)
{
  r *= value;
  g *= value;
  b *= value;
  return *this;
}

ColorRGB& ColorRGB::operator /= (float value)
{
  r /= value;
  g /= value;
  b /= value;
  return *this;
}

ColorRGB ColorRGB::operator + (const ColorRGB& color) const
{
  return ColorRGB(r + color.r, g + color.g, b + color.b);
}

ColorRGB ColorRGB::operator - (const ColorRGB& color) const
{
  return ColorRGB(r - color.r, g - color.g, b - color.b);
}

ColorRGB ColorRGB::operator * (const ColorRGB& color) const
{
  return ColorRGB(r * color.r, g * color.g, b * color.b);
}

ColorRGB ColorRGB::operator / (const ColorRGB& color) const
{
  return ColorRGB(r / color.r, g / color.g, b / color.b);
}

ColorRGB ColorRGB::operator += (const ColorRGB& color)
{
  r += color.r;
  g += color.g;
  b += color.b;
  return *this;
}

ColorRGB& ColorRGB::operator -= (const ColorRGB& color)
{
  r -= color.r;
  g -= color.g;
  b -= color.b;
  return *this;
}

ColorRGB& ColorRGB::operator *= (const ColorRGB& color)
{
  r *= color.r;
  g *= color.g;
  b *= color.b;
  return *this;
}

ColorRGB& ColorRGB::operator /= (const ColorRGB& color)
{
  r /= color.r;
  g /= color.g;
  b /= color.b;
  return *this;
}

bool ColorRGB::operator == (const ColorRGB& color) const
{
  return r == color.r && g == color.g && b == color.b;
}

bool ColorRGB::operator != (const ColorRGB& color) const
{
  return r != color.r || g != color.g || b != color.b;
}

float ColorRGB::getLength(void) const
{
  return r * r + g * g + b * b;
}

float ColorRGB::getSquaredLength(void) const
{
  return sqrtf(r * r + g * g + b * b);
}

void ColorRGB::setDefaults(void)
{
  r = 0.f;
  g = 0.f;
  b = 0.f;
}

void ColorRGB::set(float sr, float sg, float sb)
{
  r = sr;
  g = sg;
  b = sb;
}

const ColorRGB ColorRGB::WHITE(1.f, 1.f, 1.f);
const ColorRGB ColorRGB::BLACK(0.f, 0.f, 0.f);

///////////////////////////////////////////////////////////////////////

ColorRGBA::ColorRGBA(void)
{
}

ColorRGBA::ColorRGBA(float sr, float sg, float sb, float sa):
  r(sr),
  g(sg),
  b(sb),
  a(sa)
{
}

ColorRGBA::ColorRGBA(const ColorRGB& rgb, float sa):
  r(rgb.r),
  g(rgb.g),
  b(rgb.b),
  a(sa)
{
}

ColorRGBA::ColorRGBA(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  stream >> r >> g >> b >> a;
}

void ColorRGBA::clamp(void)
{
  if (r > 1.f)
    r = 1.f;
  else if (r < 0.f)
    r = 0.f;

  if (g > 1.f)
    g = 1.f;
  else if (g < 0.f)
    g = 0.f;

  if (b > 1.f)
    b = 1.f;
  else if (b < 0.f)
    b = 0.f;

  if (a > 1.f)
    a = 1.f;
  else if (a < 0.f)
    a = 0.f;
}

ColorRGBA ColorRGBA::min(const ColorRGBA& color) const
{
  return ColorRGBA(fminf(r, color.r),
                   fminf(g, color.g),
		   fminf(b, color.b),
		   fminf(a, color.a));
}

ColorRGBA ColorRGBA::max(const ColorRGBA& color) const
{
  return ColorRGBA(fmaxf(r, color.r),
                   fmaxf(g, color.g),
		   fmaxf(b, color.b),
		   fmaxf(a, color.a));
}

String ColorRGBA::asString(void) const
{
  std::ostringstream stream;

  stream << r << ' ' << g << ' ' << b << ' ' << a;
  return stream.str();
}

ColorRGBA::operator float* (void)
{
  return &(r);
}

ColorRGBA::operator const float* (void) const
{
  return &(r);
}

ColorRGBA ColorRGBA::operator - (void) const
{
  return ColorRGBA(-r, -g, -b, -a);
}

ColorRGBA ColorRGBA::operator + (float value) const
{
  return ColorRGBA(r + value, g + value, b + value, a + value);
}

ColorRGBA ColorRGBA::operator - (float value) const
{
  return ColorRGBA(r - value, g - value, b - value, a - value);
}

ColorRGBA ColorRGBA::operator * (float value) const
{
  return ColorRGBA(r * value, g * value, b * value, a * value);
}

ColorRGBA ColorRGBA::operator / (float value) const
{
  return ColorRGBA(r / value, g / value, b / value, a / value);
}

ColorRGBA ColorRGBA::operator += (float value)
{
  r += value;
  g += value;
  b += value;
  a += value;
  return *this;
}

ColorRGBA& ColorRGBA::operator -= (float value)
{
  r -= value;
  g -= value;
  b -= value;
  a -= value;
  return *this;
}

ColorRGBA& ColorRGBA::operator *= (float value)
{
  r *= value;
  g *= value;
  b *= value;
  a *= value;
  return *this;
}

ColorRGBA& ColorRGBA::operator /= (float value)
{
  r /= value;
  g /= value;
  b /= value;
  a /= value;
  return *this;
}

ColorRGBA ColorRGBA::operator + (const ColorRGBA& color) const
{
  return ColorRGBA(r + color.r, g + color.g, b + color.b, a + color.a);
}

ColorRGBA ColorRGBA::operator - (const ColorRGBA& color) const
{
  return ColorRGBA(r - color.r, g - color.g, b - color.b, a - color.a);
}

ColorRGBA ColorRGBA::operator * (const ColorRGBA& color) const
{
  return ColorRGBA(r * color.r, g * color.g, b * color.b, a * color.a);
}

ColorRGBA ColorRGBA::operator / (const ColorRGBA& color) const
{
  return ColorRGBA(r / color.r, g / color.g, b / color.b, a / color.a);
}

ColorRGBA ColorRGBA::operator += (const ColorRGBA& color)
{
  r += color.r;
  g += color.g;
  b += color.b;
  a += color.a;
  return *this;
}

ColorRGBA& ColorRGBA::operator -= (const ColorRGBA& color)
{
  r -= color.r;
  g -= color.g;
  b -= color.b;
  a -= color.a;
  return *this;
}

ColorRGBA& ColorRGBA::operator *= (const ColorRGBA& color)
{
  r *= color.r;
  g *= color.g;
  b *= color.b;
  a *= color.a;
  return *this;
}

ColorRGBA& ColorRGBA::operator /= (const ColorRGBA& color)
{
  r /= color.r;
  g /= color.g;
  b /= color.b;
  a /= color.a;
  return *this;
}

bool ColorRGBA::operator == (const ColorRGBA& color) const
{
  return r == color.r && g == color.g && b == color.b && a == color.a;
}

bool ColorRGBA::operator != (const ColorRGBA& color) const
{
  return r != color.r || g != color.g || b != color.b || a != color.a;
}

float ColorRGBA::getLength(void) const
{
  return r * r + g * g + b * b;
}

float ColorRGBA::getSquaredLength(void) const
{
  return sqrtf(r * r + g * g + b * b);
}

void ColorRGBA::setDefaults(void)
{
  r = 0.f;
  g = 0.f;
  b = 0.f;
  a = 1.f;
}

void ColorRGBA::set(const ColorRGB& rgb, float sa)
{
  r = rgb.r;
  g = rgb.g;
  b = rgb.b;
  a = sa;
}

void ColorRGBA::set(float sr, float sg, float sb, float sa)
{
  r = sr;
  g = sg;
  b = sb;
  a = sa;
}

const ColorRGBA ColorRGBA::WHITE(1.f, 1.f, 1.f, 1.f);
const ColorRGBA ColorRGBA::BLACK(0.f, 0.f, 0.f, 1.f);

///////////////////////////////////////////////////////////////////////

ColorHSL::ColorHSL(void)
{
}

ColorHSL::ColorHSL(float sh, float ss, float sl):
  h(sh),
  s(ss),
  l(sl)
{
}

ColorHSL::ColorHSL(const ColorRGB& rgb)
{
  const float maxColor = fmaxf(rgb.r, fmaxf(rgb.g, rgb.b));
  const float minColor = fminf(rgb.r, fminf(rgb.g, rgb.b));
  const float delta = maxColor - minColor;

  if (delta == 0.f)
  {
    h = 0.f;
    s = 0.f;
    l = minColor;
  }
  else
  {
    l = (minColor + maxColor) / 2.f;

    if (l < 0.5f)
      s = delta / (maxColor + minColor);
    else
      s = delta / (2.f - maxColor - minColor);

    if (rgb.r == maxColor)
      h = ((rgb.g - rgb.b) / (maxColor - minColor)) / 6.f;
    else if (rgb.g == maxColor)
      h = (2.f + (rgb.b - rgb.r) / (maxColor - minColor)) / 6.f;
    else
      h = (4.f + (rgb.r - rgb.g) / (maxColor - minColor)) / 6.f;

    if (h < 0.f)
      h += 1.f;
  }
}

void ColorHSL::clamp(void)
{
  if (h < 0.f)
    h += floorf(h);
  else if (h > 1.f)
    h -= floorf(h);

  if (s > 1.f)
    s = 1.f;
  else if (s < 0.f)
    s = 0.f;

  if (l > 1.f)
    l = 1.f;
  else if (l < 0.f)
    l = 0.f;
}

ColorHSL::operator float* (void)
{
  return &(h);
}

ColorHSL::operator const float* (void) const
{
  return &(h);
}

ColorHSL ColorHSL::operator - (void) const
{
  return ColorHSL(-h, -s, -l);
}

ColorHSL ColorHSL::operator + (float value) const
{
  return ColorHSL(h + value, s + value, l + value);
}

ColorHSL ColorHSL::operator - (float value) const
{
  return ColorHSL(h - value, s - value, l - value);
}

ColorHSL ColorHSL::operator * (float value) const
{
  return ColorHSL(h * value, s * value, l * value);
}

ColorHSL ColorHSL::operator / (float value) const
{
  return ColorHSL(h / value, s / value, l / value);
}

ColorHSL ColorHSL::operator += (float value)
{
  h += value;
  s += value;
  l += value;
  return *this;
}

ColorHSL& ColorHSL::operator -= (float value)
{
  h -= value;
  s -= value;
  l -= value;
  return *this;
}

ColorHSL& ColorHSL::operator *= (float value)
{
  h *= value;
  s *= value;
  l *= value;
  return *this;
}

ColorHSL& ColorHSL::operator /= (float value)
{
  h /= value;
  s /= value;
  l /= value;
  return *this;
}

ColorHSL ColorHSL::operator + (const ColorHSL& color) const
{
  return ColorHSL(h + color.h, s + color.s, l + color.l);
}

ColorHSL ColorHSL::operator - (const ColorHSL& color) const
{
  return ColorHSL(h - color.h, s - color.s, l - color.l);
}

ColorHSL ColorHSL::operator * (const ColorHSL& color) const
{
  return ColorHSL(h * color.h, s * color.s, l * color.l);
}

ColorHSL ColorHSL::operator / (const ColorHSL& color) const
{
  return ColorHSL(h / color.h, s / color.s, l / color.l);
}

ColorHSL ColorHSL::operator += (const ColorHSL& color)
{
  h += color.h;
  s += color.s;
  l += color.l;
  return *this;
}

ColorHSL& ColorHSL::operator -= (const ColorHSL& color)
{
  h -= color.h;
  s -= color.s;
  l -= color.l;
  return *this;
}

ColorHSL& ColorHSL::operator *= (const ColorHSL& color)
{
  h *= color.h;
  s *= color.s;
  l *= color.l;
  return *this;
}

ColorHSL& ColorHSL::operator /= (const ColorHSL& color)
{
  h /= color.h;
  s /= color.s;
  l /= color.l;
  return *this;
}

bool ColorHSL::operator == (const ColorHSL& color) const
{
  return h == color.h && s == color.s && l == color.l;
}

bool ColorHSL::operator != (const ColorHSL& color) const
{
  return h != color.h || s != color.s || l != color.l;
}

void ColorHSL::setDefaults(void)
{
  h = 0.f;
  s = 0.f;
  l = 0.f;
}

void ColorHSL::set(float sh, float ss, float sl)
{
  h = sh;
  s = ss;
  l = sl;
}

///////////////////////////////////////////////////////////////////////

ColorHSV::ColorHSV(void)
{
}

ColorHSV::ColorHSV(float sh, float ss, float sv):
  h(sh),
  s(ss),
  v(sv)
{
}

ColorHSV::ColorHSV(const ColorRGB& rgb)
{
  const float maxColor = fmaxf(rgb.r, fmaxf(rgb.g, rgb.b));
  const float minColor = fminf(rgb.r, fminf(rgb.g, rgb.b));

  v = maxColor;

  if (v == 0.f)
    s = 0.f;
  else
    s = (maxColor - minColor) / maxColor;

  if (s == 0.f)
    h = 0.f;
  else
  {
    if (rgb.r == maxColor)
      h = ((rgb.g - rgb.b) / (maxColor - minColor)) / 6.f;
    else if (rgb.g == maxColor)
      h = (2.f + (rgb.b - rgb.r) / (maxColor - minColor)) / 6.f;
    else
      h = (4.f + (rgb.r - rgb.g) / (maxColor - minColor)) / 6.f;

    if (h < 0)
      h += 1.f;
  }
}

void ColorHSV::clamp(void)
{
  if (h < 0.f)
    h += floorf(h);
  else if (h > 1.f)
    h -= floorf(h);

  if (s > 1.f)
    s = 1.f;
  else if (s < 0.f)
    s = 0.f;

  if (v > 1.f)
    v = 1.f;
  else if (v < 0.f)
    v = 0.f;
}

ColorHSV::operator float* (void)
{
  return &(h);
}

ColorHSV::operator const float* (void) const
{
  return &(h);
}

ColorHSV ColorHSV::operator - (void) const
{
  return ColorHSV(-h, -s, -v);
}

ColorHSV ColorHSV::operator + (float value) const
{
  return ColorHSV(h + value, s + value, v + value);
}

ColorHSV ColorHSV::operator - (float value) const
{
  return ColorHSV(h - value, s - value, v - value);
}

ColorHSV ColorHSV::operator * (float value) const
{
  return ColorHSV(h * value, s * value, v * value);
}

ColorHSV ColorHSV::operator / (float value) const
{
  return ColorHSV(h / value, s / value, v / value);
}

ColorHSV ColorHSV::operator += (float value)
{
  h += value;
  s += value;
  v += value;
  return *this;
}

ColorHSV& ColorHSV::operator -= (float value)
{
  h -= value;
  s -= value;
  v -= value;
  return *this;
}

ColorHSV& ColorHSV::operator *= (float value)
{
  h *= value;
  s *= value;
  v *= value;
  return *this;
}

ColorHSV& ColorHSV::operator /= (float value)
{
  h /= value;
  s /= value;
  v /= value;
  return *this;
}

ColorHSV ColorHSV::operator + (const ColorHSV& color) const
{
  return ColorHSV(h + color.h, s + color.s, v + color.v);
}

ColorHSV ColorHSV::operator - (const ColorHSV& color) const
{
  return ColorHSV(h - color.h, s - color.s, v - color.v);
}

ColorHSV ColorHSV::operator * (const ColorHSV& color) const
{
  return ColorHSV(h * color.h, s * color.s, v * color.v);
}

ColorHSV ColorHSV::operator / (const ColorHSV& color) const
{
  return ColorHSV(h / color.h, s / color.s, v / color.v);
}

ColorHSV ColorHSV::operator += (const ColorHSV& color)
{
  h += color.h;
  s += color.s;
  v += color.v;
  return *this;
}

ColorHSV& ColorHSV::operator -= (const ColorHSV& color)
{
  h -= color.h;
  s -= color.s;
  v -= color.v;
  return *this;
}

ColorHSV& ColorHSV::operator *= (const ColorHSV& color)
{
  h *= color.h;
  s *= color.s;
  v *= color.v;
  return *this;
}

ColorHSV& ColorHSV::operator /= (const ColorHSV& color)
{
  h /= color.h;
  s /= color.s;
  v /= color.v;
  return *this;
}

bool ColorHSV::operator == (const ColorHSV& color) const
{
  return h == color.h && s == color.s && v == color.v;
}

bool ColorHSV::operator != (const ColorHSV& color) const
{
  return h != color.h || s != color.s || v != color.v;
}

void ColorHSV::setDefaults(void)
{
  h = 0.f;
  s = 0.f;
  v = 0.f;
}

void ColorHSV::set(float sh, float ss, float sv)
{
  h = sh;
  s = ss;
  v = sv;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

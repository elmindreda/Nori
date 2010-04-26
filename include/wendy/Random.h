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
#ifndef WENDY_RANDOM_H
#define WENDY_RANDOM_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

template <typename T>
class Random
{
public:
  Random(void);
  Random(const T& min, const T& max);
  T generate(void) const;
  void set(const T& newMin, const T& newMax);
  T min;
  T max;
};

///////////////////////////////////////////////////////////////////////

typedef Random<float> RandomRange;
typedef Random<Vec2> RandomArea;
typedef Random<Vec3> RandomVolume;
typedef Random<ColorRGB> RandomRGB;
typedef Random<ColorRGBA> RandomRGBA;

///////////////////////////////////////////////////////////////////////

float normalizedRandom(void);

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RANDOM_H*/
///////////////////////////////////////////////////////////////////////

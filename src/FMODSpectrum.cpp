///////////////////////////////////////////////////////////////////////
// Wendy FMOD library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <moira/Config.h>
#include <moira/Core.h>

#include <wendy/Config.h>
#include <wendy/FMOD.h>
#include <wendy/FMODSpectrum.h>
#include <wendy/FMODSystem.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace FMOD
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Spectrum::Spectrum(void)
{
  FSOUND_DSP_SetActive(FSOUND_DSP_GetFFTUnit(), true);
}

unsigned int Spectrum::getPointCount(void) const
{
  return 512;
}

float Spectrum::getPoint(unsigned int index) const
{
  return FSOUND_DSP_GetSpectrum()[index];
}

void Spectrum::getPoints(PointList& points) const
{
  const float* source = FSOUND_DSP_GetSpectrum();

  points.resize(512);

  for (unsigned int i = 0;  i < points.size();  i++)
    points[i] = source[i];
}

///////////////////////////////////////////////////////////////////////

  } /*namespace FMOD*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

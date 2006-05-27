///////////////////////////////////////////////////////////////////////
// Wendy FMOD library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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
#include <moira/Log.h>

#include <wendy/Config.h>
#include <wendy/FMOD.h>
#include <wendy/FMODSample.h>
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

System::~System(void)
{
  FSOUND_Close();
}

Sample* System::createSample(const Path& path)
{
  Ptr<Sample> sample = new Sample();
  if (!sample->init(path))
    return NULL;

  return sample.detachObject();
}

Spectrum* System::getSpectrum(void)
{
  spectrum = new Spectrum();

  return spectrum;
}

bool System::create(void)
{
  Ptr<System> system = new System();
  if (!system->init())
    return false;

  set(system.detachObject());
  return true;
}

System::System(void)
{
}

bool System::init(void)
{
  if (FSOUND_GetVersion() != FMOD_VERSION)
  {
    Log::writeError("Mismatched version of FMOD detected during initialization");
    return false;
  }
  
  if (!FSOUND_Init(44100, 32, FSOUND_INIT_GLOBALFOCUS))
  {
    Log::writeError("Unable to initialize FMOD");
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace FMOD*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

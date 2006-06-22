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
#ifndef WENDY_FMODSAMPLE_H
#define WENDY_FMODSAMPLE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace FMOD
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Sample
{
  friend class System;
public:
  ~Sample(void);
  void start(void);
  void stop(void);
  void pause(void);
  void resume(void);
  bool isStarted(void) const;
  bool isPaused(void) const;
  Time getTime(void) const;
  void setTime(Time newTime);
private:
  Sample(void);
  bool init(const Path& path);
  bool started;
  Block buffer;
  FSOUND_STREAM* stream;
  int channel;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace FMOD*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_FMODSAMPLE_H*/
///////////////////////////////////////////////////////////////////////

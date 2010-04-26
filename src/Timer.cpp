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
#include <wendy/Timer.h>

#if WENDY_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <time.h>

#if _WIN32
#include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Timer::Timer(void):
  started(false),
  paused(false),
  baseTime(0.0)
{
}

void Timer::start(void)
{
  stop();

  baseTime = getCurrentTime();
  started = true;
}

void Timer::stop(void)
{
  started = false;
  paused = false;
  baseTime = 0.0;
}

void Timer::pause(void)
{
  if (!started)
    return;

  // Store expired seconds until resume.
  baseTime = getCurrentTime() - baseTime;
  paused = true;
}

void Timer::resume(void)
{
  if (!started)
    return;

  // Restore base time after pause.
  baseTime = getCurrentTime() - baseTime;
  paused = false;
}

bool Timer::isStarted(void) const
{
  return started;
}

bool Timer::isPaused(void) const
{
  return paused;
}

Time Timer::getTime(void) const
{
  if (started)
  {
    if (paused)
      return baseTime;
    else
      return getCurrentTime() - baseTime;
  }
  else
    return 0.0;
}

void Timer::setTime(Time time)
{
  if (started)
  {
    if (time < 0.0)
      time = 0.0;

    if (paused)
      baseTime = time;
    else
      baseTime += getTime() - time;
  }
}

Time Timer::getCurrentTime(void)
{
#if _WIN32
  static Time timeSlice = 0.0;

  if (!timeSlice)
  {
    __int64 frequency;
    QueryPerformanceFrequency((LARGE_INTEGER*) &frequency);
    timeSlice = 1.0 / (Time) frequency;
  }

  __int64 currentTime;
  QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);

  return (Time) currentTime * timeSlice;
#else
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return (Time) (tv.tv_sec + tv.tv_usec / 1000000.0);
#endif
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

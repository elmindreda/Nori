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

#define GLFW_NO_GLU
#include <GL/glfw3.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Timer::Timer():
  started(false),
  paused(false),
  baseTime(0.0),
  prevTime(0.0)
{
}

void Timer::start()
{
  stop();

  baseTime = getCurrentTime();
  started = true;
}

void Timer::stop()
{
  started = false;
  paused = false;
  baseTime = 0.0;
  prevTime = 0.0;
}

void Timer::pause()
{
  if (!started)
    return;

  // Store expired seconds until resume.
  baseTime = getCurrentTime() - baseTime;
  paused = true;
}

void Timer::resume()
{
  if (!started)
    return;

  // Restore base time after pause.
  baseTime = getCurrentTime() - baseTime;
  paused = false;
}

bool Timer::isStarted() const
{
  return started;
}

bool Timer::isPaused() const
{
  return paused;
}

Time Timer::getTime() const
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

Time Timer::getDeltaTime()
{
  if (started)
  {
    // Since this uses base-relative time, it doesn't need special
    // cases for the paused state (I hope)

    Time deltaTime = getTime() - prevTime;
    prevTime += deltaTime;
    return deltaTime;
  }
  else
    return 0.0;
}

Time Timer::getDeltaQueryTime() const
{
  return prevTime;
}

Time Timer::getCurrentTime()
{
  if (!glfwInit())
  {
    logError("Failed to initialize GLFW: %s", glfwErrorString(glfwGetError()));
    return 0.0;
  }

  return glfwGetTime();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

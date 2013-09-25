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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/Timer.hpp>

#define GLFW_NO_GLU
#include <GLFW/glfw3.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Timer::Timer():
  m_started(false),
  m_paused(false),
  m_baseTime(0.0),
  m_prevTime(0.0)
{
}

void Timer::start()
{
  stop();

  m_baseTime = currentTime();
  m_started = true;
}

void Timer::stop()
{
  m_started = false;
  m_paused = false;
  m_baseTime = 0.0;
  m_prevTime = 0.0;
}

void Timer::pause()
{
  if (!m_started)
    return;

  // Store expired seconds until resume.
  m_baseTime = currentTime() - m_baseTime;
  m_paused = true;
}

void Timer::resume()
{
  if (!m_started)
    return;

  // Restore base time after pause.
  m_baseTime = currentTime() - m_baseTime;
  m_paused = false;
}

Time Timer::time() const
{
  if (m_started)
  {
    if (m_paused)
      return m_baseTime;
    else
      return currentTime() - m_baseTime;
  }
  else
    return 0.0;
}

void Timer::setTime(Time newTime)
{
  if (m_started)
  {
    if (newTime < 0.0)
      newTime = 0.0;

    if (m_paused)
      m_baseTime = newTime;
    else
      m_baseTime += time() - newTime;
  }
}

Time Timer::deltaTime()
{
  if (m_started)
  {
    // Since this uses base-relative time, it doesn't need special
    // cases for the paused state (I hope)

    const Time deltaTime = time() - m_prevTime;
    m_prevTime += deltaTime;
    return deltaTime;
  }
  else
    return 0.0;
}

Time Timer::currentTime()
{
  if (!glfwInit())
  {
    logError("Failed to initialize GLFW");
    return 0.0;
  }

  return glfwGetTime();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

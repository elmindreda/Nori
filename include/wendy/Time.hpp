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
#ifndef WENDY_TIME_HPP
#define WENDY_TIME_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! Time value, in seconds.
 */
typedef double Time;

///////////////////////////////////////////////////////////////////////

/*! High-resolution timer.
 */
class Timer
{
public:
  /*! Default constructor.
   */
  Timer();
  /*! Starts the timer.
   */
  void start();
  /*! Stops a started timer.
   */
  void stop();
  /*! Pauses a running timer.
   */
  void pause();
  /*! Resumes a paused timer.
   */
  void resume();
  /*! @return @c true if the timer is started, otherwise @c false.
   */
  bool isStarted() const { return m_started; }
  /*! @return @c true if the timer is paused, otherwise @c false.
   */
  bool isPaused() const { return m_paused; }
  /*! @return The current time, in seconds.
   */
  Time time() const;
  /*! Sets the current time of a started timer.
   *  @param[in] newTime The new time, in seconds.
   */
  void setTime(Time newTime);
  /*! @return The time, in seconds, since the last call to deltaTime.
   */
  Time deltaTime();
  /*! @return The time, in seconds, between when this timer was started and
   *  deltaTime was last called.
   */
  Time deltaQueryTime() const { return m_prevTime; }
  /*! @return The current time, in seconds.
   */
  static Time currentTime();
private:
  bool m_started;
  bool m_paused;
  Time m_baseTime;
  Time m_prevTime;
};

///////////////////////////////////////////////////////////////////////

class Ticker
{
public:
  Ticker(Time period);
  uint update(Time deltaTime);
  Time period() const { return m_period; }
private:
  Time m_period;
  Time m_remainder;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_TIME_HPP*/
///////////////////////////////////////////////////////////////////////

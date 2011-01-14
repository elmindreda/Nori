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
#ifndef WENDY_TIMER_H
#define WENDY_TIMER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! High-resolution timer.
 */
class Timer
{
public:
  /*! Default constructor.
   */
  Timer(void);
  /*! Starts the timer.
   */
  void start(void);
  /*! Stops a started timer.
   */
  void stop(void);
  /*! Pauses a running timer.
   */
  void pause(void);
  /*! Resumes a paused timer.
   */
  void resume(void);
  /*! @return @c true if the timer is started, otherwise @c false.
   */
  bool isStarted(void) const;
  /*! @return @c true if the timer is paused, otherwise @c false.
   */
  bool isPaused(void) const;
  /*! @return The current time, in seconds.
   */
  Time getTime(void) const;
  /*! Sets the current time of a started timer.
   *  @param newTime [in] The new time, in seconds.
   */
  void setTime(Time newTime);
  /*! @return The time, in seconds, since the last call to getDeltaTime.
   */
  Time getDeltaTime(void);
  /*! @return The current time, in seconds.
   */
  static Time getCurrentTime(void);
private:
  bool started;
  bool paused;
  Time baseTime;
  Time prevTime;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_TIMER_H*/
///////////////////////////////////////////////////////////////////////

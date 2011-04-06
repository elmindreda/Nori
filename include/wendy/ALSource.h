///////////////////////////////////////////////////////////////////////
// Wendy OpenAL library
// Copyright (c) 2007 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_ALSOURCE_H
#define WENDY_ALSOURCE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup openal
 */
class Source : public RefObject
{
public:
  ~Source(void);
  void start(void);
  void stop(void);
  void pause(void);
  void resume(void);
  bool isStarted(void) const;
  bool isPaused(void) const;
  bool isLooping(void) const;
  void setLooping(bool newState);
  const vec3& getPosition(void) const;
  void setPosition(const vec3& newPosition);
  const vec3& getVelocity(void) const;
  void setVelocity(const vec3& newVelocity);
  Buffer* getBuffer(void) const;
  void setBuffer(Buffer* newBuffer);
  Context& getContext(void) const;
  static Ref<Source> create(Context& context);
private:
  Source(Context& context);
  Source(const Source& source);
  bool init(void);
  Source& operator = (const Source& source);
  Context& context;
  unsigned int sourceID;
  bool started;
  bool paused;
  bool looping;
  vec3 position;
  vec3 velocity;
  Ref<Buffer> buffer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ALSOURCE_H*/
///////////////////////////////////////////////////////////////////////

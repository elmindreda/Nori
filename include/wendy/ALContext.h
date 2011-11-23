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
#ifndef WENDY_ALCONTEXT_H
#define WENDY_ALCONTEXT_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

/*! @brief OpenAL context singleton.
 *  @ingroup openal
 */
class Context : public Singleton<Context>
{
public:
  /*! Destructor.
   */
  ~Context();
  /*! @return The position of the context listener.
   */
  const vec3& getListenerPosition() const;
  /*! Sets the position of the context listener.
   */
  void setListenerPosition(const vec3& newPosition);
  /*! @return The velocity of the context listener.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the source velocity to calculate doppler shift.
   */
  const vec3& getListenerVelocity() const;
  /*! Sets the velocity of the context listener.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the source velocity to calculate doppler shift.
   */
  void setListenerVelocity(const vec3& newVelocity);
  /*! @return The gain of the context listener.
   */
  const quat& getListenerRotation() const;
  /*! Sets the rotation of the context listener.
   */
  void setListenerRotation(const quat& newRotation);
  /*! @return The gain of the context listener.
   */
  float getListenerGain() const;
  /*! Sets the listener gain of this context.
   */
  void setListenerGain(float newGain);
  /*! @return The resource cache used by this context.
   */
  ResourceCache& getCache() const;
  /*! Creates the context singleton object.
   *  @param[in] cache The resource cache to use.
   *  @return @c true if successful, or @c false otherwise.
   */
  static bool createSingleton(ResourceCache& cache);
private:
  Context(ResourceCache& cache);
  Context(const Context& source);
  bool init();
  Context& operator = (const Context& source);
  ResourceCache& cache;
  void* device;
  void* context;
  vec3 listenerPosition;
  vec3 listenerVelocity;
  quat listenerRotation;
  float listenerGain;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ALCONTEXT_H*/
///////////////////////////////////////////////////////////////////////

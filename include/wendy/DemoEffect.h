///////////////////////////////////////////////////////////////////////
// Wendy demo system
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
#ifndef WENDY_DEMOEFFECT_H
#define WENDY_DEMOEFFECT_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @defgroup demo Demo system
 *
 *  These classes are specifically for the production of demos. They provide
 *  hierarchial demo effect management, demo show and effect instancing from
 *  XML files, generic events and more.
 */

///////////////////////////////////////////////////////////////////////

class Effect;

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class Event //: public Decorated
{
public:
  const String& getName(void) const;
  Time getMoment(void) const;
private:
  String name;
  Time moment;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Demo effect factory interface.
 *  @ingroup demo
 */
class EffectType : public Managed<EffectType>
{
public:
  /*! Constructor.
   *  @param name The name of this effect type.
   */
  EffectType(const String& name);
  /*! Creates an instance of this effect type.
   *  @param name The desired name of the created effect object.
   *  @return The newly created effect object, or @c NULL.
   */
  virtual Effect* createEffect(const String& name = "",
                               Time duration = 0.0) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Template helper for effect factory class creation.
 *  @ingroup demo
 */
template <typename T>
class EffectTemplate : public EffectType
{
public:
  inline EffectTemplate(const String& name);
  inline Effect* createEffect(const String& name = "",
                              Time duration = 0.0);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Demo effect base class.
 *  @ingroup demo
 */
class Effect : public Node<Effect>, public Managed<Effect>
{
  friend class Player;
public:
  Effect(const String& name = "",
         EffectType* type = NULL,
	 Time duration = 0.0);
  bool isActive(void) const;
  EffectType* getType(void) const;
  Time getDuration(void) const;
  Time getTimeElapsed(void) const;
protected:
  void prepareChildren(void) const;
  void renderChildren(void) const; 
  virtual void prepare(void) const;
  virtual void render(void) const;
  virtual void update(Time deltaTime);
  virtual void trigger(const Event& event);
  virtual void restart(void);
private:
  EffectType* type;
  Time duration;
  Time elapsed;
  bool active;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup default
 */
class NullEffect : public Effect
{
public:
  NullEffect(const String& name = "",
             EffectType* type = NULL,
	     Time duration = 0.0);
  bool init(void);
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup default
 */
class ClearEffect : public Effect
{
public:
  ClearEffect(const String& name = "",
              EffectType* type = NULL,
	      Time duration = 0.0);
  bool init(void);
private:
  void render(void) const;
  void trigger(Time moment, const String& name, const String& value);
  void restart(void);
  ColorRGBA color;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline EffectTemplate<T>::EffectTemplate(const String& name):
  EffectType(name)
{
}

template <typename T>
inline Effect* EffectTemplate<T>::createEffect(const String& name,
                                                       Time duration)
{
  Ptr<T> effect = new T(name, this, duration);
  if (!effect->init())
    return NULL;

  return effect.detachObject();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOEFFECT_H*/
///////////////////////////////////////////////////////////////////////
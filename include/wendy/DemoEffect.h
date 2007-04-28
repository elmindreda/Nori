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
 *  XML files and more.
 */

///////////////////////////////////////////////////////////////////////

class Effect;

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
  virtual Effect* createEffect(const String& name = "") = 0;
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
  inline Effect* createEffect(const String& name = "");
};

///////////////////////////////////////////////////////////////////////

/*! @brief Demo effect base class.
 *  @ingroup demo
 */
class Effect : public Node<Effect>, public Managed<Effect>
{
  friend class Show;
  friend class Property;
public:
  typedef std::vector<Property*> PropertyList;
  Effect(EffectType& type, const String& name = "");
  ~Effect(void);
  Property* findProperty(const String& name);
  bool isActive(void) const;
  EffectType& getType(void) const;
  Time getGlobalOffset(void) const;
  Time getStartTime(void) const;
  void setStartTime(Time newTime);
  Time getDuration(void) const;
  void setDuration(Time newDuration);
  Time getTimeElapsed(void) const;
  const PropertyList& getProperties(void) const;
protected:
  void prepareChildren(void) const;
  void renderChildren(void) const; 
  virtual void prepare(void) const;
  virtual void render(void) const;
  virtual void update(Time deltaTime);
  virtual void restart(void);
private:
  bool active;
  bool updated;
  EffectType& type;
  Time start;
  Time duration;
  Time elapsed;
  PropertyList properties;
};

///////////////////////////////////////////////////////////////////////

class NullEffect : public Effect
{
public:
  NullEffect(EffectType& type, const String& name = "");
  bool init(void);
};

///////////////////////////////////////////////////////////////////////

class ClearEffect : public Effect
{
public:
  ClearEffect(EffectType& type, const String& name = "");
  bool init(void);
private:
  void render(void) const;
  ColorPropertyRGB color;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline EffectTemplate<T>::EffectTemplate(const String& name):
  EffectType(name)
{
}

template <typename T>
inline Effect* EffectTemplate<T>::createEffect(const String& name)
{
  Ptr<T> effect = new T(*this, name);
  if (!effect->init())
  {
    Log::writeWarning("Demo effect %s of type %s failed to initialize",
                      name.c_str(),
		      getName().c_str());
    return NULL;
  }

  return effect.detachObject();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOEFFECT_H*/
///////////////////////////////////////////////////////////////////////

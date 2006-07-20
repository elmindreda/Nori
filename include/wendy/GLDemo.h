///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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
//
// Credits to AMC and Outbreak for inspiration and discussion.
//
///////////////////////////////////////////////////////////////////////
#ifndef WENDY_GLDEMO_H
#define WENDY_GLDEMO_H
///////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <stack>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class DemoEffect;

///////////////////////////////////////////////////////////////////////

/*! Demo effect factory interface.
 */
class DemoEffectType : public Managed<DemoEffectType>
{
public:
  /*! Constructor.
   *  @param name [in] The name of this effect type.
   */
  DemoEffectType(const String& name);
  /*! Creates an instance of this effect type.
   *  @param name [in] The desired name of the created effect object.
   *  @return The newly created effect object, or @c NULL.
   */
  virtual DemoEffect* createEffect(const String& name = "",
                                   Time duration = 0.0) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! Template helper for automatic effect types.
 */
template <typename T>
class DemoEffectTemplate : public DemoEffectType
{
public:
  inline DemoEffectTemplate(const String& name);
  inline DemoEffect* createEffect(const String& name = "",
                                  Time duration = 0.0);
};

///////////////////////////////////////////////////////////////////////

class DemoEffect : public Node<DemoEffect>, public Managed<DemoEffect>
{
  friend class Demo;
public:
  DemoEffect(const String& name = "",
             DemoEffectType* initType = NULL,
	     Time initDuration = 0.0);
  bool isActive(void) const;
  DemoEffectType* getType(void) const;
  Time getDuration(void) const;
  Time getTimeElapsed(void) const;
protected:
  void prepareChildren(void) const;
  void renderChildren(void) const; 
  virtual void prepare(void) const;
  virtual void render(void) const;
  virtual void update(Time deltaTime);
  virtual void trigger(Time moment, const String& name, const String& value);
  virtual void restart(void);
private:
  DemoEffectType* type;
  Time duration;
  Time elapsed;
  bool active;
};

///////////////////////////////////////////////////////////////////////

class NullEffect : public DemoEffect
{
public:
  NullEffect(const String& name = "",
             DemoEffectType* type = NULL,
	     Time duration = 0.0);
  bool init(void);
};

///////////////////////////////////////////////////////////////////////

class ClearEffect : public DemoEffect
{
public:
  ClearEffect(const String& name = "",
              DemoEffectType* type = NULL,
	      Time duration = 0.0);
  bool init(void);
private:
  void render(void) const;
  void trigger(Time moment, const String& name, const String& value);
  void restart(void);
  ColorRGBA color;
};

///////////////////////////////////////////////////////////////////////

class Demo
{
public:
  ~Demo(void);
  bool addEffect(const String& instanceName,
                 const String& typeName,
                 Time start,
                 Time duration,
		 const String& parentName = "");
  bool addEffectEvent(const String& instanceName,
		      const String& eventName,
		      const String& eventValue,
		      Time moment);
  bool createContext(void);
  bool createEffectInstances(void);
  void destroyEffectInstances(void);
  void render(void) const;
  const ContextMode& getContextMode(void) const;
  void setContextMode(const ContextMode& newMode);
  const String& getTitle(void) const;
  Time getDuration(void) const;
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time time);
  DemoEffect* getRootEffect(void);
  static Demo* createInstance(const String& title);
  static Demo* createInstance(const Path& path);
private:
  class Event
  {
  public:
    String name;
    String value;
    Time moment;
  };
  class Effect : public Node<Effect>
  {
  public:
    typedef std::vector<Event> EventList;
    String instanceName;
    String typeName;
    Time start;
    Time duration;
    Ptr<DemoEffect> instance;
    EventList events;
  };
  typedef std::map<String, Effect*> EffectMap;
  Demo(const String& initTitle);
  bool init(void);
  Effect* findEffect(const String& name);
  void updateEffect(Effect& effect, Time newTime);
  bool createEffectInstance(Effect& effect);
  void destroyEffectInstance(Effect& effect);
  Effect rootEffect;
  EffectMap effectMap;
  ContextMode contextMode;
  String title;
};

///////////////////////////////////////////////////////////////////////

class DemoReader : public XML::Reader
{
public:
  Demo* read(const Path& path);
  Demo* read(Stream& stream);
private:
  bool beginElement(const String& name);
  bool endElement(const String& name);
  Ptr<Demo> demo;
  std::stack<String> effectNameStack;
};

///////////////////////////////////////////////////////////////////////

class DemoWriter : public XML::Writer
{
public:
  bool write(const Path& path, const Demo& demo);
  bool write(Stream& stream, const Demo& demo);
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline DemoEffectTemplate<T>::DemoEffectTemplate(const String& name):
  DemoEffectType(name)
{
}

template <typename T>
inline DemoEffect* DemoEffectTemplate<T>::createEffect(const String& name, Time duration)
{
  Ptr<T> effect = new T(name, this, duration);
  if (!effect->init())
    return NULL;

  return effect.detachObject();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLDEMO_H*/
///////////////////////////////////////////////////////////////////////

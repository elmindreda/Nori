///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@home.se>
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
  DemoEffectType(const std::string& name);
  /*! Creates an instance of this effect type.
   *  @param name [in] The desired name of the created effect object.
   *  @return The newly created effect object, or @c NULL.
   */
  virtual DemoEffect* createEffect(const std::string& name = "", Time duration = 0.0) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! Template helper for automatic effect types.
 */
template <typename T>
class DemoEffectTemplate : public DemoEffectType
{
public:
  inline DemoEffectTemplate(const std::string& name);
  inline DemoEffect* createEffect(const std::string& name = "", Time duration = 0.0);
};

///////////////////////////////////////////////////////////////////////

class DemoEffect : public Node<DemoEffect>, public Managed<DemoEffect>
{
  friend class Demo;
public:
  DemoEffect(const std::string& name = "",
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
  virtual void trigger(Time moment, const std::string& name, const std::string& value);
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
  NullEffect(const std::string& name = "", DemoEffectType* type = NULL, Time duration = 0.0);
  bool init(void);
};

///////////////////////////////////////////////////////////////////////

class ClearEffect : public DemoEffect
{
public:
  ClearEffect(const std::string& name = "", DemoEffectType* type = NULL, Time duration = 0.0);
  bool init(void);
private:
  void render(void) const;
  void trigger(Time moment, const std::string& name, const std::string& value);
  void restart(void);
  ColorRGBA color;
};

///////////////////////////////////////////////////////////////////////

class Demo
{
public:
  ~Demo(void);
  bool addEffect(const std::string& instanceName,
                 const std::string& typeName,
                 Time start,
                 Time duration,
		 const std::string& parentName = "");
  bool addEffectEvent(const std::string& instanceName,
		      const std::string& eventName,
		      const std::string& eventValue,
		      Time moment);
  bool createContext(void);
  bool createEffectInstances(void);
  void destroyEffectInstances(void);
  void render(void) const;
  const ContextMode& getContextMode(void) const;
  void setContextMode(const ContextMode& newMode);
  const std::string& getTitle(void) const;
  Time getDuration(void) const;
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time time);
  DemoEffect* getRootEffect(void);
  static Demo* createInstance(const std::string& title);
  static Demo* createInstance(const Path& path);
private:
  class Event
  {
  public:
    std::string name;
    std::string value;
    Time moment;
  };
  class Effect : public Node<Effect>
  {
  public:
    typedef std::vector<Event> EventList;
    std::string instanceName;
    std::string typeName;
    Time start;
    Time duration;
    Ptr<DemoEffect> instance;
    EventList events;
  };
  typedef std::map<std::string, Effect*> EffectMap;
  Demo(const std::string& initTitle);
  bool init(void);
  Effect* findEffect(const std::string& name);
  void updateEffect(Effect& effect, Time newTime);
  bool createEffectInstance(Effect& effect);
  void destroyEffectInstance(Effect& effect);
  Effect rootEffect;
  EffectMap effectMap;
  ContextMode contextMode;
  std::string title;
};

///////////////////////////////////////////////////////////////////////

class DemoReader : public XML::Reader
{
public:
  Demo* read(const Path& path);
  Demo* read(Stream& stream);
private:
  bool beginElement(const std::string& name, const AttributeMap& attributes);
  bool endElement(const std::string& name);
  Ptr<Demo> demo;
  std::stack<std::string> effectNameStack;
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
inline DemoEffectTemplate<T>::DemoEffectTemplate(const std::string& name):
  DemoEffectType(name)
{
}

template <typename T>
inline DemoEffect* DemoEffectTemplate<T>::createEffect(const std::string& name, Time duration)
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

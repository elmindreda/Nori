///////////////////////////////////////////////////////////////////////
// Wendy demo system
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <vector>
#include <stack>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class ShowEvent
{
public:
  String name;
  String value;
  Time moment;
};

///////////////////////////////////////////////////////////////////////

class ShowEffect : public Node<ShowEffect>
{
public:
  typedef std::vector<Event> EventList;
  String instanceName;
  String typeName;
  Time start;
  Time duration;
  EventList events;
  Ptr<Effect> instance;
};

///////////////////////////////////////////////////////////////////////

class Show : public Resource<Show>
{
public:
  ~Player(void);
  bool addEffect(const String& instanceName,
                 const String& typeName,
                 Time start,
                 Time duration,
		 const String& parentName = "");
  bool addEffectEvent(const String& instanceName,
		      const String& eventName,
		      const String& eventValue,
		      Time moment);
  bool createEffectInstances(void);
  void destroyEffectInstances(void);
  void render(void) const;
  const String& getTitle(void) const;
  void setTitle(const String& newTitle);
  Time getDuration(void) const;
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time time);
  Effect* getRootEffect(void);
  static Player* createInstance(const String& name = "");
private:
  typedef std::map<String, ShowEffect*> EffectMap;
  Player(const String& name);
  bool init(void);
  Effect* findEffect(const String& name);
  void updateEffect(Effect& effect, Time newTime);
  bool createEffectInstance(Effect& effect);
  void destroyEffectInstance(Effect& effect);
  Effect rootEffect;
  EffectMap effectMap;
  String title;
};

///////////////////////////////////////////////////////////////////////

class ShowCodecXML : public ResourceCodec<Show>, public XML::Codec
{
public:
  DemoCodecXML(void);
  Demo* read(const Path& path, const String& name = "");
  Demo* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Demo& demo);
  bool write(Stream& stream, const Demo& demo);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<Demo> demo;
  std::stack<String> effectNameStack;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOEFFECT_H*/
///////////////////////////////////////////////////////////////////////

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
//
#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>

#include <wendy/DemoEffect.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

EffectType::EffectType(const String& name):
  Managed<EffectType>(name)
{
}

///////////////////////////////////////////////////////////////////////

Effect::Effect(EffectType& initType,
               const String& name):
  Managed<Effect>(name),
  type(initType),
  duration(0.0),
  elapsed(0.0),
  active(false)
{
}

bool Effect::createChild(const String& typeName,
                         const String& instanceName)
{
  EffectType* type = EffectType::findInstance(typeName);
  if (!type)
  {
    Log::writeError("Cannot find effect type %s", typeName.c_str());
    return false;
  }

  Effect* instance = type->createEffect(instanceName);
  if (!instance)
    return false;

  addChild(*instance);
  return true;
}

bool Effect::isActive(void) const
{
  return active;
}

EffectType& Effect::getType(void) const
{
  return type;
}

Time Effect::getStartTime(void) const
{
  return start;
}

void Effect::setStartTime(Time newTime)
{
  start = newTime;
}

Time Effect::getDuration(void) const
{
  return duration;
}

void Effect::setDuration(Time newDuration)
{
  duration = newDuration;
}

Time Effect::getTimeElapsed(void) const
{
  return elapsed;
}

void Effect::prepareChildren(void) const
{
  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if ((*i)->isActive())
      (*i)->prepare();
  }
}

void Effect::renderChildren(void) const
{
  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if ((*i)->isActive())
      (*i)->render();
  }
}

void Effect::prepare(void) const
{
  prepareChildren();
}

void Effect::render(void) const
{
  renderChildren();
}

void Effect::update(Time deltaTime)
{
}

void Effect::restart(void)
{
}

///////////////////////////////////////////////////////////////////////

NullEffect::NullEffect(EffectType& type, const String& name):
  Effect(type, name)
{
}

bool NullEffect::init(void)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

ClearEffect::ClearEffect(EffectType& type, const String& name):
  Effect(type, name)
{
}

bool ClearEffect::init(void)
{
  return true;
}

void ClearEffect::render(void) const
{
  GL::Canvas* canvas = GL::Canvas::getCurrent();
  
  canvas->clearColorBuffer(ColorRGBA::BLACK);
  canvas->clearDepthBuffer();

  renderChildren();
}

///////////////////////////////////////////////////////////////////////

/*
Demo::~Demo(void)
{
  destroyEffectInstances();
}

bool Demo::addEffect(const String& instanceName,
                     const String& typeName,
                     Time start,
                     Time duration,
		     const String& parentName)
{
  if (!DemoEffectType::findInstance(typeName))
  {
    Log::writeError("Effect type %s does not exist", typeName.c_str());
    return false;
  }

  if (findEffect(instanceName))
  {
    Log::writeError("Duplicate effect instance name %s", instanceName.c_str());
    return false;
  }

  Effect* parent = findEffect(parentName.empty() ? "root" : parentName);
  if (!parent)
  {
    Log::writeError("Parent effect %s does not exist", parentName.c_str());
    return false;
  }

  if (parent->instanceName == "root")
  {
    if (start + duration > parent->duration)
      parent->duration = start + duration;
  }

  Effect* effect = new Effect();
  parent->addChildLast(*effect);
  effectMap[instanceName] = effect;

  effect->instanceName = instanceName;
  effect->typeName = typeName;
  effect->start = start;
  effect->duration = duration;

  return true;
}

bool Demo::addEffectEvent(const String& instanceName,
			  const String& eventName,
			  const String& eventValue,
			  Time moment)
{
  Effect* effect = findEffect(instanceName);
  if (!effect)
  {
    Log::writeError("Effect instance %s does not exist", instanceName.c_str());
    return false;
  }

  effect->events.push_back(Event());
  Event& event = effect->events.back();

  event.name = eventName;
  event.value = eventValue;
  event.moment = moment;
  return true;
}

bool Demo::createContext(void)
{
  if (!Context::create(contextMode))
    return false;

  Context::get()->setTitle(title);
  return true;
}

bool Demo::createEffectInstances(void)
{
  destroyEffectInstances();

  return createEffectInstance(rootEffect);
}

void Demo::destroyEffectInstances(void)
{
  destroyEffectInstance(rootEffect);
}

void Demo::render(void) const
{
  if (const Effect* instance = rootEffect.instance)
  {
    instance->prepare();
    instance->render();
  }
}

const ContextMode& Demo::getContextMode(void) const
{
  return contextMode;
}

void Demo::setContextMode(const ContextMode& newMode)
{
  contextMode = newMode;
}

const String& Demo::getTitle(void) const
{
  return title;
}

void Demo::setTitle(const String& newTitle)
{
  title = newTitle;
}

Time Demo::getDuration(void) const
{
  return rootEffect.duration;
}

Time Demo::getTimeElapsed(void) const
{
  if (rootEffect.instance)
    return rootEffect.instance->getTimeElapsed();

  return 0.f;
}

void Demo::setTimeElapsed(Time newTime)
{
  if (rootEffect.instance)
    updateEffect(rootEffect, newTime);
}

Demo* Demo::createInstance(const String& title)
{
  Ptr<Demo> demo = new Demo(title);
  if (!demo->init())
    return NULL;

  return demo.detachObject();
}

Demo::Demo(const String& name):
  Resource<Demo>(name)
{
}

bool Demo::init(void)
{
  if (!EffectType::findInstance("null"))
    new EffectTemplate<NullEffect>("null");

  if (!EffectType::findInstance("clear"))
    new EffectTemplate<ClearEffect>("clear");

  rootEffect.instanceName = "root";
  rootEffect.typeName = "null";
  rootEffect.start = 0.f;
  rootEffect.duration = 0.f;

  effectMap[rootEffect.instanceName] = &rootEffect;

  return true;
}

void Demo::updateEffect(Effect& effect, Time newTime)
{
  Time currentTime = effect.start + effect.instance->elapsed;
  Time deltaTime = newTime - currentTime;

  if (newTime == currentTime)
    return;

  if ((currentTime == effect.start) || (newTime < currentTime))
  {
    effect.instance->restart();
    effect.instance->active = false;
    effect.instance->elapsed = 0.0;
    currentTime = 0.0;
  }

  if (effect.instance->active)
  {
    if (newTime > effect.start + effect.duration)
    {
      effect.instance->active = false;
      effect.instance->elapsed = effect.duration;
    }
  }
  else
  {
    if ((currentTime == 0.0 && effect.start == 0.0) ||
	(currentTime < effect.start &&
	 newTime >= effect.start &&
	 newTime <= effect.start + effect.duration))
    {
      effect.instance->active = true;
    }
  }

  if (effect.instance->active)
  {
    effect.instance->elapsed = newTime - effect.start;

    // TODO: Replay events in proper order (sort them beforehand)

    for (Effect::EventList::const_iterator event = effect.events.begin();  event != effect.events.end();  event++)
    {
      if ((effect.start + (*event).moment >= currentTime) && (effect.start + (*event).moment < newTime))
	effect.instance->trigger((*event).moment, (*event).name, (*event).value);
    }
      
    effect.instance->update(deltaTime);

    for (Effect* child = effect.getFirstChild();  child != NULL;  child = child->getNextSibling())
      updateEffect(*child, newTime - effect.start);
  }
}

bool Demo::createEffectInstance(Effect& effect)
{
  EffectType* type = EffectType::findInstance(effect.typeName);
  if (!type)
  {
    Log::writeError("Effect type %s does not exist", effect.typeName.c_str());
    return false;
  }

  try
  {
    effect.instance = type->createEffect(effect.instanceName, effect.duration);
    if (!effect.instance)
    {
      Log::writeError("Failed to create effect %s of type %s",
                      effect.instanceName.c_str(),
		      effect.typeName.c_str());
      return false;
    }
  }
  catch (const Exception& e)
  {
    Log::writeError("Effect instance creation failed: %s", e.what());
    return false;
  }

  if (Effect* parent = effect.getParent())
  {
    if (!parent->instance->addChildLast(*effect.instance))
    {
      Log::writeError("Effect tree loop detected. This is impossible. Have a nice day.");
      return false;
    }
  }

  for (Effect* child = effect.getFirstChild();  child != NULL;  child = child->getNextSibling())
  {
    if (!createEffectInstance(*child))
      return false;
  }

  return true;
}

void Demo::destroyEffectInstance(Effect& effect)
{
  for (Effect* child = effect.getFirstChild();  child != NULL;  child = child->getNextSibling())
    destroyEffectInstance(*child);

  effect.instance = NULL;
}

Demo::Effect* Demo::findEffect(const String& name)
{
  EffectMap::iterator i = effectMap.find(name);
  if (i == effectMap.end())
    return NULL;

  return (*i).second;
}
*/

///////////////////////////////////////////////////////////////////////

/*
CodecXML::CodecXML(void):
  ResourceCodec<Demo>("Demo XML codec")
{
  addSuffix("demo");
  addSuffix("xml");
}

Demo* CodecXML::read(const Path& path, const String& name)
{
  return ResourceCodec<Demo>::read(path, name);
}

Demo* CodecXML::read(Stream& stream, const String& name)
{
  while (!effectNameStack.empty())
    effectNameStack.pop();

  if (!Reader::read(stream))
    return NULL;

  return demo.detachObject();
}

bool CodecXML::write(const Path& path, const Demo& demo)
{
  return ResourceCodec<Demo>::write(path, demo);
}

bool DemoCodecXML::write(Stream& stream, const Demo& demo)
{
  // TODO: Implement.

  return false;
}

bool DemoCodecXML::onBeginElement(const String& name)
{
  if (name == "demo")
  {
    demo = Demo::createInstance();
    if (!demo)
      return false;

    demo->setTitle(readString("title"));
    return true;
  }

  if (demo)
  {
    if (name == "context")
    {
      ContextMode mode;

      mode.width = readInteger("width", 640);
      mode.height = readInteger("height", 480);

      mode.colorBits = readInteger("color", 24); 
      mode.depthBits = readInteger("depth", 32); 
      mode.stencilBits = readInteger("stencil", 0); 

      if (readBoolean("windowed", true))
        mode.flags |= ContextMode::WINDOWED;

      demo->setContextMode(mode);
      return true;
    }

    if (name == "effect")
    {
      String instanceName = readString("name");

      String parentName;
      if (!effectNameStack.empty())
	parentName = effectNameStack.top();

      if (!demo->addEffect(instanceName,
                           readString("type"),
                           readFloat("start"),
                           readFloat("duration"),
			   parentName))
        return false;

      effectNameStack.push(instanceName);
      return true;
    }

    if (!effectNameStack.empty())
    {
      if (name == "event")
      {
        if (!demo->addEffectEvent(effectNameStack.top(),
				  readString("name"),
				  readString("value"),
				  readFloat("moment")))
          return false;

        return true;
      }
    }
  }
  
  return true;
}

bool DemoCodecXML::onEndElement(const String& name)
{
  if (demo)
  {
    if (name == "effect")
      effectNameStack.pop();
  }
  
  return true;
}
*/

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

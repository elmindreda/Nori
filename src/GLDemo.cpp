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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Node.h>
#include <moira/Log.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Point.h>
#include <moira/Stream.h>
#include <moira/XML.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLDemo.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

DemoEffectType::DemoEffectType(const std::string& name):
  Managed<DemoEffectType>(name)
{
}

///////////////////////////////////////////////////////////////////////

DemoEffect::DemoEffect(const std::string& name, DemoEffectType* initType):
  Managed<DemoEffect>(name),
  type(initType),
  elapsed(0.f),
  duration(0.f),
  active(false)
{
}

bool DemoEffect::isActive(void) const
{
  return active;
}

DemoEffectType* DemoEffect::getType(void) const
{
  return type;
}

Time DemoEffect::getDuration(void) const
{
  return duration;
}

Time DemoEffect::getTimeElapsed(void) const
{
  return elapsed;
}

void DemoEffect::prepareChildren(void) const
{
  for (const DemoEffect* child = getFirstChild();  child != NULL;  child = child->getNextSibling())
    child->prepare();
}

void DemoEffect::renderChildren(void) const
{
  for (const DemoEffect* child = getFirstChild();  child != NULL;  child = child->getNextSibling())
    child->render();
}

void DemoEffect::prepare(void) const
{
  prepareChildren();
}

void DemoEffect::render(void) const
{
  renderChildren();
}

void DemoEffect::update(Time deltaTime)
{
}

void DemoEffect::trigger(const std::string& name, const std::string& value)
{
}

void DemoEffect::restart(void)
{
}

///////////////////////////////////////////////////////////////////////

bool Demo::addEffect(const std::string& instanceName,
                     const std::string& typeName,
                     Time start,
                     Time duration,
		     const std::string& parentName)
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
    Log::writeError("Parent effect %s does not exist", instanceName.c_str());
    return false;
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

bool Demo::addEffectEvent(const std::string& instanceName,
			  const std::string& eventName,
			  const std::string& eventValue,
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
  return createEffectInstance(rootEffect);
}

void Demo::destroyEffectInstances(void)
{
  destroyEffectInstance(rootEffect);
}

const ContextMode& Demo::getContextMode(void) const
{
  return contextMode;
}

void Demo::setContextMode(const ContextMode& newMode)
{
  contextMode = newMode;
}

const std::string& Demo::getTitle(void) const
{
  return title;
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

Demo* Demo::createInstance(const std::string& title)
{
  return new Demo(title);
}

Demo* Demo::createInstance(const Path& path)
{
  DemoReader reader;
  return reader.read(path);
}

Demo::Demo(const std::string& initTitle):
  title(initTitle)
{
}

void Demo::updateEffect(Effect& effect, Time newTime)
{
  const Time currentTime = effect.instance->elapsed;
  const Time deltaTime = newTime - currentTime;

  if (newTime == currentTime)
    return;

  if (currentTime == 0.f || newTime < currentTime)
    effect.instance->restart();

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
    if ((currentTime == effect.start == 0.f) ||
	(currentTime < effect.start &&
	 newTime >= effect.start &&
	 newTime <= effect.start + effect.duration))
    {
      effect.instance->active = true;
    }
  }

  if (effect.instance->active)
  {
    effect.instance->elapsed = newTime;

    // TODO: Replay events in proper order (sort them beforehand)

    for (Effect::EventList::const_iterator event = effect.events.begin();  event != effect.events.end();  event++)
    {
      if ((*event).moment >= currentTime && (*event).moment < newTime)
	effect.instance->trigger((*event).name, (*event).value);
    }
      
    effect.instance->update(deltaTime);
  }

  for (Effect* child = effect.getFirstChild();  child != NULL;  child = child->getNextSibling())
    updateEffect(*child, newTime - effect.start);
}

bool Demo::createEffectInstance(Effect& effect)
{
  DemoEffectType* type = DemoEffectType::findInstance(effect.typeName);
  if (!type)
  {
    Log::writeError("Effect type %s does not exist", effect.typeName.c_str());
    return false;
  }

  effect.instance = type->createEffect(effect.instanceName);
  if (!effect.instance)
    return false;

  if (Effect* parent = effect.getParent())
  {
    if (!parent->instance->addChildLast(*effect.instance))
    {
      Log::writeError("Effect tree loop detected. This is impossible. Have a nice day.");
      return false;
    }
  }

  for (Effect* child = effect.getFirstChild();  child != NULL;  child = child->getNextSibling())
    createEffectInstance(*child);

  return true;
}

void Demo::destroyEffectInstance(Effect& effect)
{
  for (Effect* child = effect.getFirstChild();  child != NULL;  child = child->getNextSibling())
    destroyEffectInstance(*child);

  effect.instance = NULL;
}

Demo::Effect* Demo::findEffect(const std::string& name)
{
  EffectMap::iterator i = effectMap.find(name);
  if (i == effectMap.end())
    return NULL;

  return (*i).second;
}

///////////////////////////////////////////////////////////////////////

Demo* DemoReader::read(const Path& path)
{
  Ptr<Stream> file = FileStream::createInstance(path, Stream::READABLE);
  if (!file)
    return false;

  return read(*file);
}

Demo* DemoReader::read(Stream& stream)
{
  while (!effectNameStack.empty())
    effectNameStack.pop();

  if (!Reader::read(stream))
    return NULL;

  return demo.detachObject();
}

bool DemoReader::beginElement(const std::string& name, const AttributeMap& attributes)
{
  if (name == "demo")
  {
    demo = Demo::createInstance(readString("title", attributes));
    return true;
  }

  if (demo)
  {
    if (name == "context")
    {
      ContextMode mode;

      mode.width = readInteger("width", attributes, 640);
      mode.height = readInteger("height", attributes, 480);

      mode.colorBits = readInteger("color", attributes, 24); 
      mode.depthBits = readInteger("depth", attributes, 32); 
      mode.stencilBits = readInteger("stencil", attributes, 0); 

      if (readBoolean("windowed", attributes, true))
        mode.flags |= ContextMode::WINDOWED;

      demo->setContextMode(mode);
      return true;
    }

    if (name == "effect")
    {
      std::string instanceName = readString("name", attributes);

      std::string parentName;
      if (!effectNameStack.empty())
	parentName = effectNameStack.top();

      if (!demo->addEffect(instanceName,
                           readString("type", attributes),
                           readFloat("start", attributes),
                           readFloat("duration", attributes),
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
				  readString("name", attributes),
				  readString("value", attributes),
				  readFloat("moment", attributes)))
          return false;

        return true;
      }
    }
  }
  
  return true;
}

bool DemoReader::endElement(const std::string& name)
{
  if (demo)
  {
    if (name == "effect")
      effectNameStack.pop();
  }
  
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Wendy demo system
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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>

#include <wendy/DemoEffect.h>
#include <wendy/DemoShow.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

void Show::render(void) const
{
  if (root)
    root->render();
}

Effect& Show::getRootEffect(void)
{
  return *root;
}

const String& Show::getTitle(void) const
{
  return title;
}

void Show::setTitle(const String& newTitle)
{
  title = newTitle;
}

Time Show::getDuration(void) const
{
  if (root)
    return root->getDuration();

  return 0.f;
}

Time Show::getTimeElapsed(void) const
{
  if (root)
    return root->getTimeElapsed();

  return 0.f;
}

void Show::setTimeElapsed(Time newTime)
{
  if (root)
    updateEffect(*root, newTime);
}

bool Show::create(void)
{
  if (get())
    return true;

  Ptr<Show> show = new Show();
  if (!show->init())
    return false;

  set(show.detachObject());
  return true;
}

Show::Show(void)
{
}

bool Show::init(void)
{
  // Create built-in effect types
  {
    if (!EffectType::findInstance("null"))
      new EffectTemplate<NullEffect>("null");

    if (!EffectType::findInstance("clear"))
      new EffectTemplate<ClearEffect>("clear");
  }

  root = EffectType::findInstance("null")->createEffect("root");

  return true;
}

void Show::updateEffect(Effect& effect, Time newTime)
{
  Time currentTime = effect.start + effect.elapsed;
  Time deltaTime = newTime - currentTime;

  if (newTime == currentTime)
    return;

  if ((currentTime == effect.start) || (newTime < currentTime))
  {
    effect.restart();
    effect.active = false;
    effect.elapsed = 0.0;
    currentTime = 0.0;
  }

  if (effect.active)
  {
    if (newTime > effect.start + effect.duration)
    {
      effect.active = false;
      effect.elapsed = effect.duration;
    }
  }
  else
  {
    if ((currentTime == 0.0 && effect.start == 0.0) ||
	(currentTime < effect.start &&
	 newTime >= effect.start &&
	 newTime <= effect.start + effect.duration))
    {
      effect.active = true;
    }
  }

  if (effect.active)
  {
    effect.elapsed = newTime - effect.start;

    // TODO: Replay events in proper order (sort them beforehand)

    /*
    for (Effect::EventList::const_iterator event = effect.events.begin();  event != effect.events.end();  event++)
    {
      if ((effect.start + (*event).moment >= currentTime) && (effect.start + (*event).moment < newTime))
	effect.instance->trigger((*event).moment, (*event).name, (*event).value);
    }
    */
      
    effect.update(deltaTime);

    const Effect::List& children = effect.getChildren();

    for (Effect::List::const_iterator i = children.begin();  i != children.end();  i++)
      updateEffect(**i, newTime - effect.start);
  }
}

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

///////////////////////////////////////////////////////////////////////
// Wendy demo system
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/XML.h>

#include <wendy/DemoProperty.h>
#include <wendy/DemoEffect.h>
#include <wendy/DemoShow.h>

#include <internal/DemoShowIO.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {

///////////////////////////////////////////////////////////////////////

ShowCodec::ShowCodec(void):
  ResourceCodec<Show>("Demo show XML codec")
{
  addSuffix("show");
}

Show* ShowCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<Show>::read(path, name);
}

Show* ShowCodec::read(Stream& stream, const String& name)
{
  while (!effectStack.empty())
    effectStack.pop();

  if (!Reader::read(stream))
    return NULL;

  return show.detachObject();
}

bool ShowCodec::write(const Path& path, const Show& show)
{
  return ResourceCodec<Show>::write(path, show);
}

bool ShowCodec::write(Stream& stream, const Show& show)
{
  try
  {
    setStream(&stream);

    beginElement("show");
    addAttribute("title", show.getTitle());
    addAttribute("music", show.getMusicPath().asString());

    const Effect::List& effects = show.getEffects();
    for (Effect::List::const_iterator i = effects.begin();  i != effects.end();  i++)
      writeEffect(**i);

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write demo show \'%s\': %s",
                    show.getName().c_str(),
		    exception.what());
    return false;
  }

  return true;
}

bool ShowCodec::onBeginElement(const String& name)
{
  if (name == "show")
  {
    show = Show::createInstance();
    if (!show)
      return false;

    show->setTitle(readString("title"));
    show->setMusicPath(Path(readString("music")));
    return true;
  }

  if (show)
  {
    if (name == "effect")
    {
      String typeName = readString("type");

      EffectType* type = EffectType::findInstance(typeName);
      if (!type)
      {
	Log::writeError("Effect type \'%s\' does not exist", typeName.c_str());
	return false;
      }

      Effect* effect = type->createEffect(readString("name"));
      if (!effect)
	return false;

      effect->setStartTime(readFloat("start"));
      effect->setDuration(readFloat("duration"));

      if (effectStack.empty())
        show->addEffect(*effect);
      else
        effectStack.top()->addChild(*effect);

      effectStack.push(effect);
      return true;
    }

    if (!effectStack.empty())
    {
      Effect* effect = effectStack.top();

      if (name == "property")
      {
	String propertyName = readString("name");

	currentProperty = effect->findProperty(propertyName);
	if (!currentProperty)
	{
	  Log::writeError("Effect \'%s\' does not have property %s",
	                  effect->getName().c_str(),
	                  propertyName.c_str());
	  return false;
	}

        return true;
      }

      if (currentProperty)
      {
	if (name == "key")
	{
	  currentProperty->createKey(readFloat("moment"), readString("value"));
	  return true;
	}
      }
    }
  }

  return true;
}

bool ShowCodec::onEndElement(const String& name)
{
  if (show)
  {
    if (name == "effect")
      effectStack.pop();

    if (name == "property")
      currentProperty = NULL;
  }

  return true;
}

void ShowCodec::writeEffect(const Effect& effect)
{
  beginElement("effect");
  addAttribute("name", effect.getName());
  addAttribute("type", effect.getType().getName());
  addAttribute("start", effect.getStartTime());
  addAttribute("duration", effect.getDuration());

  const Effect::PropertyList& properties = effect.getProperties();

  for (Effect::PropertyList::const_iterator p = properties.begin();  p != properties.end();  p++)
  {
    beginElement("property");
    addAttribute("name", (*p)->getName());

    const Property::KeyList& keys = (*p)->getKeys();

    for (Property::KeyList::const_iterator k = keys.begin();  k != keys.end();  k++)
    {
      beginElement("key");
      addAttribute("moment", (float) (*k)->getMoment());
      addAttribute("value", (*k)->asString());
      endElement();
    }

    endElement();
  }

  const Effect::List& children = effect.getChildren();

  for (Effect::List::const_iterator i = children.begin();  i != children.end();  i++)
    writeEffect(**i);

  endElement();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

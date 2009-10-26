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

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLProgram.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UISlider.h>

#include <wendy/DemoProperty.h>
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

void Show::prepare(void) const
{
  root->prepare();
}

void Show::render(void) const
{
  root->render();
}

const String& Show::getTitle(void) const
{
  return title;
}

void Show::setTitle(const String& newTitle)
{
  title = newTitle;
}

const Path& Show::getMusicPath(void) const
{
  return musicPath;
}

void Show::setMusicPath(const Path& newPath)
{
  musicPath = newPath;
}

Time Show::getDuration(void) const
{
  Time duration = 0.0;

  const Effect::List& children = root->getChildren();

  for (Effect::List::const_iterator i = children.begin();  i != children.end();  i++)
    duration = std::max(duration, (*i)->start + (*i)->duration);

  return duration;
}

Time Show::getTimeElapsed(void) const
{
  return root->getTimeElapsed();
}

void Show::setTimeElapsed(Time newTime)
{
  newTime = std::max(newTime, 0.0);

  // TODO: Remove this hack.
  root->setDuration(getDuration());

  updateEffect(*root, newTime);
}

Effect& Show::getRootEffect(void)
{
  return *root;
}

const Effect& Show::getRootEffect(void) const
{
  return *root;
}

Show* Show::createInstance(const String& name)
{
  Ptr<Show> show(new Show(name));
  if (!show->init())
    return NULL;

  return show.detachObject();
}

Show::Show(const String& name):
  Resource<Show>(name)
{
}

bool Show::init(void)
{
  // Create built-in effect types
  {
    String name;

    name = "Null effect";

    if (!EffectType::findInstance(name))
      new EffectTemplate<NullEffect>(name);

    name = "Clear screen";

    if (!EffectType::findInstance(name))
      new EffectTemplate<ClearEffect>(name);
  }

  root = EffectType::findInstance("Null effect")->createEffect("Root effect");
  if (!root)
  {
    Log::writeError("Failed to create root effect");
    return false;
  }

  title = "Demo";
  return true;
}

void Show::updateEffect(Effect& effect, Time newTime)
{
  Time currentTime = effect.start + effect.elapsed;

  if (newTime < currentTime)
  {
    effect.restart();
    effect.active = false;
    effect.updated = false;
    effect.elapsed = 0.0;

    currentTime = effect.start;
  }

  if (effect.active)
  {
    if (newTime > effect.start + effect.duration)
    {
      deactivateChildren(effect);
      effect.active = false;
      effect.elapsed = effect.duration;
    }
  }
  else
  {
    if (newTime >= effect.start && newTime <= effect.start + effect.duration)
      effect.active = true;
  }

  if (effect.active)
  {
    const Time maxDelta = 1.0;
    Time deltaTime = newTime - currentTime;

    while (!effect.updated || deltaTime > 0.0)
    {
      Time localDelta = std::min(deltaTime, maxDelta);

      effect.elapsed += localDelta;
      effect.update(localDelta);
      effect.updated = true;

      deltaTime -= localDelta;
    }

    const Effect::List& children = effect.getChildren();

    for (Effect::List::const_iterator i = children.begin();  i != children.end();  i++)
      updateEffect(**i, newTime - effect.start);
  }
}

void Show::deactivateChildren(Effect& effect)
{
  const Effect::List& children = effect.getChildren();

  for (Effect::List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    deactivateChildren(**i);
    (*i)->active = false;
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

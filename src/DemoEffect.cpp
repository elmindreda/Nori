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

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <wendy/DemoProperty.h>
#include <wendy/DemoEffect.h>
#include <wendy/DemoShow.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {

///////////////////////////////////////////////////////////////////////

EffectType::EffectType(const String& name):
  Managed<EffectType>(name)
{
}

///////////////////////////////////////////////////////////////////////

Effect::Effect(EffectType& initType, const String& initName):
  type(initType),
  name(initName),
  show(NULL),
  parent(NULL),
  active(false),
  updated(false),
  start(0.0),
  duration(0.0),
  elapsed(0.0)
{
}

Effect::~Effect(void)
{
  destroyChildren();
  removeFromParent();

  while (!properties.empty())
    delete properties.back();
}

bool Effect::addChild(Effect& child)
{
  if (isChildOf(child))
    return false;

  child.removeFromParent();

  children.push_back(&child);
  child.parent = this;
  child.setShow(show);

  return true;
}

void Effect::destroyChildren(void)
{
  while (!children.empty())
    delete children.back();
}

void Effect::removeFromParent(void)
{
  if (parent)
  {
    List& siblings = parent->children;
    siblings.erase(std::find(siblings.begin(), siblings.end(), this));

    parent = NULL;
  }
  else if (show)
  {
    List& effects = show->effects;
    effects.erase(std::find(effects.begin(), effects.end(), this));
  }

  setShow(NULL);
}

Property* Effect::findProperty(const String& name)
{
  for (PropertyList::const_iterator i = properties.begin();  i != properties.end();  i++)
  {
    if ((*i)->getName() == name)
      return *i;
  }

  return NULL;
}

bool Effect::isActive(void) const
{
  return active;
}

bool Effect::isChildOf(const Effect& effect) const
{
  if (parent != NULL)
  {
    if (parent == &effect)
      return true;

    return parent->isChildOf(effect);
  }

  return false;
}

EffectType& Effect::getType(void) const
{
  return type;
}

const String& Effect::getName(void) const
{
  return name;
}

Show* Effect::getShow(void) const
{
  return show;
}

Effect* Effect::getParent(void) const
{
  return parent;
}

const Effect::List& Effect::getChildren(void) const
{
  return children;
}

Time Effect::getGlobalOffset(void) const
{
  Time offset = 0.0;

  const Effect* effect = this;
  while (effect)
  {
    offset += effect->getStartTime();
    effect = effect->getParent();
  }

  return offset;
}

Time Effect::getStartTime(void) const
{
  return start;
}

void Effect::setStartTime(Time newTime)
{
  if (newTime < start)
    restart();

  start = newTime;
  updated = false;
}

Time Effect::getDuration(void) const
{
  return duration;
}

void Effect::setDuration(Time newDuration)
{
  duration = newDuration;
  updated = false;
}

Time Effect::getTimeElapsed(void) const
{
  return elapsed;
}

const Effect::PropertyList& Effect::getProperties(void) const
{
  return properties;
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

void Effect::setShow(Show* newShow)
{
  show = newShow;

  for (List::const_iterator c = children.begin();  c != children.end();  c++)
    (*c)->setShow(show);
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
  Effect(type, name),
  color(*this, "Color")
{
}

bool ClearEffect::init(void)
{
  return true;
}

void ClearEffect::render(void) const
{
  GL::Context* context = GL::Context::get();

  context->clearColorBuffer(color.getValue(getTimeElapsed()));
  context->clearDepthBuffer();

  renderChildren();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

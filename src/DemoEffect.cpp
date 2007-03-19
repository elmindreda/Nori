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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLPass.h>

#include <wendy/RenderFont.h>
#include <wendy/RenderStyle.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>

#include <wendy/DemoParameter.h>
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

Effect::Effect(EffectType& initType, const String& name):
  Managed<Effect>(name),
  type(initType),
  start(0.0),
  duration(0.0),
  elapsed(0.0),
  active(false)
{
}

bool Effect::init(void)
{
  return true;
}

Effect* Effect::createChild(const String& typeName,
                            const String& instanceName)
{
  EffectType* type = EffectType::findInstance(typeName);
  if (!type)
  {
    Log::writeError("Cannot find effect type %s", typeName.c_str());
    return NULL;
  }

  Effect* instance = type->createEffect(instanceName);
  if (!instance)
    return NULL;

  addChild(*instance);
  return instance;
}

Parameter* Effect::findParameter(const String& name)
{
  for (ParameterList::const_iterator i = parameters.begin();  i != parameters.end();  i++)
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

const Effect::ParameterList& Effect::getParameters(void)
{
  return parameters;
}

void Effect::addParameter(Parameter& parameter)
{
  if (std::find(parameters.begin(), parameters.end(), &parameter) == parameters.end())
    parameters.push_back(&parameter);
}

void Effect::removeParameter(Parameter& parameter)
{
  ParameterList::iterator i = std::find(parameters.begin(), parameters.end(), &parameter);
  if (i != parameters.end())
    parameters.erase(i);
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

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

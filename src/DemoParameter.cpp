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
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLPass.h>

#include <wendy/RenderFont.h>
#include <wendy/RenderStyle.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>

#include <wendy/DemoParameter.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

ParameterKey::ParameterKey(Time initMoment):
  moment(initMoment)
{
}

ParameterKey::~ParameterKey(void)
{
}

Time ParameterKey::getMoment(void) const
{
  return moment;
}

///////////////////////////////////////////////////////////////////////

Parameter::Parameter(const String& initName):
  name(initName)
{
}

Parameter::~Parameter(void)
{
  while (!keys.empty())
  {
    delete keys.back();
    keys.pop_back();
  }
}

void Parameter::destroyKey(ParameterKey& key)
{
  KeyList::iterator i = std::find(keys.begin(), keys.end(), &key);
  if (i != keys.end())
  {
    delete *i;
    keys.erase(i);
  }
}

const String& Parameter::getName(void) const
{
  return name;
}

const Parameter::KeyList& Parameter::getKeys(void) const
{
  return keys;
}

void Parameter::registerKey(ParameterKey& key)
{
  KeyList::iterator i;

  for (i = keys.begin();  i != keys.end();  i++)
  {
    if ((*i)->moment >= key.moment)
      break;
  }

  keys.insert(i, &key);
}

///////////////////////////////////////////////////////////////////////

FloatKey::FloatKey(FloatParameter& initParameter, Time moment):
  ParameterKey(moment),
  parameter(initParameter),
  value(0.f)
{
}

float FloatKey::getValue(void) const
{
  return value;
}

void FloatKey::asString(String& result) const
{
  Variant::convertToString(result, value);
}

void FloatKey::setValue(float newValue)
{
  value = newValue;
  value = std::min(std::max(value, parameter.getMaxValue()),
                   parameter.getMinValue());
}

void FloatKey::setStringValue(const String& newValue)
{
  value = Variant::convertToFloat(newValue);
  value = std::min(std::max(value, parameter.getMaxValue()),
                   parameter.getMinValue());
}

///////////////////////////////////////////////////////////////////////

FloatParameter::FloatParameter(const String& name,
	                       float initMinValue,
	                       float initMaxValue):
  ParameterTemplate<FloatKey, float>(name),
  minValue(initMinValue),
  maxValue(initMaxValue)
{
}

template <>
ParameterKey& ParameterTemplate<FloatKey,float>::createKey(Time moment,
                                                           const String& value)
{
  FloatKey* key = new FloatKey(dynamic_cast<FloatParameter&>(*this), moment);
  registerKey(*key);

  key->setStringValue(value);
  return *key;
}

float FloatParameter::getMinValue(void) const
{
  return minValue;
}

float FloatParameter::getMaxValue(void) const
{
  return maxValue;
}

float FloatParameter::interpolateKeys(const FloatKey& start,
                                      const FloatKey& end,
			              float t) const
{
  return start.getValue() * (1.f - t) + end.getValue() * t;
}

float FloatParameter::getDefaultValue(void) const
{
  return 0.f;
}

///////////////////////////////////////////////////////////////////////

BooleanKey::BooleanKey(Time moment):
  ParameterKey(moment),
  value(false)
{
}

bool BooleanKey::getValue(void) const
{
  return value;
}

void BooleanKey::asString(String& result) const
{
  Variant::convertToString(result, value);
}

void BooleanKey::setValue(bool newValue)
{
  value = newValue;
}

void BooleanKey::setStringValue(const String& newValue)
{
  value = Variant::convertToBoolean(newValue);
}

///////////////////////////////////////////////////////////////////////

BooleanParameter::BooleanParameter(const String& name):
  ParameterTemplate<BooleanKey, bool>(name)
{
}

bool BooleanParameter::interpolateKeys(const BooleanKey& start,
                                       const BooleanKey& end,
			               float t) const
{
  return start.getValue();
}

bool BooleanParameter::getDefaultValue(void) const
{
  return false;
}

///////////////////////////////////////////////////////////////////////

StyleKey::StyleKey(Time moment):
  ParameterKey(moment)
{
}

render::Style* StyleKey::getStyle(void) const
{
  return style;
}

void StyleKey::setStyle(render::Style* newStyle)
{
  style = newStyle;
}

void StyleKey::asString(String& result) const
{
  if (style)
    result = style->getName();
  else
    result.clear();
}

void StyleKey::setStringValue(const String& newValue)
{
  style = render::Style::readInstance(newValue);
}

///////////////////////////////////////////////////////////////////////

StyleParameter::StyleParameter(const String& name):
  ParameterTemplate<StyleKey, render::Style*>(name)
{
}

render::Style* StyleParameter::getDefaultValue(void) const
{
  return NULL;
}

render::Style* StyleParameter::interpolateKeys(const StyleKey& start,
			                       const StyleKey& end,
			                       float t) const
{
  return start.getStyle();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

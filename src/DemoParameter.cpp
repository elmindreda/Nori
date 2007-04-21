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

ParameterKey::ParameterKey(Parameter& initParameter):
  parameter(initParameter)
{
}

ParameterKey::~ParameterKey(void)
{
}

Parameter& ParameterKey::getParameter(void) const
{
  return parameter;
}

Time ParameterKey::getMoment(void) const
{
  return moment;
}

void ParameterKey::setMoment(Time newMoment)
{
  moment = newMoment;
  // TODO: Reinsert into parameter.
}

///////////////////////////////////////////////////////////////////////

Parameter::Parameter(Effect& initEffect, const String& initName):
  effect(initEffect),
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

Effect& Parameter::getEffect(void) const
{
  return effect;
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
    if ((*i)->getMoment() >= key.getMoment())
      break;
  }

  keys.insert(i, &key);
}

///////////////////////////////////////////////////////////////////////

FloatKey::FloatKey(Parameter& parameter):
  ParameterKey(parameter),
  value(0.f)
{
}

float FloatKey::getValue(void) const
{
  return value;
}

void FloatKey::setValue(float newValue)
{
  FloatParameter& parameter = dynamic_cast<FloatParameter&>(getParameter());

  value = newValue;
  value = std::min(std::max(value, parameter.getMaxValue()),
                   parameter.getMinValue());
}

String FloatKey::asString(void) const
{
  String result;
  Variant::convertToString(result, value);
  return result;
}

void FloatKey::setStringValue(const String& newValue)
{
  FloatParameter& parameter = dynamic_cast<FloatParameter&>(getParameter());

  value = Variant::convertToFloat(newValue);
  value = std::min(std::max(value, parameter.getMaxValue()),
                   parameter.getMinValue());
}

///////////////////////////////////////////////////////////////////////

FloatParameter::FloatParameter(Effect& effect,
                               const String& name,
	                       float initMinValue,
	                       float initMaxValue):
  ParameterTemplate<FloatKey, float>(effect, name),
  minValue(initMinValue),
  maxValue(initMaxValue)
{
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

BooleanKey::BooleanKey(Parameter& parameter):
  ParameterKey(parameter)
{
}

bool BooleanKey::getValue(void) const
{
  return value;
}

String BooleanKey::asString(void) const
{
  String result;
  Variant::convertToString(result, value);
  return result;
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

BooleanParameter::BooleanParameter(Effect& effect, const String& name):
  ParameterTemplate<BooleanKey, bool>(effect, name)
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

StyleKey::StyleKey(Parameter& parameter):
  ParameterKey(parameter)
{
}

render::Style* StyleKey::getValue(void) const
{
  return style;
}

void StyleKey::setValue(render::Style* newStyle)
{
  style = newStyle;
}

String StyleKey::asString(void) const
{
  if (style)
    return style->getName();

  return String();
}

void StyleKey::setStringValue(const String& newValue)
{
  style = render::Style::readInstance(newValue);
}

///////////////////////////////////////////////////////////////////////

StyleParameter::StyleParameter(Effect& effect, const String& name):
  ParameterTemplate<StyleKey, render::Style*>(effect, name)
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
  return start.getValue();
}

///////////////////////////////////////////////////////////////////////

ColorKeyRGB::ColorKeyRGB(Parameter& parameter):
  ParameterKey(parameter)
{
}

ColorRGB ColorKeyRGB::getValue(void) const
{
  return value;
}

void ColorKeyRGB::setValue(ColorRGB newValue)
{
  value = newValue;
}

String ColorKeyRGB::asString(void) const
{
  String result;
  // TODO: Implement.
  return result;
}

void ColorKeyRGB::setStringValue(const String& newValue)
{
  // TODO: Implement.
}

///////////////////////////////////////////////////////////////////////

ColorParameterRGB::ColorParameterRGB(Effect& effect, const String& name):
  ParameterTemplate<ColorKeyRGB, ColorRGB>(effect, name)
{
}

ColorRGB ColorParameterRGB::getDefaultValue(void) const
{
  return ColorRGB::BLACK;
}

ColorRGB ColorParameterRGB::interpolateKeys(const ColorKeyRGB& start,
			                    const ColorKeyRGB& end,
			                    float t) const
{
  return start.getValue();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

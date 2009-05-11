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
#include <wendy/UIWidget.h>
#include <wendy/UISlider.h>

#include <wendy/DemoProperty.h>
#include <wendy/DemoEffect.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

PropertyKey::PropertyKey(Property& initProperty):
  property(initProperty),
  moment(0.0)
{
  insert();
}

PropertyKey::~PropertyKey(void)
{
  remove();
}

Property& PropertyKey::getProperty(void) const
{
  return property;
}

Time PropertyKey::getMoment(void) const
{
  return moment;
}

void PropertyKey::setMoment(Time newMoment)
{
  moment = std::max(newMoment, 0.0);
  remove();
  insert();
}

void PropertyKey::insert(void)
{
  Property::KeyList::iterator i;

  for (i = property.keys.begin();  i != property.keys.end();  i++)
  {
    if ((*i)->getMoment() >= moment)
      break;
  }

  property.keys.insert(i, this);
}

void PropertyKey::remove(void)
{
  Property::KeyList::iterator i = std::find(property.keys.begin(), property.keys.end(), this);
  if (i != property.keys.end())
    property.keys.erase(i);
}

///////////////////////////////////////////////////////////////////////

Property::Property(Effect& initEffect, const String& initName):
  effect(initEffect),
  name(initName),
  mode(LINEAR)
{
  if (std::find(effect.properties.begin(), effect.properties.end(), this) ==
      effect.properties.end())
    effect.properties.push_back(this);
}

Property::~Property(void)
{
  Effect::PropertyList::iterator i = std::find(effect.properties.begin(), effect.properties.end(), this);
  if (i != effect.properties.end())
    effect.properties.erase(i);

  while (!keys.empty())
    delete keys.back();
}

Time Property::getSequenceStart(void) const
{
  return getSequenceStart(effect.getTimeElapsed());
}

Time Property::getSequenceStart(Time moment) const
{
  unsigned int index = 0;

  for (index = 0;  index < keys.size();  index++)
  {
    if (keys[index]->getMoment() > moment)
      break;
  }

  if (index == 0)
    return 0.0;

  if (index == keys.size())
    return keys.back()->getMoment();

  return keys[index - 1]->getMoment();
}

Time Property::getSequenceDuration(void) const
{
  return getSequenceDuration(effect.getTimeElapsed());
}

Time Property::getSequenceDuration(Time moment) const
{
  if (keys.empty())
    return effect.getDuration();

  unsigned int index = 0;

  for (index = 0;  index < keys.size();  index++)
  {
    if (keys[index]->getMoment() > moment)
      break;
  }

  if (index == 0)
    index++;

  if (index == keys.size())
    return effect.getDuration() - keys.back()->getMoment();

  return keys[index]->getMoment() - keys[index - 1]->getMoment();
}

unsigned int Property::getSequenceIndex(void) const
{
  return getSequenceIndex(effect.getTimeElapsed());
}

unsigned int Property::getSequenceIndex(Time moment) const
{
  if (keys.empty())
    return 0;

  unsigned int index = 0;

  for (index = 0;  index < keys.size();  index++)
  {
    if (keys[index]->getMoment() > moment)
      break;
  }

  if (index == keys.size())
    index--;

  return index;
}

Property::BlendMode Property::getBlendMode(void) const
{
  return mode;
}

void Property::setBlendMode(BlendMode newBlendMode)
{
  mode = newBlendMode;
}

Effect& Property::getEffect(void) const
{
  return effect;
}

const String& Property::getName(void) const
{
  return name;
}

const Property::KeyList& Property::getKeys(void) const
{
  return keys;
}

///////////////////////////////////////////////////////////////////////

FloatKey::FloatKey(Property& property):
  PropertyKey(property),
  value(0.f)
{
}

UI::Widget* FloatKey::createManipulator(void)
{
  FloatProperty& property = dynamic_cast<FloatProperty&>(getProperty());

  UI::Slider* slider = new UI::Slider(UI::HORIZONTAL);
  slider->setValueRange(property.getMinValue(), property.getMaxValue());
  slider->getValueChangedSignal().connect(*this, &FloatKey::onValueChanged);

  return slider;
}

float FloatKey::getValue(void) const
{
  return value;
}

void FloatKey::setValue(float newValue)
{
  FloatProperty& property = dynamic_cast<FloatProperty&>(getProperty());

  value = newValue;
  value = std::min(std::max(value, property.getMaxValue()),
                   property.getMinValue());
}

String FloatKey::asString(void) const
{
  String result;
  Variant::convertToString(result, value);
  return result;
}

void FloatKey::setStringValue(const String& newValue)
{
  FloatProperty& property = dynamic_cast<FloatProperty&>(getProperty());

  value = Variant::convertToFloat(newValue);
  value = std::max(std::min(value, property.getMaxValue()),
                   property.getMinValue());
}

void FloatKey::onValueChanged(UI::Slider& slider)
{
  value = slider.getValue();
}

///////////////////////////////////////////////////////////////////////

FloatProperty::FloatProperty(Effect& effect,
                               const String& name,
	                       float initMinValue,
	                       float initMaxValue):
  PropertyTemplate<FloatKey, float>(effect, name),
  minValue(initMinValue),
  maxValue(initMaxValue)
{
}

float FloatProperty::getMinValue(void) const
{
  return minValue;
}

float FloatProperty::getMaxValue(void) const
{
  return maxValue;
}

float FloatProperty::interpolateKeys(const FloatKey& start,
                                     const FloatKey& end,
			             float t) const
{
  return start.getValue() * (1.f - t) + end.getValue() * t;
}

float FloatProperty::getDefaultValue(void) const
{
  return 0.f;
}

///////////////////////////////////////////////////////////////////////

BooleanKey::BooleanKey(Property& property):
  PropertyKey(property),
  value(false)
{
}

UI::Widget* BooleanKey::createManipulator(void)
{
  return NULL;
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

BooleanProperty::BooleanProperty(Effect& effect, const String& name):
  PropertyTemplate<BooleanKey, bool>(effect, name)
{
}

bool BooleanProperty::interpolateKeys(const BooleanKey& start,
                                      const BooleanKey& end,
			              float t) const
{
  return start.getValue();
}

bool BooleanProperty::getDefaultValue(void) const
{
  return false;
}

///////////////////////////////////////////////////////////////////////

EnumKey::EnumKey(Property& property):
  PropertyKey(property),
  value(0)
{
}

UI::Widget* EnumKey::createManipulator(void)
{
  return NULL;
}

unsigned int EnumKey::getValue(void) const
{
  return value;
}

String EnumKey::asString(void) const
{
  EnumProperty& property = dynamic_cast<EnumProperty&>(getProperty());

  return property.getSymbolName(value);
}

void EnumKey::setValue(unsigned int newValue)
{
  value = newValue;
}

void EnumKey::setStringValue(const String& newValue)
{
  EnumProperty& property = dynamic_cast<EnumProperty&>(getProperty());

  value = property.getSymbolID(newValue);
}

///////////////////////////////////////////////////////////////////////

EnumProperty::EnumProperty(Effect& effect, const String& name):
  PropertyTemplate<EnumKey, unsigned int>(effect, name)
{
  symbols.setDefaults("<unregistered>", 0);
}

void EnumProperty::addSymbol(const String& name, unsigned int ID)
{
  symbols[name] = ID;
}

const String& EnumProperty::getSymbolName(unsigned int ID) const
{
  return symbols[ID];
}

unsigned int EnumProperty::getSymbolID(const String& name) const
{
  return symbols[name];
}

unsigned int EnumProperty::interpolateKeys(const EnumKey& start,
                                           const EnumKey& end,
			                   float t) const
{
  return start.getValue();
}

unsigned int EnumProperty::getDefaultValue(void) const
{
  return 0;
}

///////////////////////////////////////////////////////////////////////

TextureKey::TextureKey(Property& property):
  PropertyKey(property)
{
}

UI::Widget* TextureKey::createManipulator(void)
{
  return NULL;
}

GL::Texture* TextureKey::getValue(void) const
{
  return texture;
}

void TextureKey::setValue(GL::Texture* newTexture)
{
  texture = newTexture;
}

String TextureKey::asString(void) const
{
  if (texture)
    return texture->getName();

  return String();
}

void TextureKey::setStringValue(const String& newValue)
{
  texture = GL::Texture::readInstance(newValue);
}

///////////////////////////////////////////////////////////////////////

TextureProperty::TextureProperty(Effect& effect, const String& name):
  PropertyTemplate<TextureKey, GL::Texture*>(effect, name)
{
}

bool TextureProperty::isComplete(void) const
{
  const KeyList& keys = getKeys();

  for (KeyList::const_iterator i = keys.begin();  i != keys.end();  i++)
  {
    if (!dynamic_cast<TextureKey*>(*i)->getValue())
      return false;
  }

  return true;
}  

GL::Texture* TextureProperty::getDefaultValue(void) const
{
  return NULL;
}

GL::Texture* TextureProperty::interpolateKeys(const TextureKey& start,
			                      const TextureKey& end,
			                      float t) const
{
  return start.getValue();
}

///////////////////////////////////////////////////////////////////////

MaterialKey::MaterialKey(Property& property):
  PropertyKey(property)
{
}

UI::Widget* MaterialKey::createManipulator(void)
{
  return NULL;
}

render::Material* MaterialKey::getValue(void) const
{
  return material;
}

void MaterialKey::setValue(render::Material* newMaterial)
{
  material = newMaterial;
}

String MaterialKey::asString(void) const
{
  if (material)
    return material->getName();

  return String();
}

void MaterialKey::setStringValue(const String& newValue)
{
  material = render::Material::readInstance(newValue);
}

///////////////////////////////////////////////////////////////////////

MaterialProperty::MaterialProperty(Effect& effect, const String& name):
  PropertyTemplate<MaterialKey, render::Material*>(effect, name)
{
}

bool MaterialProperty::isComplete(void) const
{
  const KeyList& keys = getKeys();

  for (KeyList::const_iterator i = keys.begin();  i != keys.end();  i++)
  {
    if (!dynamic_cast<MaterialKey*>(*i)->getValue())
      return false;
  }

  return true;
}  

render::Material* MaterialProperty::getDefaultValue(void) const
{
  return NULL;
}

render::Material* MaterialProperty::interpolateKeys(const MaterialKey& start,
			                      const MaterialKey& end,
			                      float t) const
{
  return start.getValue();
}

///////////////////////////////////////////////////////////////////////

ColorKeyRGB::ColorKeyRGB(Property& property):
  PropertyKey(property),
  value(ColorRGB::BLACK)
{
}

UI::Widget* ColorKeyRGB::createManipulator(void)
{
  return NULL;
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
  value.convertToString(result);
  return result;
}

void ColorKeyRGB::setStringValue(const String& newValue)
{
  value = newValue;
}

///////////////////////////////////////////////////////////////////////

ColorPropertyRGB::ColorPropertyRGB(Effect& effect, const String& name):
  PropertyTemplate<ColorKeyRGB, ColorRGB>(effect, name)
{
}

ColorRGB ColorPropertyRGB::getDefaultValue(void) const
{
  return ColorRGB::BLACK;
}

ColorRGB ColorPropertyRGB::interpolateKeys(const ColorKeyRGB& start,
			                   const ColorKeyRGB& end,
			                   float t) const
{
  return start.getValue() * (1.f - t) + end.getValue() * t;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

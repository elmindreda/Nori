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
#ifndef WENDY_DEMOPROPERTY_H
#define WENDY_DEMOPROPERTY_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Property;
class Effect;

///////////////////////////////////////////////////////////////////////

/*! @brief Demo effect property key superclass.
 *  @ingroup demo
 */
class PropertyKey : public Trackable
{
public:
  PropertyKey(Property& property);
  virtual ~PropertyKey(void);
  virtual UI::Widget* createManipulator(void) = 0;
  virtual String asString(void) const = 0;
  virtual void setStringValue(const String& newValue) = 0;
  Property& getProperty(void) const;
  Time getMoment(void) const;
  void setMoment(Time newMoment);
private:
  void insert(void);
  void remove(void);
  Property& property;
  Time moment;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Demo effect property superclass.
 *  @ingroup demo
 */
class Property
{
  friend class PropertyKey;
public:
  enum BlendMode
  {
    SELECT_START,
    SELECT_END,
    LINEAR,
  };
  typedef std::vector<PropertyKey*> KeyList;
  Property(Effect& effect, const String& name);
  virtual ~Property(void);
  virtual PropertyKey& createKey(Time moment, const String& value = "") = 0;
  Time getSequenceStart(void) const;
  Time getSequenceStart(Time moment) const;
  Time getSequenceDuration(void) const;
  Time getSequenceDuration(Time moment) const;
  unsigned int getSequenceIndex(void) const;
  unsigned int getSequenceIndex(Time moment) const;
  BlendMode getBlendMode(void) const;
  void setBlendMode(BlendMode newBlendMode);
  Effect& getEffect(void) const;
  const String& getName(void) const;
  const KeyList& getKeys(void) const;
private:
  Effect& effect;
  String name;
  KeyList keys;
  BlendMode mode;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
template <typename K, typename T>
class PropertyTemplate : public Property
{
public:
  inline PropertyTemplate(Effect& effect, const String& name);
  inline PropertyKey& createKey(Time moment, const String& value = "");
  inline T getValue(void) const;
  inline T getValue(Time moment) const;
protected:
  virtual T getDefaultValue(void) const = 0;
  virtual T interpolateKeys(const K& start, const K& end, float t) const = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class FloatKey : public PropertyKey
{
public:
  FloatKey(Property& property);
  UI::Widget* createManipulator(void);
  float getValue(void) const;
  void setValue(float newValue);
  String asString(void) const;
  void setStringValue(const String& newValue);
private:
  void onValueChanged(UI::Slider& slider);
  float value;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class FloatProperty : public PropertyTemplate<FloatKey, float>
{
public:
  FloatProperty(Effect& effect,
                 const String& name,
                 float minValue,
		 float maxValue);
  float getMinValue(void) const;
  float getMaxValue(void) const;
private:
  float getDefaultValue(void) const;
  float interpolateKeys(const FloatKey& start,
                        const FloatKey& end,
			float t) const;
  float minValue;
  float maxValue;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class BooleanKey : public PropertyKey
{
public:
  BooleanKey(Property& property);
  UI::Widget* createManipulator(void);
  bool getValue(void) const;
  void setValue(bool newValue);
  String asString(void) const;
  void setStringValue(const String& newValue);
private:
  bool value;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class BooleanProperty : public PropertyTemplate<BooleanKey, bool>
{
public:
  BooleanProperty(Effect& effect, const String& name);
private:
  bool getDefaultValue(void) const;
  bool interpolateKeys(const BooleanKey& start,
                       const BooleanKey& end,
		       float t) const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class EnumKey : public PropertyKey
{
public:
  EnumKey(Property& property);
  UI::Widget* createManipulator(void);
  unsigned int getValue(void) const;
  void setValue(unsigned int newValue);
  String asString(void) const;
  void setStringValue(const String& newValue);
private:
  unsigned int value;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class EnumProperty : public PropertyTemplate<EnumKey, unsigned int>
{
public:
  EnumProperty(Effect& effect, const String& name);
  void addSymbol(const String& name, unsigned int ID);
  const String& getSymbolName(unsigned int ID) const;
  unsigned int getSymbolID(const String& name) const;
private:
  unsigned int getDefaultValue(void) const;
  unsigned int interpolateKeys(const EnumKey& start,
                               const EnumKey& end,
		               float t) const;
  Mapper<String, unsigned int> symbols;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class TextureKey : public PropertyKey
{
public:
  TextureKey(Property& property);
  UI::Widget* createManipulator(void);
  GL::Texture* getValue(void) const;
  void setValue(GL::Texture* newTexture);
  String asString(void) const;
  void setStringValue(const String& newValue);
private:
  Ref<GL::Texture> texture;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class TextureProperty : public PropertyTemplate<TextureKey, GL::Texture*>
{
public:
  TextureProperty(Effect& effect, const String& name);
  bool isComplete(void) const;
private:
  GL::Texture* getDefaultValue(void) const;
  GL::Texture* interpolateKeys(const TextureKey& start,
                               const TextureKey& end,
			       float t) const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class StyleKey : public PropertyKey
{
public:
  StyleKey(Property& property);
  UI::Widget* createManipulator(void);
  render::Style* getValue(void) const;
  void setValue(render::Style* newStyle);
  String asString(void) const;
  void setStringValue(const String& newValue);
private:
  Ref<render::Style> style;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class StyleProperty : public PropertyTemplate<StyleKey, render::Style*>
{
public:
  StyleProperty(Effect& effect, const String& name);
  bool isComplete(void) const;
private:
  render::Style* getDefaultValue(void) const;
  render::Style* interpolateKeys(const StyleKey& start,
                                 const StyleKey& end,
			         float t) const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class ColorKeyRGB : public PropertyKey
{
public:
  ColorKeyRGB(Property& property);
  UI::Widget* createManipulator(void);
  ColorRGB getValue(void) const;
  void setValue(ColorRGB newValue);
  String asString(void) const;
  void setStringValue(const String& newValue);
private:
  ColorRGB value;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class ColorPropertyRGB : public PropertyTemplate<ColorKeyRGB, ColorRGB>
{
public:
  ColorPropertyRGB(Effect& effect, const String& name);
private:
  ColorRGB getDefaultValue(void) const;
  ColorRGB interpolateKeys(const ColorKeyRGB& start,
                           const ColorKeyRGB& end,
			   float t) const;
};

///////////////////////////////////////////////////////////////////////

template <typename K, typename T>
inline PropertyTemplate<K,T>::PropertyTemplate(Effect& effect, const String& name):
  Property(effect, name)
{
}

template <typename K, typename T>
inline PropertyKey& PropertyTemplate<K,T>::createKey(Time moment, const String& value)
{
  K* key = new K(*this);
  key->setStringValue(value);
  key->setMoment(moment);

  return *key;
}

template <typename K, typename T>
inline T PropertyTemplate<K,T>::getValue(void) const
{
  return getValue(getEffect().getTimeElapsed());
}

template <typename K, typename T>
inline T PropertyTemplate<K,T>::getValue(Time moment) const
{
  const KeyList& keys = getKeys();

  if (keys.empty())
    return getDefaultValue();

  unsigned int index = 0;

  for (index = 0;  index < keys.size();  index++)
  {
    if (keys[index]->getMoment() > moment)
      break;
  }

  if (index == 0)
    return dynamic_cast<K*>(keys.front())->getValue();

  if (index == keys.size())
    return dynamic_cast<K*>(keys.back())->getValue();

  const K& startKey = *dynamic_cast<K*>(keys[index - 1]);
  const K& endKey = *dynamic_cast<K*>(keys[index]);

  switch (getBlendMode())
  {
    case SELECT_START:
      return startKey.getValue();

    case SELECT_END:
      return endKey.getValue();

    case LINEAR:
    default:
    {
      const Time start = startKey.getMoment();

      const float t = (moment - start) / (endKey.getMoment() - start);

      return interpolateKeys(startKey, endKey, t);
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOPROPERTY_H*/
///////////////////////////////////////////////////////////////////////

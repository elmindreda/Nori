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
#ifndef WENDY_DEMOPARAMETER_H
#define WENDY_DEMOPARAMETER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Parameter;
class Effect;

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 *  @brief Demo effect parameter key superclass.
 */
class ParameterKey
{
public:
  ParameterKey(Parameter& parameter);
  virtual ~ParameterKey(void);
  virtual String asString(void) const = 0;
  virtual void setStringValue(const String& newValue) = 0;
  Parameter& getParameter(void) const;
  Time getMoment(void) const;
  void setMoment(Time newMoment);
private:
  Parameter& parameter;
  Time moment;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 *  @brief Demo effect parameter superclass.
 */
class Parameter
{
  friend class ParameterKey;
public:
  typedef std::vector<ParameterKey*> KeyList;
  Parameter(Effect& effect, const String& name);
  virtual ~Parameter(void);
  virtual ParameterKey& createKey(Time moment, const String& value = "") = 0;
  void destroyKey(ParameterKey& key);
  //Time getSequenceStart(Time moment) const;
  //Time getSequenceDuration(Time moment) const;
  //unsigned int getSequenceIndex(Time moment) const;
  Effect& getEffect(void) const;
  const String& getName(void) const;
  const KeyList& getKeys(void) const;
protected:
  void registerKey(ParameterKey& key);
private:
  Effect& effect;
  String name;
  KeyList keys;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
template <typename K, typename T>
class ParameterTemplate : public Parameter
{
public:
  inline ParameterTemplate(Effect& effect, const String& name);
  inline ParameterKey& createKey(Time moment, const String& value = "");
  inline T getValue(Time moment) const;
protected:
  virtual T getDefaultValue(void) const = 0;
  virtual T interpolateKeys(const K& start, const K& end, float t) const = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class FloatKey : public ParameterKey
{
public:
  FloatKey(Parameter& parameter);
  float getValue(void) const;
  void setValue(float newValue);
  String asString(void) const;
  void setStringValue(const String& newValue);
private:
  float value;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class FloatParameter : public ParameterTemplate<FloatKey, float>
{
public:
  FloatParameter(Effect& effect,
                 const String& name,
                 float minValue = 0.f,
		 float maxValue = 1.f);
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
class BooleanKey : public ParameterKey
{
public:
  BooleanKey(Parameter& parameter);
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
class BooleanParameter : public ParameterTemplate<BooleanKey, bool>
{
public:
  BooleanParameter(Effect& effect, const String& name);
  bool getValue(Time moment) const;
private:
  bool getDefaultValue(void) const;
  bool interpolateKeys(const BooleanKey& start,
                       const BooleanKey& end,
		       float t) const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class StyleKey : public ParameterKey
{
public:
  StyleKey(Parameter& parameter);
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
class StyleParameter : public ParameterTemplate<StyleKey, render::Style*>
{
public:
  StyleParameter(Effect& effect, const String& name);
private:
  render::Style* getDefaultValue(void) const;
  render::Style* interpolateKeys(const StyleKey& start,
                                 const StyleKey& end,
			         float t) const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class ColorKeyRGB : public ParameterKey
{
public:
  ColorKeyRGB(Parameter& parameter);
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
class ColorParameterRGB : public ParameterTemplate<ColorKeyRGB, ColorRGB>
{
public:
  ColorParameterRGB(Effect& effect, const String& name);
private:
  ColorRGB getDefaultValue(void) const;
  ColorRGB interpolateKeys(const ColorKeyRGB& start,
                           const ColorKeyRGB& end,
			   float t) const;
};

///////////////////////////////////////////////////////////////////////

template <typename K, typename T>
inline ParameterTemplate<K,T>::ParameterTemplate(Effect& effect, const String& name):
  Parameter(effect, name)
{
}

template <typename K, typename T>
ParameterKey& ParameterTemplate<K,T>::createKey(Time moment, const String& value)
{
  K* key = new K(*this);
  key->setStringValue(value);
  key->setMoment(moment);
  registerKey(*key);

  return *key;
}

template <typename K, typename T>
inline T ParameterTemplate<K,T>::getValue(Time moment) const
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

  const Time start = startKey.getMoment();

  const float t = (moment - start) / (endKey.getMoment() - start);

  return interpolateKeys(startKey, endKey, t);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOPARAMETER_H*/
///////////////////////////////////////////////////////////////////////

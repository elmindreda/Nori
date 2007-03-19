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

class ParameterKey
{
  friend class Parameter;
public:
  ParameterKey(Time moment);
  virtual ~ParameterKey(void);
  virtual void asString(String& result) const = 0;
  virtual void setStringValue(const String& newValue) = 0;
  Time getMoment(void) const;
private:
  Time moment;
};

///////////////////////////////////////////////////////////////////////

class Parameter
{
public:
  typedef std::vector<ParameterKey*> KeyList;
  Parameter(const String& name);
  virtual ~Parameter(void);
  virtual ParameterKey& createKey(Time moment, const String& value = "") = 0;
  void destroyKey(ParameterKey& key);
  const String& getName(void) const;
  const KeyList& getKeys(void) const;
protected:
  void registerKey(ParameterKey& key);
private:
  String name;
  KeyList keys;
};

///////////////////////////////////////////////////////////////////////

template <typename K, typename T>
class ParameterTemplate : public Parameter
{
public:
  inline ParameterTemplate(const String& name);
  inline ParameterKey& createKey(Time moment, const String& value = "");
  inline T getValue(Time moment) const;
protected:
  virtual T getDefaultValue(void) const = 0;
  virtual T interpolateKeys(const K& start, const K& end, float t) const = 0;
};

///////////////////////////////////////////////////////////////////////

class FloatParameter;

///////////////////////////////////////////////////////////////////////

class FloatKey : public ParameterKey
{
public:
  FloatKey(FloatParameter& parameter, Time moment);
  float getValue(void) const;
  void setValue(float newValue);
  void asString(String& result) const;
  void setStringValue(const String& newValue);
private:
  FloatParameter& parameter;
  float value;
};

///////////////////////////////////////////////////////////////////////

class FloatParameter : public ParameterTemplate<FloatKey, float>
{
public:
  FloatParameter(const String& name,
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

class BooleanParameter;

///////////////////////////////////////////////////////////////////////

class BooleanKey : public ParameterKey
{
public:
  BooleanKey(Time moment);
  bool getValue(void) const;
  void setValue(bool newValue);
  void asString(String& result) const;
  void setStringValue(const String& newValue);
private:
  bool value;
};

///////////////////////////////////////////////////////////////////////

class BooleanParameter : public ParameterTemplate<BooleanKey, bool>
{
public:
  BooleanParameter(const String& name);
  bool getValue(Time moment) const;
private:
  bool getDefaultValue(void) const;
  bool interpolateKeys(const BooleanKey& start,
                       const BooleanKey& end,
		       float t) const;
};

///////////////////////////////////////////////////////////////////////

class StyleParameter;

///////////////////////////////////////////////////////////////////////

class StyleKey : public ParameterKey
{
public:
  StyleKey(Time moment);
  render::Style* getStyle(void) const;
  void setStyle(render::Style* newStyle);
  void asString(String& result) const;
  void setStringValue(const String& newValue);
private:
  Ref<render::Style> style;
};

///////////////////////////////////////////////////////////////////////

class StyleParameter : public ParameterTemplate<StyleKey, render::Style*>
{
public:
  StyleParameter(const String& name);
private:
  render::Style* getDefaultValue(void) const;
  render::Style* interpolateKeys(const StyleKey& start,
                                 const StyleKey& end,
			         float t) const;
};

///////////////////////////////////////////////////////////////////////

template <typename K, typename T>
inline ParameterTemplate<K,T>::ParameterTemplate(const String& name):
  Parameter(name)
{
}

template <typename K, typename T>
ParameterKey& ParameterTemplate<K,T>::createKey(Time moment, const String& value)
{
  K* key = new K(moment);
  registerKey(*key);

  key->setStringValue(value);
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

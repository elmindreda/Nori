///////////////////////////////////////////////////////////////////////
// Wendy user interface library
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
#ifndef WENDY_UISCROLLER_HPP
#define WENDY_UISCROLLER_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Scroller : public Widget
{
public:
  Scroller(Layer& layer, Orientation orientation);
  Orientation orientation() const;
  float minValue() const { return m_minValue; }
  float maxValue() const { return m_maxValue; }
  void setValueRange(float newMinValue, float newMaxValue);
  float value() const { return m_value; }
  void setValue(float newValue);
  float percentage() const { return m_percentage; }
  void setPercentage(float newPercentage);
  SignalProxy<void, Scroller&> valueChangedSignal();
protected:
  void draw() const;
private:
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onKey(Key key, Action action, uint mods) override;
  void onScroll(vec2 offset) override;
  void onDragBegun(vec2 point) override;
  void onDragMoved(vec2 point) override;
  void setValue(float newValue, bool notify);
  float handleSize() const;
  float handleOffset() const;
  float valueStep() const;
  Signal<void, Scroller&> m_valueChangedSignal;
  float m_minValue;
  float m_maxValue;
  float m_value;
  float m_percentage;
  float m_reference;
  Orientation m_orientation;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UISCROLLER_HPP*/
///////////////////////////////////////////////////////////////////////

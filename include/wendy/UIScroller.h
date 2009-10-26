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
#ifndef WENDY_UISCROLLER_H
#define WENDY_UISCROLLER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Scroller : public Widget
{
public:
  Scroller(Orientation orientation = HORIZONTAL);
  Orientation getOrientation(void) const;
  float getMinValue(void) const;
  float getMaxValue(void) const;
  void setValueRange(float newMinValue, float newMaxValue);
  float getValue(void) const;
  void setValue(float newValue);
  float getPercentage(void) const;
  void setPercentage(float newPercentage);
  SignalProxy1<void, Scroller&> getValueChangedSignal(void);
protected:
  void draw(void) const;
private:
  void onButtonClicked(Widget& widget,
		       const Vec2& position,
		       input::Button button,
		       bool clicked);
  void onKeyPressed(Widget& widget, input::Key key, bool pressed);
  void onWheelTurned(Widget& widget, int offset);
  void onDragBegun(Widget& widget, const Vec2& position);
  void onDragMoved(Widget& widget, const Vec2& position);
  void setValue(float newValue, bool notify);
  float getHandleSize(void) const;
  float getHandleOffset(void) const;
  float getValueStep(void) const;
  Signal1<void, Scroller&> valueChangedSignal;
  float minValue;
  float maxValue;
  float value;
  float percentage;
  float reference;
  Orientation orientation;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UISCROLLER_H*/
///////////////////////////////////////////////////////////////////////

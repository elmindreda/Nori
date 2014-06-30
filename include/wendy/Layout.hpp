///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_LAYOUT_HPP
#define WENDY_LAYOUT_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

enum LayoutMode
{
  COVER_PARENT,
  STATIC_SIZE
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Layout : public Widget
{
public:
  Layout(Layer& layer, Widget* parent, Orientation orientation, LayoutMode mode);
  LayoutMode mode() const { return m_mode; }
  Orientation orientation() const { return m_orientation; }
  float borderSize() const { return m_borderSize; }
  void setBorderSize(float newSize);
protected:
  void onChildAdded(Widget& child) override;
  void onChildDesiredSizeChanged(Widget& child) override;
  void onChildRemoved(Widget& child) override;
  void onAreaChanged() override;
  void onAreaChanged(Widget& parent);
  void onSizeChanged(Layer& layer);
private:
  void update();
  float m_borderSize;
  Orientation m_orientation;
  LayoutMode m_mode;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_LAYOUT_HPP*/
///////////////////////////////////////////////////////////////////////

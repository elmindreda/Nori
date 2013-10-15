///////////////////////////////////////////////////////////////////////
// Wendy user interface library
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
#ifndef WENDY_UILAYOUT_HPP
#define WENDY_UILAYOUT_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Layout : public Widget
{
public:
  Layout(Layer& layer, Orientation orientation, bool expanding = true);
  void addChild(Widget& child);
  void addChild(Widget& child, float size);
  bool isExpanding() const;
  Orientation orientation() const;
  float borderSize() const;
  void setBorderSize(float newSize);
  float childSize(Widget& child) const;
  void setChildSize(Widget& child, float newSize);
protected:
  void addedChild(Widget& child);
  void removedChild(Widget& child);
  void addedToParent(Widget& parent);
  void removedFromParent(Widget& parent);
  void onAreaChanged(Widget& parent);
private:
  typedef std::pair<Widget*, float> Size;
  void update();
  std::vector<Size> m_sizes;
  float m_borderSize;
  Orientation m_orientation;
  bool m_expanding;
  Ptr<SignalSlot> m_parentAreaSlot;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UILAYOUT_HPP*/
///////////////////////////////////////////////////////////////////////

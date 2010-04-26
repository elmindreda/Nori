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
#ifndef WENDY_UILAYOUT_H
#define WENDY_UILAYOUT_H
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
  Layout(Desktop& desktop, Widget* parent, Orientation orientation, bool expanding = true);
  bool isExpanding(void) const;
  Orientation getOrientation(void) const;
  float getBorderSize(void) const;
  void setBorderSize(float newSize);
  float getChildSize(Widget& child) const;
  void setChildSize(Widget& child, float newSize);
protected:
  void addedChild(Widget& child);
  void removedChild(Widget& child);
  void onAreaChanged(Widget& parent);
private:
  typedef std::map<Widget*, float> SizeMap;
  void update(void);
  SizeMap sizes;
  float borderSize;
  Orientation orientation;
  bool expanding;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UILAYOUT_H*/
///////////////////////////////////////////////////////////////////////

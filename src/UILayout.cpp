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

#include <wendy/Config.h>

#include <wendy/UIDrawer.h>
#include <wendy/UILayer.h>
#include <wendy/UIWidget.h>
#include <wendy/UILayout.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Layout::Layout(Layer& layer, Orientation initOrientation, bool initExpanding):
  Widget(layer),
  borderSize(0.f),
  orientation(initOrientation),
  expanding(initExpanding)
{
  if (!expanding)
    getAreaChangedSignal().connect(*this, &Layout::onAreaChanged);
}

void Layout::addChild(Widget& child)
{
  Widget::addChild(child);
}

void Layout::addChild(Widget& child, float size)
{
  setChildSize(child, size);
  Widget::addChild(child);
}

bool Layout::isExpanding() const
{
  return expanding;
}

Orientation Layout::getOrientation() const
{
  return orientation;
}

float Layout::getBorderSize() const
{
  return borderSize;
}

void Layout::setBorderSize(float newSize)
{
  borderSize = newSize;
  update();
}

float Layout::getChildSize(Widget& child) const
{
  for (auto s = sizes.begin();  s != sizes.end();  s++)
  {
    if (s->first == &child)
      return s->second;
  }

  panic("Unknown child widget found in layout");
}

void Layout::setChildSize(Widget& child, float newSize)
{
  for (auto s = sizes.begin();  s != sizes.end();  s++)
  {
    if (s->first == &child)
    {
      s->second = newSize;
      return;
    }
  }

  sizes.push_back(Size(&child, newSize));
}

void Layout::addedChild(Widget& child)
{
  auto s = sizes.begin();

  for ( ;  s != sizes.end();  s++)
  {
    if (s->first == &child)
      break;
  }

  if (s == sizes.end())
  {
    if (orientation == VERTICAL)
      sizes.push_back(Size(&child, child.getHeight()));
    else
      sizes.push_back(Size(&child, child.getWidth()));
  }

  update();
}

void Layout::removedChild(Widget& child)
{
  for (auto s = sizes.begin();  s != sizes.end();  s++)
  {
    if (s->first == &child)
    {
      sizes.erase(s);
      break;
    }
  }

  update();
}

void Layout::onAreaChanged(Widget& widget)
{
  if (expanding)
    setArea(Rect(vec2(0.f), widget.getSize()));

  update();
}

void Layout::addedToParent(Widget& parent)
{
  if (expanding)
  {
    parentAreaSlot = parent.getAreaChangedSignal().connect(*this, &Layout::onAreaChanged);
    onAreaChanged(parent);
  }
}

void Layout::removedFromParent(Widget& parent)
{
  if (expanding)
    parentAreaSlot = NULL;
}

void Layout::update()
{
  const WidgetList& children = getChildren();
  const vec2& size = getSize();
  unsigned int flexibleCount = 0;

  if (orientation == VERTICAL)
  {
    float stackHeight = borderSize;

    for (auto s = sizes.begin();  s != sizes.end();  s++)
    {
      const float height = s->second;
      if (height == 0.f)
        flexibleCount++;

      stackHeight += height + borderSize;
    }

    float width = size.x - borderSize * 2.f;

    float flexibleSize;
    if (flexibleCount)
      flexibleSize = (size.y - stackHeight) / flexibleCount;

    float position = size.y;

    for (auto c = children.begin();  c != children.end();  c++)
    {
      float height = getChildSize(**c);
      if (height == 0.f)
        height = flexibleSize;

      position -= height + borderSize;
      (*c)->setArea(Rect(borderSize, position, width, height));
    }
  }
  else
  {
    float stackWidth = borderSize;

    for (auto s = sizes.begin();  s != sizes.end();  s++)
    {
      const float width = s->second;
      if (width == 0.f)
        flexibleCount++;

      stackWidth += width + borderSize;
    }

    float height = size.y - borderSize * 2.f;

    float flexibleSize;
    if (flexibleCount)
      flexibleSize = (size.x - stackWidth) / flexibleCount;

    float position = size.x;

    for (auto c = children.begin();  c != children.end();  c++)
    {
      float width = getChildSize(**c);
      if (width == 0.f)
        width = flexibleSize;

      position -= width + borderSize;
      (*c)->setArea(Rect(position, borderSize, width, height));
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

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

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UILayout.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Layout::Layout(Desktop& desktop,
               Widget* parent,
               Orientation initOrientation,
               bool initExpanding):
  Widget(desktop, parent),
  orientation(initOrientation),
  expanding(initExpanding),
  borderSize(1.f)
{
  if (expanding)
  {
    if (parent)
      parent->getAreaChangedSignal().connect(*this, &Layout::onAreaChanged);
  }
  else
    getAreaChangedSignal().connect(*this, &Layout::onAreaChanged);
}

bool Layout::isExpanding(void) const
{
  return expanding;
}

Orientation Layout::getOrientation(void) const
{
  return orientation;
}

float Layout::getBorderSize(void) const
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
  SizeMap::const_iterator i = sizes.find(&child);
  if (i == sizes.end())
    return 0.f;

  return (*i).second;
}

void Layout::setChildSize(Widget& child, float newSize)
{
  SizeMap::iterator i = sizes.find(&child);
  if (i != sizes.end())
    sizes[&child] = newSize;
}

void Layout::addedChild(Widget& child)
{
  if (orientation == VERTICAL)
    sizes[&child] = child.getArea().size.y;
  else
    sizes[&child] = child.getArea().size.x;

  update();
}

void Layout::removedChild(Widget& child)
{
  sizes.erase(&child);
  update();
}

void Layout::onAreaChanged(Widget& widget)
{
  if (expanding)
    setArea(Rect(Vec2::ZERO, widget.getArea().size));

  update();
}

void Layout::update(void)
{
  const WidgetList& children = getChildren();
  const Vec2& size = getArea().size;
  unsigned int flexibleCount = 0;

  if (orientation == VERTICAL)
  {
    float stackHeight = borderSize;

    for (WidgetList::const_iterator i = children.begin();  i != children.end();  i++)
    {
      const float height = sizes[*i];
      if (height == 0.f)
	flexibleCount++;

      stackHeight += height + borderSize;
    }

    float width = size.x - borderSize * 2.f;

    float flexibleSize;
    if (flexibleCount)
      flexibleSize = (size.y - stackHeight) / flexibleCount;

    float position = size.y;

    for (WidgetList::const_iterator i = children.begin();  i != children.end();  i++)
    {
      float height = sizes[*i];
      if (height == 0.f)
	height = flexibleSize;

      position -= height + borderSize;
      (*i)->setArea(Rect(borderSize, position, width, height));
    }
  }
  else
  {
    float stackWidth = borderSize;

    for (WidgetList::const_iterator i = children.begin();  i != children.end();  i++)
    {
      const float width = sizes[*i];
      if (width == 0.f)
	flexibleCount++;

      stackWidth += width + borderSize;
    }

    float height = size.y - borderSize * 2.f;

    float flexibleSize;
    if (flexibleCount)
      flexibleSize = (size.x - stackWidth) / flexibleCount;

    float position = size.x;

    for (WidgetList::const_iterator i = children.begin();  i != children.end();  i++)
    {
      float width = sizes[*i];
      if (width == 0.f)
	width = flexibleSize;

      position -= width + borderSize;
      (*i)->setArea(Rect(position, borderSize, width, height));
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

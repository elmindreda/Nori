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
//
#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIBook.h>
#include <wendy/UIScroller.h>
#include <wendy/UILayout.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Layout::Layout(Orientation initOrientation, bool initExpanding):
  orientation(initOrientation),
  expanding(initExpanding),
  scroller(NULL),
  borderSize(1.f)
{
  if (!expanding)
    getAreaChangedSignal().connect(*this, &Layout::onAreaChanged);

  scroller = new Scroller(orientation);
  scroller->getValueChangedSignal().connect(*this, &Layout::onValueChanged);
  scroller->setVisible(false);
  addChild(*scroller);
}

void Layout::addChild(Widget& child)
{
  Widget::addChild(child);
}

void Layout::addChild(Widget& child, float size)
{
  Widget::addChild(child);
  sizes[&child] = size;
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
  if (&child == scroller)
    return;

  if (orientation == VERTICAL)
    sizes[&child] = child.getArea().size.y;
  else
    sizes[&child] = child.getArea().size.x;

  update();
}

void Layout::removedChild(Widget& child)
{
  if (&child == scroller)
    return;

  sizes.erase(&child);
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

void Layout::onAreaChanged(Widget& widget)
{
  if (expanding)
    setArea(Rectangle(Vector2::ZERO, widget.getArea().size));

  update();
}

void Layout::onValueChanged(Scroller& scroller)
{
}

void Layout::update(void)
{
  const List& children = getChildren();
  const Vector2& size = getArea().size;
  unsigned int flexibleCount = 0;

  if (orientation == VERTICAL)
  {
    float stackHeight = borderSize;

    for (List::const_iterator i = children.begin();  i != children.end();  i++)
    {
      if (*i == scroller)
	continue;

      const float height = sizes[*i];
      if (height == 0.f)
	flexibleCount++;

      stackHeight += height + borderSize;
    }

    float width = size.x - borderSize * 2.f;

    if (stackHeight > size.y)
    {
      width -= scroller->getArea().size.x;

      scroller->setArea(Rectangle(size.x - scroller->getArea().size.x,
				  0.f,
				  scroller->getArea().size.x,
				  size.y));
      scroller->setValueRange(0.f, size.y);
      scroller->setPercentage(size.y / stackHeight);
      scroller->setVisible(true);
    }
    else
      scroller->setVisible(false);

    float flexibleSize;
    if (flexibleCount)
      flexibleSize = (size.y - stackHeight) / flexibleCount;

    float position = size.y;

    for (List::const_iterator i = children.begin();  i != children.end();  i++)
    {
      if (*i == scroller)
	continue;

      float height = sizes[*i];
      if (height == 0.f)
	height = flexibleSize;

      position -= height + borderSize;
      (*i)->setArea(Rectangle(borderSize, position, width, height));
    }
  }
  else
  {
    float stackWidth = borderSize;

    for (List::const_iterator i = children.begin();  i != children.end();  i++)
    {
      if (*i == scroller)
	continue;

      const float width = sizes[*i];
      if (width == 0.f)
	flexibleCount++;

      stackWidth += width + borderSize;
    }

    float height = size.y - borderSize * 2.f;

    if (stackWidth > size.x)
    {
      height -= scroller->getArea().size.y;

      scroller->setArea(Rectangle(0.f, 0.f, size.x, scroller->getArea().size.y));
      scroller->setValueRange(0.f, size.x);
      scroller->setPercentage(size.x / stackWidth);
      scroller->setVisible(true);
    }
    else
      scroller->setVisible(false);

    float flexibleSize;
    if (flexibleCount)
      flexibleSize = (size.x - stackWidth) / flexibleCount;

    float position = size.x;

    for (List::const_iterator i = children.begin();  i != children.end();  i++)
    {
      if (*i == scroller)
	continue;

      float width = sizes[*i];
      if (width == 0.f)
	width = flexibleSize;

      position -= width + borderSize;
      (*i)->setArea(Rectangle(position, borderSize, width, height));
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

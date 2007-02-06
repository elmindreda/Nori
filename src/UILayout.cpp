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
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UILayout.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Layout::Layout(void):
  borderSize(0.f),
  parentAreaSlot(NULL)
{
}

float Layout::getBorderSize(void) const
{
  return borderSize;
}

void Layout::setBorderSize(float newSize)
{
  borderSize = newSize;
}

void Layout::addedChild(Widget& child)
{
  update();
}

void Layout::removedChild(Widget& child)
{
  update();
}

void Layout::addedToParent(Widget& parent)
{
  parentAreaSlot = parent.getAreaChangedSignal().connect(*this, &Layout::onAreaChanged);
  onAreaChanged(parent, parent.getArea());
}

void Layout::removedFromParent(Widget& parent)
{
  parentAreaSlot = NULL;
}

void Layout::onAreaChanged(Widget& parent, const Rectangle& area)
{
  setArea(Rectangle(Vector2::ZERO, area.size));
  update();
}

void Layout::update(void)
{
  const List& children = getChildren();

  float stackHeight = 0.f;

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
    stackHeight += (*i)->getArea().size.y + borderSize;

  const float width = getArea().size.x - borderSize * 2.f;

  float position = getArea().size.y - stackHeight;

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    const float height = (*i)->getArea().size.y;
    (*i)->setArea(Rectangle(borderSize, position, width, height));
    position += height + borderSize;
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

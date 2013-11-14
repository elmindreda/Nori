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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UILayout.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Layout::Layout(Layer& layer, Orientation orientation, bool expanding):
  Widget(layer),
  m_borderSize(0.f),
  m_orientation(orientation),
  m_expanding(expanding)
{
  if (!m_expanding)
    areaChangedSignal().connect(*this, &Layout::onAreaChanged);
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
  return m_expanding;
}

Orientation Layout::orientation() const
{
  return m_orientation;
}

float Layout::borderSize() const
{
  return m_borderSize;
}

void Layout::setBorderSize(float newSize)
{
  m_borderSize = newSize;
  update();
}

float Layout::childSize(Widget& child) const
{
  for (auto& s : m_sizes)
  {
    if (s.first == &child)
      return s.second;
  }

  panic("Unknown child widget found in layout");
}

void Layout::setChildSize(Widget& child, float newSize)
{
  for (auto& s : m_sizes)
  {
    if (s.first == &child)
    {
      s.second = newSize;
      return;
    }
  }

  m_sizes.push_back(Size(&child, newSize));
}

void Layout::addedChild(Widget& child)
{
  auto s = m_sizes.begin();

  for ( ;  s != m_sizes.end();  s++)
  {
    if (s->first == &child)
      break;
  }

  if (s == m_sizes.end())
  {
    if (m_orientation == VERTICAL)
      m_sizes.push_back(Size(&child, child.height()));
    else
      m_sizes.push_back(Size(&child, child.width()));
  }

  update();
}

void Layout::removedChild(Widget& child)
{
  for (auto s = m_sizes.begin();  s != m_sizes.end();  s++)
  {
    if (s->first == &child)
    {
      m_sizes.erase(s);
      break;
    }
  }

  update();
}

void Layout::onAreaChanged(Widget& widget)
{
  if (m_expanding)
    setArea(Rect(vec2(0.f), widget.size()));

  update();
}

void Layout::addedToParent(Widget& parent)
{
  if (m_expanding)
  {
    m_parentAreaSlot = parent.areaChangedSignal().connect(*this, &Layout::onAreaChanged);
    onAreaChanged(parent);
  }
}

void Layout::removedFromParent(Widget& parent)
{
  if (m_expanding)
    m_parentAreaSlot = nullptr;
}

void Layout::update()
{
  uint flexibleCount = 0;
  float stackSize = m_borderSize;

  for (auto& s : m_sizes)
  {
    if (s.second == 0.f)
      flexibleCount++;

    stackSize += s.second + m_borderSize;
  }

  if (m_orientation == VERTICAL)
  {
    const float childWidth = width() - m_borderSize * 2.f;
    float flexibleHeight = 0.f, positionY = height();

    if (flexibleCount)
      flexibleHeight = (height() - stackSize) / flexibleCount;

    for (auto c : children())
    {
      float childHeight = childSize(*c);
      if (childHeight == 0.f)
        childHeight = flexibleHeight;

      positionY -= childHeight + m_borderSize;
      c->setArea(Rect(m_borderSize, positionY, childWidth, childHeight));
    }
  }
  else
  {
    const float childHeight = height() - m_borderSize * 2.f;
    float flexibleWidth = 0.f, positionX = width();

    if (flexibleCount)
      flexibleWidth = (width() - stackSize) / flexibleCount;

    for (auto c : children())
    {
      float childWidth = childSize(*c);
      if (childWidth == 0.f)
        childWidth = flexibleWidth;

      positionX -= childWidth + m_borderSize;
      c->setArea(Rect(positionX, m_borderSize, childWidth, childHeight));
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

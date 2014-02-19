///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UICanvas.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Canvas::Canvas(Layer& layer):
  Widget(layer)
{
}

Canvas::Canvas(Widget& parent):
  Widget(parent)
{
}

SignalProxy<void, const Canvas&> Canvas::drawSignal()
{
  return m_drawSignal;
}

void Canvas::draw() const
{
  UI::Drawer& drawer = layer().drawer();
  RenderContext& context = drawer.context();

  const Recti area(0, 0, int(width()), int(height()));

  Recti oldViewport = context.viewportArea();
  Recti oldScissor = context.scissorArea();

  context.setViewportArea(area);
  context.setScissorArea(area);

  drawer.end();
  m_drawSignal(*this);
  drawer.begin();

  context.setViewportArea(oldViewport);
  context.setScissorArea(oldScissor);

  Widget::draw();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

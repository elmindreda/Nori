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

#include <wendy/Config.h>

#include <wendy/UIDrawer.h>
#include <wendy/UILayer.h>
#include <wendy/UIWidget.h>
#include <wendy/UICanvas.h>

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

SignalProxy1<void, const Canvas&> Canvas::getDrawSignal()
{
  return drawSignal;
}

void Canvas::draw() const
{
  UI::Drawer& drawer = getLayer().getDrawer();
  GL::Context& context = drawer.getContext();

  const Recti area(0, 0, int(getWidth()), int(getHeight()));

  Recti oldViewport = context.getViewportArea();
  Recti oldScissor = context.getScissorArea();

  context.setViewportArea(area);
  context.setScissorArea(area);

  drawer.end();
  drawSignal(*this);
  drawer.begin();

  context.setViewportArea(oldViewport);
  context.setScissorArea(oldScissor);

  Widget::draw();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

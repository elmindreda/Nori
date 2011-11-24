///////////////////////////////////////////////////////////////////////
// Wendy debug interface
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/UILabel.h>
#include <wendy/UILayout.h>

#include <wendy/DebugUI.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace debug
  {

///////////////////////////////////////////////////////////////////////

Panel::Panel(UI::Layer& layer):
  UI::Widget(layer)
{
}

void Panel::draw() const
{
  const Rect& area = getGlobalArea();

  UI::Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.fillRectangle(area, vec4(0.5f));
    UI::Widget::draw();
    drawer.popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

Interface::Interface(input::Context& context, UI::Drawer& drawer):
  UI::Layer(context, drawer),
  root(NULL)
{
  root = new Panel(*this);
  root->setArea(Rect(0.f, 0.f, 150.f, 400.f));
  addRootWidget(*root);

  UI::Layout* layout = new UI::Layout(*this, UI::VERTICAL, true);
  root->addChild(*layout);

  for (size_t i = 0;  i < LABEL_COUNT;  i++)
  {
    labels[i] = new UI::Label(*this);
    labels[i]->setTextAlignment(UI::RIGHT_ALIGNED);
    layout->addChild(*labels[i], 0.f);
  }
}

void Interface::update()
{
  GL::Stats* stats = getInputContext().getContext().getStats();

  if (stats)
  {
    const unsigned int rate = (unsigned int) (stats->getFrameRate() + 0.5f);
    labels[LABEL_FRAMERATE]->setText(format("%u fps", rate).c_str());

    const GL::Stats::Frame& frame = stats->getFrame();

    labels[LABEL_STATECHANGES]->setText(format("%u state changes", frame.stateChangeCount).c_str());
    labels[LABEL_OPERATIONS]->setText(format("%u operations", frame.operationCount).c_str());
    labels[LABEL_VERTICES]->setText(format("%u vertices", frame.vertexCount).c_str());
    labels[LABEL_POINTS]->setText(format("%u points", frame.pointCount).c_str());
    labels[LABEL_LINES]->setText(format("%u lines", frame.lineCount).c_str());
    labels[LABEL_TRIANGLES]->setText(format("%u triangles", frame.triangleCount).c_str());
  }
  else
  {
    for (size_t i = 0;  i < LABEL_COUNT;  i++)
      labels[i]->setText("No stats available");
  }
}

void Interface::draw()
{
  GL::Context& context = getInputContext().getContext();

  GL::Framebuffer& framebuffer = context.getCurrentFramebuffer();
  root->setSize(vec2(root->getWidth(), float(framebuffer.getHeight())));

  GL::Stats* previous = context.getStats();
  context.setStats(NULL);

  UI::Layer::draw();

  context.setStats(previous);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace debug*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

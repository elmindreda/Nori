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

namespace
{

unsigned int reduce(size_t value)
{
  while (value >= 1024)
    value /= 1024;

  return value;
}

const char* suffix(size_t value)
{
  if (value)
    value = size_t(glm::log(float(value)) / glm::log(1024.f));

  switch (value)
  {
    case 0:
      return "bytes";
    case 1:
      return "KB";
    case 2:
      return "MB";
    case 3:
      return "GB";
    case 4:
      return "TB";
    case 5:
      return "PB";
    case 6:
      return "EB";
  }

  return "wow";
}

} /*namespace*/

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

Interface::Interface(input::Window& window, UI::Drawer& drawer):
  UI::Layer(window, drawer),
  root(NULL)
{
  root = new Panel(*this);
  root->setArea(Rect(0.f, 0.f, 150.f, 220.f));
  addRootWidget(*root);

  UI::Layout* layout = new UI::Layout(*this, UI::VERTICAL, true);
  layout->setBorderSize(2.f);
  root->addChild(*layout);

  for (size_t i = 0;  i < ITEM_COUNT;  i++)
  {
    labels[i] = new UI::Label(*this);
    labels[i]->setTextAlignment(UI::RIGHT_ALIGNED);
    layout->addChild(*labels[i], 0.f);
  }
}

void Interface::update()
{
  GL::Stats* stats = getWindow().getContext().getStats();

  if (stats)
  {
    const GL::Stats::Frame& frame = stats->getCurrentFrame();

    updateCountItem(ITEM_FRAMERATE, "fps", (size_t) (stats->getFrameRate() + 0.5f));
    updateCountItem(ITEM_STATECHANGES, "states / f", frame.stateChangeCount);
    updateCountItem(ITEM_OPERATIONS, "operations / f", frame.operationCount);
    updateCountItem(ITEM_VERTICES, "vertices / f", frame.vertexCount);
    updateCountItem(ITEM_POINTS, "points / f", frame.pointCount);
    updateCountItem(ITEM_LINES, "lines / f", frame.lineCount);
    updateCountItem(ITEM_TRIANGLES, "triangles / f", frame.triangleCount);

    updateCountItem(ITEM_PROGRAMS, "programs", stats->getProgramCount());
    updateCountSizeItem(ITEM_TEXTURES,
                        "textures",
                        stats->getTextureCount(),
                        stats->getTotalTextureSize());
    updateCountSizeItem(ITEM_VERTEXBUFFERS,
                        "VBs",
                        stats->getVertexBufferCount(),
                        stats->getTotalVertexBufferSize());
    updateCountSizeItem(ITEM_INDEXBUFFERS,
                        "IBs",
                        stats->getIndexBufferCount(),
                        stats->getTotalIndexBufferSize());
    updateCountSizeItem(ITEM_RENDERBUFFERS,
                        "RBs",
                        stats->getRenderBufferCount(),
                        stats->getTotalRenderBufferSize());
  }
  else
  {
    for (size_t i = 0;  i < ITEM_COUNT;  i++)
      labels[i]->setText("No stats available");
  }
}

void Interface::draw()
{
  GL::Context& context = getWindow().getContext();

  GL::Framebuffer& framebuffer = context.getCurrentFramebuffer();
  root->setSize(vec2(150.f, float(framebuffer.getHeight())));

  GL::Stats* previous = context.getStats();
  context.setStats(NULL);

  UI::Layer::draw();

  context.setStats(previous);
}

void Interface::updateCountItem(Item item, const char* unit, size_t count)
{
  labels[item]->setText(format("%u %s", (unsigned int) count, unit).c_str());
}

void Interface::updateCountSizeItem(Item item,
                                    const char* unit,
                                    size_t count,
                                    size_t size)
{
  labels[item]->setText(format("%u %s (%u %s)",
                               (unsigned int) count,
                               unit,
                               reduce(size),
                               suffix(size)).c_str());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace debug*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

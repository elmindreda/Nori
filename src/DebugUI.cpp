///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Config.hpp>

#include <wendy/Drawer.hpp>
#include <wendy/Layer.hpp>
#include <wendy/Widget.hpp>
#include <wendy/Label.hpp>
#include <wendy/Layout.hpp>

#include <wendy/DebugUI.hpp>

namespace wendy
{
  namespace debug
  {

namespace
{

uint reduce(size_t value)
{
  while (value >= 1024)
    value /= 1024;

  return uint(value);
}

const char* suffix(size_t value)
{
  const char* suffixes[] = { "bytes", "KB", "MB", "GB", "TB", "PB", "EB" };

  if (value == 0)
    return suffixes[0];

  return suffixes[uint(glm::log(double(value)) / glm::log(1024.0))];
}

} /*namespace*/

Panel::Panel(Layer& layer):
  Widget(layer)
{
}

void Panel::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.fillRectangle(area, vec4(0.5f));
    Widget::draw();
    drawer.popClipArea();
  }
}

Interface::Interface(Window& window, Drawer& drawer):
  Layer(drawer),
  root(nullptr)
{
  root = new Panel(*this);
  root->setArea(Rect(0.f, 0.f, 150.f, 220.f));

  Layout* layout = new Layout(*this, root, VERTICAL, COVER_PARENT);
  layout->setBorderSize(2.f);

  for (size_t i = 0;  i < ITEM_COUNT;  i++)
  {
    labels[i] = new Label(*this, layout);
    labels[i]->setTextAlignment(RIGHT_ALIGNED);
  }
}

void Interface::update()
{
  RenderStats* stats = drawer().context().stats();

  if (stats)
  {
    const RenderStats::Frame& frame = stats->currentFrame();

    updateCountItem(ITEM_FRAMERATE, "fps", (size_t) (stats->frameRate() + 0.5f));
    updateCountItem(ITEM_STATECHANGES, "states / f", frame.stateChangeCount);
    updateCountItem(ITEM_OPERATIONS, "operations / f", frame.operationCount);
    updateCountItem(ITEM_VERTICES, "vertices / f", frame.vertexCount);
    updateCountItem(ITEM_POINTS, "points / f", frame.pointCount);
    updateCountItem(ITEM_LINES, "lines / f", frame.lineCount);
    updateCountItem(ITEM_TRIANGLES, "triangles / f", frame.triangleCount);

    updateCountItem(ITEM_PROGRAMS, "programs", stats->programCount());
    updateCountSizeItem(ITEM_TEXTURES,
                        "textures",
                        stats->textureCount(),
                        stats->totalTextureSize());
    updateCountSizeItem(ITEM_VERTEXBUFFERS,
                        "VBs",
                        stats->vertexBufferCount(),
                        stats->totalVertexBufferSize());
    updateCountSizeItem(ITEM_INDEXBUFFERS,
                        "IBs",
                        stats->indexBufferCount(),
                        stats->totalIndexBufferSize());
  }
  else
  {
    for (size_t i = 0;  i < ITEM_COUNT;  i++)
      labels[i]->setText("No stats available");
  }
}

void Interface::draw()
{
  RenderContext& context = drawer().context();

  Framebuffer& framebuffer = context.framebuffer();
  root->setSize(vec2(150.f, float(framebuffer.height())));

  RenderStats* previous = context.stats();
  context.setStats(nullptr);

  Layer::draw();

  context.setStats(previous);
}

void Interface::updateCountItem(Item item, const char* unit, size_t count)
{
  labels[item]->setText(format("%u %s", (uint) count, unit).c_str());
}

void Interface::updateCountSizeItem(Item item,
                                    const char* unit,
                                    size_t count,
                                    size_t size)
{
  labels[item]->setText(format("%u %s (%u %s)",
                               (uint) count,
                               unit,
                               reduce(size),
                               suffix(size)).c_str());
}

  } /*namespace debug*/
} /*namespace wendy*/


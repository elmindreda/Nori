///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Core.hpp>
#include <nori/Rect.hpp>
#include <nori/Path.hpp>
#include <nori/Resource.hpp>
#include <nori/Pixel.hpp>
#include <nori/Image.hpp>

#include <nori/Texture.hpp>
#include <nori/Program.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/RenderContext.hpp>

#include <nori/Theme.hpp>
#include <nori/Layer.hpp>
#include <nori/Widget.hpp>
#include <nori/Label.hpp>
#include <nori/Layout.hpp>

#include <nori/DebugUI.hpp>

namespace nori
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
  Theme& theme = layer().theme();

  const Rect area = globalArea();
  if (theme.pushClipArea(area))
  {
    VectorContext& vc = theme.context();
    vc.beginPath();
    vc.rect(area);
    vc.fillColor(vec4(0.5f));
    vc.fill();

    Widget::draw();
    theme.popClipArea();
  }
}

Interface::Interface(Theme& theme):
  Layer(theme),
  root(nullptr)
{
  root = new Panel(*this);
  root->setArea(Rect(0.f, 0.f, 150.f, 220.f));

  Layout* layout = new Layout(*this, root, VERTICAL, COVER_PARENT);
  layout->setBorderSize(2.f);

  for (size_t i = 0;  i < ITEM_COUNT;  i++)
  {
    labels[i] = new Label(*this, layout);
    labels[i]->setTextAlignment(ALIGN_RIGHT | ALIGN_MIDDLE);
  }
}

void Interface::update()
{
  RenderStats* stats = theme().context().context().stats();

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
  RenderContext& context = theme().context().context();

  Framebuffer& framebuffer = context.framebuffer();
  root->setSize(vec2(150.f, float(framebuffer.height())));

  RenderStats* previous = context.stats();
  context.setStats(nullptr);

  Layer::draw();

  context.setStats(previous);
}

void Interface::updateCountItem(Item item, const char* unit, size_t count)
{
  labels[item]->setText(format("%u %s", (uint) count, unit));
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
                               suffix(size)));
}

  } /*namespace debug*/
} /*namespace nori*/


///////////////////////////////////////////////////////////////////////
// Wendy debug tools
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLStatistics.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/DebugHUD.h>

#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace debug
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

void HUD::draw(void) const
{
  GL::Statistics* statistics = GL::Statistics::get();

  std::stringstream text;
  text << "Vertices: " << statistics->getFrame().vertexCount << '\n';
  text << "FPS: " << statistics->getFrameRate() << '\n';

  GL::Canvas* canvas = GL::Canvas::getCurrent();

  Rectangle area(0.f, 0.f, (float) canvas->getPhysicalWidth(),
                           (float) canvas->getPhysicalHeight());

  Rectangle metrics = font->getTextMetrics(text.str());

  Vector2 penPosition;
  penPosition.x = area.getCenter().x - metrics.getCenter().x;
  penPosition.y = area.getCenter().y - metrics.getCenter().y;

  GL::Renderer* renderer = GL::Renderer::get();

  renderer->begin2D(Vector2((float) canvas->getPhysicalWidth(),
                            (float) canvas->getPhysicalHeight()));

  font->setColor(ColorRGBA::WHITE);
  font->setPenPosition(penPosition);
  font->drawText(text.str());

  renderer->end();
}

bool HUD::create(void)
{
  Ptr<HUD> display = new HUD();
  if (!display->init())
    return false;

  set(display.detachObject());
  return true;
}

HUD::HUD(void)
{
}

bool HUD::init(void)
{
  if (!GL::Statistics::get())
  {
    Log::writeError("Cannot create debug HUD without statistics tracker");
    return false;
  }

  font = render::Font::readInstance("default");
  if (!font)
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace debug*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

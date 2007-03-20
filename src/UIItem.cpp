//////////////////////////////////////////////////////////////////////
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
#include <wendy/UIItem.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Item::Item(const String& initValue):
  value(initValue)
{
}

bool Item::operator < (const Item& other) const
{
  return value < other.value;
}

float Item::getWidth(void) const
{
  render::Font* font = Renderer::get()->getCurrentFont();

  float width = font->getWidth() * 2.f;

  if (value.empty())
    width += font->getWidth() * 3.f;
  else
    width += font->getTextMetrics(value).size.x;

  return width;
}

float Item::getHeight(void) const
{
  return Renderer::get()->getCurrentFont()->getHeight() * 1.5f;
}

const String& Item::getValue(void) const
{
  return value;
}

void Item::setValue(const String& newValue)
{
  value = newValue;
}

void Item::draw(const Rectangle& area, bool selected) const
{
  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    render::Font* font = renderer->getCurrentFont();

    Rectangle textArea = area;
    textArea.position.x += font->getWidth() / 2.f;
    textArea.size.x -= font->getWidth();

    if (selected)
    {
      GL::Pass pass;
      pass.setDefaultColor(ColorRGBA(renderer->getSelectionColor(), 1.f));
      pass.setDepthTesting(false);
      pass.setDepthWriting(false);
      pass.apply();

      glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);
    }

    renderer->drawText(textArea, value, LEFT_ALIGNED, selected);
    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

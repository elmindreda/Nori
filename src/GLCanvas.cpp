///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>

#define GLEW_STATIC
#include <GL/glew.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Canvas::Canvas(void)
{
  scissorStack.push(Rect(0.f, 0.f, 1.f, 1.f));
  viewportArea.set(0.f, 0.f, 1.f, 1.f);
}

Canvas::~Canvas(void)
{
  if (current == this)
  {
    Log::writeError("Destruction of current canvas detected; don't do this");
    current = NULL;
  }
}

void Canvas::begin(void) const
{
  if (current)
    throw Exception("Cannot nest canvases");

  apply();

  updateViewportArea();

  current = const_cast<Canvas*>(this);
}

void Canvas::end(void) const
{
  if (current != this)
    throw Exception("Cannot end non-current canvas");

  finish();
  current = NULL;
}

bool Canvas::pushScissorArea(const Rect& area)
{
  if (!scissorStack.push(area))
    return false;

  if (current == this)
    updateScissorArea();

  return true;
}

void Canvas::popScissorArea(void)
{
  if (scissorStack.getCount() == 1)
    throw Exception("Cannot pop empty scissor clip stack");

  scissorStack.pop();

  if (scissorStack.getTotal() != Rect(0.f, 0.f, 1.f, 1.f))
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);

  if (current == this)
    updateScissorArea();
}

void Canvas::clearColorBuffer(const ColorRGBA& color) const
{
  if (getCurrent() != this)
  {
    Log::writeError("Cannot clear non-current canvas");
    return;
  }

  glPushAttrib(GL_COLOR_BUFFER_BIT);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
  glPopAttrib();
}

void Canvas::clearDepthBuffer(float depth) const
{
  if (getCurrent() != this)
  {
    Log::writeError("Cannot clear non-current canvas");
    return;
  }

  glPushAttrib(GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_TRUE);
  glClearDepth(depth);
  glClear(GL_DEPTH_BUFFER_BIT);
  glPopAttrib();
}

void Canvas::clearStencilBuffer(unsigned int value) const
{
  if (getCurrent() != this)
  {
    Log::writeError("Cannot clear non-current canvas");
    return;
  }

  glPushAttrib(GL_STENCIL_BUFFER_BIT);
  glStencilMask(GL_TRUE);
  glClearStencil(value);
  glClear(GL_STENCIL_BUFFER_BIT);
  glPopAttrib();
}

float Canvas::getPhysicalAspectRatio(void) const
{
  return getPhysicalWidth() / (float) getPhysicalHeight();
}

const Rect& Canvas::getScissorArea(void) const
{
  return scissorStack.getTotal();
}

const Rect& Canvas::getViewportArea(void) const
{
  return viewportArea;
}

void Canvas::setViewportArea(const Rect& newArea)
{
  viewportArea = newArea;
  viewportArea.clipBy(Rect(0.f, 0.f, 1.f, 1.f));

  if (current == this)
    updateViewportArea();
}

Canvas* Canvas::getCurrent(void)
{
  return current;
}

Canvas::Canvas(const Canvas& source)
{
  // NOTE: Not implemented.
}

Canvas& Canvas::operator = (const Canvas& source)
{
  // NOTE: Not implemented.

  return *this;
}

Canvas* Canvas::current = NULL;

///////////////////////////////////////////////////////////////////////

unsigned int ScreenCanvas::getPhysicalWidth(void) const
{
  return Context::get()->getWidth();
}

unsigned int ScreenCanvas::getPhysicalHeight(void) const
{
  return Context::get()->getHeight();
}

void ScreenCanvas::apply(void) const
{
}

void ScreenCanvas::finish(void) const
{
}

void ScreenCanvas::updateScissorArea(void) const
{
  const Rect& viewportArea = getViewportArea();

  Rect area = getScissorArea();
  area *= viewportArea.size;
  area.position += viewportArea.position;

  const unsigned int width = getPhysicalWidth();
  const unsigned int height = getPhysicalHeight();

  glScissor((GLint) floorf(area.position.x * width),
	    (GLint) floorf(area.position.y * height),
	    (GLsizei) ceilf(area.size.x * width),
	    (GLsizei) ceilf(area.size.y * height));

  if (area == Rect(0.f, 0.f, 1.f, 1.f))
    glDisable(GL_SCISSOR_TEST);
  else
    glEnable(GL_SCISSOR_TEST);
}

void ScreenCanvas::updateViewportArea(void) const
{
  const Rect& area = getViewportArea();

  const unsigned int width = getPhysicalWidth();
  const unsigned int height = getPhysicalHeight();

  glViewport((GLint) (area.position.x * width),
             (GLint) (area.position.y * height),
	     (GLsizei) (area.size.x * width),
	     (GLsizei) (area.size.y * height));

  updateScissorArea();
}

///////////////////////////////////////////////////////////////////////

unsigned int TextureCanvas::getPhysicalWidth(void) const
{
  return texture->getPhysicalWidth();
}

unsigned int TextureCanvas::getPhysicalHeight(void) const
{
  return texture->getPhysicalHeight();
}

Texture& TextureCanvas::getTexture(void) const
{
  return *texture;
}

TextureCanvas* TextureCanvas::createInstance(Texture& texture)
{
  Ptr<TextureCanvas> canvas = new TextureCanvas();
  if (!canvas->init(texture))
    return false;

  return canvas.detachObject();
}

bool TextureCanvas::init(Texture& initTexture)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create texture canvas without OpenGL context");
    return false;
  }

  texture = &initTexture;

  return true;
}

void TextureCanvas::apply(void) const
{
}

void TextureCanvas::finish(void) const
{
  // TODO: Implement.

  /*
  TextureLayer layer(0);
  layer.setTexture(texture);
  layer.apply();

  if (texture->getTarget() == GL_TEXTURE_1D)
    glCopyTexSubImage1D(texture->getTarget(),
                        0, 0, 0, 0,
                        texture->getPhysicalWidth());
  else
    glCopyTexSubImage2D(texture->getTarget(),
                        0, 0, 0, 0, 0,
                        texture->getPhysicalWidth(),
                        texture->getPhysicalHeight());
  */
}

void TextureCanvas::updateScissorArea(void) const
{
  const Rect& viewportArea = getViewportArea();

  Rect area = getScissorArea();
  area *= viewportArea.size;
  area.position += viewportArea.position;

  const unsigned int width = getPhysicalWidth();
  const unsigned int height = getPhysicalHeight();

  glScissor((GLint) (area.position.x * width),
	    (GLint) (area.position.y * height),
	    (GLsizei) (area.size.x * width),
	    (GLsizei) (area.size.y * height));

  if (area == Rect(0.f, 0.f, 1.f, 1.f))
    glDisable(GL_SCISSOR_TEST);
  else
    glEnable(GL_SCISSOR_TEST);
}

void TextureCanvas::updateViewportArea(void) const
{
  const Rect& area = getViewportArea();

  const unsigned int width = getPhysicalWidth();
  const unsigned int height = getPhysicalHeight();

  glViewport((GLint) (area.position.x * width),
             (GLint) (area.position.y * height),
	     (GLsizei) (area.size.x * width),
	     (GLsizei) (area.size.y * height));

  updateScissorArea();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

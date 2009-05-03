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

float Canvas::getPhysicalAspectRatio(void) const
{
  return getPhysicalWidth() / (float) getPhysicalHeight();
}

Canvas::Canvas(void)
{
}

Canvas::~Canvas(void)
{
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

///////////////////////////////////////////////////////////////////////

unsigned int TextureCanvas::getPhysicalWidth(void) const
{
  return width;
}

unsigned int TextureCanvas::getPhysicalHeight(void) const
{
  return height;
}

Texture* TextureCanvas::getColorBufferTexture(void) const
{
  return texture;
}

void TextureCanvas::setColorBufferTexture(Texture* newTexture, unsigned int newLevel)
{
  if (texture)
    finish();

  texture = newTexture;
  level = newLevel;
}

TextureCanvas* TextureCanvas::createInstance(unsigned int width, unsigned int height)
{
  Ptr<TextureCanvas> canvas = new TextureCanvas();
  if (!canvas->init(width, height))
    return false;

  return canvas.detachObject();
}

bool TextureCanvas::init(unsigned int initWidth, unsigned int initHeight)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create texture canvas without OpenGL context");
    return false;
  }

  width = initWidth;
  height = initHeight;

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

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

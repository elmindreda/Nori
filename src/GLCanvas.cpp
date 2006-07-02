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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Log.h>
#include <moira/Vector.h>
#include <moira/Color.h>
#include <moira/Image.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

unsigned int getClosestPower(unsigned int value, unsigned int maximum)
{
  unsigned int result;

  if (value > maximum)
    result = maximum;
  else
    result = value;

  if (result & (result - 1))
  {
    // value is not power of two
    // find largest power lesser than maximum

    unsigned int i;

    for (i = 0;  result & ~1;  i++)
      result >>= 1;

    result = 1 << i;
  }

  if ((result << 1) > maximum)
  {
    // maximum is not power of two, so we give up here
    return result;
  }

  if (value > result)
  {
    // there is room to preserve all detail, so use it
    return result << 1;
  }

  return result;
}

}

///////////////////////////////////////////////////////////////////////

Canvas::Canvas(void)
{
  size.set(1.f, 1.f);
  position.set(0.f, 0.f);
}

Canvas::~Canvas(void)
{
}

void Canvas::begin2D(const Vector2& resolution) const
{
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.f, resolution.x, 0.f, resolution.y);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPopAttrib();
}

void Canvas::begin3D(float FOV, float aspect, float nearZ, float farZ) const
{
  if (aspect == 0.f)
    aspect = (float) getPhysicalWidth() / (float) getPhysicalHeight();

  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPerspective(FOV, aspect, nearZ, farZ);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPopAttrib();
}
  
void Canvas::end(void) const
{
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

void Canvas::push(void) const
{
  stack.push(const_cast<Canvas*>(this));

  apply();
}

void Canvas::pop(void) const
{
  if (stack.top() != this)
    throw Exception("Canvas stack pop out of order");

  stack.pop();

  if (!stack.empty())
    stack.top()->apply();
}

void Canvas::clearColor(const ColorRGBA& color)
{
  glPushAttrib(GL_COLOR_BUFFER_BIT);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
  glPopAttrib();
}

void Canvas::clearDepth(float depth)
{
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_TRUE);
  glClearDepth(depth);
  glClear(GL_DEPTH_BUFFER_BIT);
  glPopAttrib();
}

void Canvas::clearStencil(unsigned int value)
{
  glPushAttrib(GL_STENCIL_BUFFER_BIT);
  glStencilMask(GL_TRUE);
  glClearStencil(value);
  glClear(GL_STENCIL_BUFFER_BIT);
  glPopAttrib();
}

const Vector2& Canvas::getAreaPosition(void) const
{
  return position;
}

const Vector2& Canvas::getAreaSize(void) const
{
  return size;
}

void Canvas::setArea(const Vector2& newPosition, const Vector2& newSize)
{
  position = newPosition;
  size = newSize;
}

Canvas* Canvas::getCurrent(void)
{
  if (stack.empty())
    return NULL;

  return stack.top();
}

Canvas::CanvasStack Canvas::stack;

///////////////////////////////////////////////////////////////////////

void ContextCanvas::apply(void) const
{
  const unsigned int width = Context::get()->getWidth();
  const unsigned int height = Context::get()->getHeight();

  glViewport((GLint) (position.x * width),
             (GLint) (position.y * height),
	     (GLsizei) (size.x * width),
	     (GLsizei) (size.y * height));

  if (position == Vector2::ZERO && size == Vector2::ONE)
    glDisable(GL_SCISSOR_TEST);
  else
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint) (position.x * width),
              (GLint) (position.y * height),
	      (GLsizei) (size.x * width),
	      (GLsizei) (size.y * height));
  }
}

unsigned int ContextCanvas::getPhysicalWidth(void) const
{
  return (unsigned int) (Context::get()->getWidth() * size.x);
}

unsigned int ContextCanvas::getPhysicalHeight(void) const
{
  return (unsigned int) (Context::get()->getHeight() * size.y);
}

///////////////////////////////////////////////////////////////////////

void TextureCanvas::pop(void) const
{
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture(texture->getTarget(), texture->getGLID());

  if (texture->getTarget() == GL_TEXTURE_1D)
    glCopyTexSubImage1D(texture->getTarget(),
                        0, 0, 0, 0,
                        texture->getWidth());
  else
    glCopyTexSubImage2D(texture->getTarget(),
                        0, 0, 0, 0, 0,
                        texture->getWidth(),
                        texture->getHeight());

  glPopAttrib();

  Canvas::pop();
}

unsigned int TextureCanvas::getPhysicalWidth(void) const
{
  return (unsigned int) (texture->getWidth() * size.x);
}

unsigned int TextureCanvas::getPhysicalHeight(void) const
{
  return (unsigned int) (texture->getHeight() * size.y);
}

const Texture& TextureCanvas::getTexture(void) const
{
  return *texture;
}

TextureCanvas* TextureCanvas::createInstance(const std::string& textureName,
                                             unsigned int width,
                                             unsigned int height)
{
  Ptr<TextureCanvas> canvas = new TextureCanvas();
  if (!canvas->init(textureName, width, height))
    return false;

  return canvas.detachObject();
}

TextureCanvas::TextureCanvas(void)
{
}

bool TextureCanvas::init(const std::string& textureName, unsigned int width, unsigned int height)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create texture canvas without OpenGL context");
    return false;
  }

  unsigned int maxSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*) &maxSize);

  unsigned int maxWidth = Context::get()->getWidth();
  if (maxWidth > maxSize)
    maxWidth = maxSize;

  unsigned int maxHeight = Context::get()->getHeight();
  if (maxHeight > maxSize)
    maxHeight = maxSize;

  if (GLEW_ARB_texture_non_power_of_two)
  {
    if (width > maxWidth)
    {
      height = (unsigned int) ((float) height * (float) maxWidth / (float) width);
      width = maxWidth;
    }

    if (height > maxHeight)
    {
      width = (unsigned int) ((float) width * (float) maxHeight / (float) height);
      height = maxHeight;
    }
  }
  else
  {
    width = getClosestPower(width, maxWidth);
    height = getClosestPower(height, maxHeight);
  }

  Image image(ImageFormat::RGB888, width, height);

  unsigned int flags = 0;
  if ((width & (width - 1)) || (height & (height - 1)))
    flags |= Texture::RECTANGULAR;

  texture = Texture::createInstance(textureName, image, flags);
  if (!texture)
    return false;

  return true;
}

void TextureCanvas::apply(void) const
{
  const unsigned int width = texture->getWidth();
  const unsigned int height = texture->getHeight();

  glViewport((GLint) (position.x * width),
             (GLint) (position.y * height),
	     (GLsizei) (size.x * width),
	     (GLsizei) (size.y * height));

  if (position == Vector2::ZERO && size == Vector2::ONE)
    glDisable(GL_SCISSOR_TEST);
  else
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint) (position.x * width),
              (GLint) (position.y * height),
	      (GLsizei) (size.x * width),
	      (GLsizei) (size.y * height));
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

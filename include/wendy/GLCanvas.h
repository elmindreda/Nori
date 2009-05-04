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
#ifndef WENDY_GLCANVAS_H
#define WENDY_GLCANVAS_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Texture;

///////////////////////////////////////////////////////////////////////

/*! @brief Rendering canvas.
 *  @ingroup opengl
 *
 *  This class represents a render target, i.e. a framebuffer.
 */
class Canvas
{
  friend class Renderer;
public:
  /*! @return The width, in pixels, of this canvas.
   */
  virtual unsigned int getPhysicalWidth(void) const = 0;
  /*! @return The height, in pixels, of this canvas.
   */
  virtual unsigned int getPhysicalHeight(void) const = 0;
  /*! @return The aspect ratio of the dimensions, in pixels, of this canvas.
   */
  float getPhysicalAspectRatio(void) const;
protected:
  /*! Constructor.
   */
  Canvas(void);
  /*! Destructor.
   */
  virtual ~Canvas(void);
  /*! Called when this canvas is to be made current.
   */
  virtual void apply(void) const = 0;
  virtual void finish(void) const = 0;
private:
  Canvas(const Canvas& source);
  Canvas& operator = (const Canvas& source);
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Canvas for rendering to the screen.
 *  @ingroup opengl
 */
class ScreenCanvas : public Canvas
{
  friend class Renderer;
public:
  unsigned int getPhysicalWidth(void) const;
  unsigned int getPhysicalHeight(void) const;
private:
  ScreenCanvas(void);
  void apply(void) const;
  void finish(void) const;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Canvas for rendering to a texture.
 *  @ingroup opengl
 */
class TextureCanvas : public Canvas
{
public:
  unsigned int getPhysicalWidth(void) const;
  unsigned int getPhysicalHeight(void) const;
  /*! @return The texture that this canvas uses as a color buffer.
   */
  Texture* getColorBufferTexture(void) const;
  /*! Sets the texture to use as the color buffer for this canvas.
   *  @param[in] newTexture The desired texture, or @c NULL to detach the currently set texture.
   *  @remarks A texture canvas cannot be rendered to without a texture to use as color buffer.
   */
  void setColorBufferTexture(Texture* newTexture);
  /*! Creates a texture canvas for the specified texture.
   *  @param texture The texture for which to create a texture canvas.
   */
  static TextureCanvas* createInstance(unsigned int width, unsigned int height);
private:
  TextureCanvas(void);
  bool init(unsigned int width, unsigned int height);
  void apply(void) const;
  void finish(void) const;
  unsigned int width;
  unsigned int height;
  Ref<Texture> texture;
  unsigned int level;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCANVAS_H*/
///////////////////////////////////////////////////////////////////////

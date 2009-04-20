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
 *  This class represents a render target, i.e. a framebuffer binding.
 *
 *  @remarks Most higher-level objects capable of rendering wont work
 *  without an active canvas.
 */
class Canvas
{
public:
  /*! Constructor.
   */
  Canvas(void);
  /*! Destructor.
   */
  virtual ~Canvas(void);
  /*! Makes this the current canvas and begins rendering.
   */
  void begin(void) const;
  /*! Finishes rendering to this canvas.  After this call, this canvas
   *  will no longer be current.
   */
  void end(void) const;
  /*! Pushes the specified area onto the scissor area clip stack.  The
   *  resulting scissor area is the specified scissor area clipped by the
   *  current scissor area.
   *  @param area The desired area to push.
   *  @return @c true if the resulting scissor area has a non-zero size,
   *  otherwise @c false.
   *  @remarks If the resulting scissor area is empty, it is not pushed
   *  onto the stack, so you do not need to (and should not) pop it. The
   *  recommended pattern is:
   *  @code
   *  if (canvas.pushScissorArea(childArea))
   *  {
   *	drawStuff();
   *	canvas.popScissorArea();
   *  }
   *  @endcode
   */
  bool pushScissorArea(const Rect& area);
  /*! Pops the top area from the scissor area clip stack.
   */
  void popScissorArea(void);
  /*! Clears the color buffer of this canvas with the specified color.
   */
  void clearColorBuffer(const ColorRGBA& color = ColorRGBA::BLACK) const;
  /*! Clears the depth buffer of this canvas with the specified depth
   *  value.
   */
  void clearDepthBuffer(float depth = 1.f) const;
  /*! Clears the stencil buffer of this canvas with the specified
   *  stencil value.
   */
  void clearStencilBuffer(unsigned int value = 0) const;
  /*! @return The width, in pixels, of this canvas.
   */
  virtual unsigned int getPhysicalWidth(void) const = 0;
  /*! @return The height, in pixels, of this canvas.
   */
  virtual unsigned int getPhysicalHeight(void) const = 0;
  /*! @return The aspect ratio of the dimensions, in pixels, of this canvas.
   */
  float getPhysicalAspectRatio(void) const;
  /*! @return The scissor rectangle of this canvas.
   */
  const Rect& getScissorArea(void) const;
  /*! @return The viewport rectangle of this canvas.
   */
  const Rect& getViewportArea(void) const;
  /*! Sets the viewport rectangle for this canvas.
   *  @param[in] newArea The desired viewport rectangle.
   */
  void setViewportArea(const Rect& newArea);
  /*! @return The current canvas, or @c NULL if there is no current
   *  canvas.
   */
  static Canvas* getCurrent(void);
protected:
  /*! Called when this canvas is to be made current.
   */
  virtual void apply(void) const = 0;
  virtual void finish(void) const = 0;
  virtual void updateScissorArea(void) const = 0;
  virtual void updateViewportArea(void) const = 0;
private:
  Canvas(const Canvas& source);
  Canvas& operator = (const Canvas& source);
  RectClipStack scissorStack;
  Rect viewportArea;
  static Canvas* current;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Canvas for rendering to the screen.
 *  @ingroup opengl
 */
class ScreenCanvas : public Canvas
{
public:
  unsigned int getPhysicalWidth(void) const;
  unsigned int getPhysicalHeight(void) const;
private:
  void apply(void) const;
  void finish(void) const;
  void updateScissorArea(void) const;
  void updateViewportArea(void) const;
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
  /*! @return The texture targeted by this texture canvas.
   */
  Texture& getTexture(void) const;
  /*! Creates a texture canvas for the specified texture.
   *  @param texture The texture for which to create a texture canvas.
   */
  static TextureCanvas* createInstance(Texture& texture);
private:
  bool init(Texture& texture);
  void apply(void) const;
  void finish(void) const;
  void updateScissorArea(void) const;
  void updateViewportArea(void) const;
  Ref<Texture> texture;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCANVAS_H*/
///////////////////////////////////////////////////////////////////////

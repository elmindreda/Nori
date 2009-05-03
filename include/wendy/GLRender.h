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
#ifndef WENDY_GLRENDER_H
#define WENDY_GLRENDER_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Canvas;

///////////////////////////////////////////////////////////////////////

/*! @brief The renderer singleton.
 *  @ingroup opengl
 *
 *  This is the central renderer class for the OpenGL layer.
 */
class Renderer : public Trackable, public Singleton<Renderer>
{
public:
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
   *  if (renderer.pushScissorArea(childArea))
   *  {
   *	drawStuff();
   *	renderer.popScissorArea();
   *  }
   *  @endcode
   */
  bool pushScissorArea(const Rect& area);
  /*! Pops the top area from the scissor area clip stack.
   */
  void popScissorArea(void);
  void pushTransform(const Mat4& transform);
  void popTransform(void);
  /*! Clears the current color buffer with the specified color.
   *  @param[in] color The color value to clear the color buffer with.
   */
  void clearColorBuffer(const ColorRGBA& color = ColorRGBA::BLACK);
  /*! Clears the current depth buffer with the specified depth value.
   *  @param[in] depth The depth value to clear the depth buffer with.
   */
  void clearDepthBuffer(float depth = 1.f);
  /*! Clears the current stencil buffer with the specified stencil value.
   *  @param[in] value The stencil value to clear the stencil buffer with.
   */
  void clearStencilBuffer(unsigned int value = 0);
  /*! Renders the current primitive range to the current canvas, using the
   *  current shader program and transforms.
   *  @pre A shader program must be set before calling this method.
   */
  void render(void);
  /*! Allocates a range of temporary indices of the specified type.
   *  @param[out] range The newly allocated index range.
   *  @param[in] count The number of indices to allocate.
   *  @param[in] type The type of indices to allocate.
   *  @return @c true if the allocation succeeded, or @c false if an
   *  error occurred.
   *  @remarks The allocated index range is only valid until the end of the
   *  current frame.
   */
  bool allocateIndices(IndexRange& range,
		       unsigned int count,
                       IndexBuffer::Type type = IndexBuffer::UBYTE);
  /*! Allocates a range of temporary vertices of the specified format.
   *  @param[out] range The newly allocated vertex range.
   *  @param[in] count The number of vertices to allocate.
   *  @param[in] format The format of vertices to allocate.
   *  @return @c true if the allocation succeeded, or @c false if an
   *  error occurred.
   *  @remarks The allocated vertex range is only valid until the end of the
   *  current frame.
   */
  bool allocateVertices(VertexRange& range,
			unsigned int count,
                        const VertexFormat& format);
  bool isReservedUniform(const String& name) const;
  Context& getContext(void) const;
  Texture& getDefaultTexture(void) const;
  Program& getDefaultProgram(void) const;
  /*! @return The current scissor rectangle.
   */
  const Rect& getScissorArea(void) const;
  /*! @return The current viewport rectangle.
   */
  const Rect& getViewportArea(void) const;
  /*! Sets the current viewport rectangle.
   *  @param[in] newArea The desired viewport rectangle.
   */
  void setViewportArea(const Rect& newArea);
  Canvas& getCurrentCanvas(void) const;
  Program* getCurrentProgram(void) const;
  const PrimitiveRange& getCurrentPrimitiveRange(void) const;
  const Mat4& getProjectionMatrix(void) const;
  const Mat4& getModelViewMatrix(void) const;
  void setScreenCanvasCurrent(void);
  void setCurrentCanvas(Canvas& newCanvas);
  /*! Sets the projection matrix.
   *  @param[in] newProjection The desired projection matrix.
   */
  void setProjectionMatrix(const Mat4& newProjection);
  /*! Sets an orthographic projection matrix as ([0..width], [0..height], [-1, 1]).
   *  @param[in] width The width of the projected space.
   *  @param[in] height The height of the projected space.
   */
  void setProjectionMatrix2D(float width, float height);
  /*! Sets a perspective projection matrix.
   *  @param[in] FOV The desired field of view of the projection.
   *  @param[in] aspect The desired aspect ratio of the projection.
   *  @param[in] nearZ The desired near plane distance of the projection.
   *  @param[in] farZ The desired far plane distance of the projection.
   *  @remarks If @a aspect is set to zero, the aspect ratio is calculated from
   *  the dimensions of the current canvas.
   *  @see Canvas::getPhysicalAspectRatio
   */
  void setProjectionMatrix3D(float FOV = 90.f,
                             float aspect = 0.f,
	                     float nearZ = 0.01f,
	                     float farZ = 1000.f);
  /*! Sets the current shader program for use when rendering.
   *  @param[in] newProgram The desired shader program, or @c NULL to detach
   *  the current shader program.
   */
  void setCurrentProgram(Program* newProgram);
  /*! Sets the current primitive range for use when rendering.
   *  @param[in] newRange The desired primitive range to use.
   */
  void setCurrentPrimitiveRange(const PrimitiveRange& newRange);
  /*! Creates the renderer singleton.
   *  @param[in] context The context to create the renderer for.
   *  @return @c true if successful, or @c false if an error occurred.
   */
  static bool create(Context& context);
private:
  /*! @internal
   */
  struct IndexBufferSlot
  {
    Ptr<IndexBuffer> indexBuffer;
    unsigned int available;
  };
  /*! @internal
   */
  struct VertexBufferSlot
  {
    Ptr<VertexBuffer> vertexBuffer;
    unsigned int available;
  };
  Renderer(Context& context);
  bool init(void);
  void onContextFinish(void);
  void updateScissorArea(void);
  void updateViewportArea(void);
  typedef std::list<IndexBufferSlot> IndexBufferList;
  typedef std::list<VertexBufferSlot> VertexBufferList;
  Context& context;
  RectClipStack scissorStack;
  Rect viewportArea;
  Mat4 projection;
  MatrixStack4 modelview;
  IndexBufferList indexBufferPool;
  VertexBufferList vertexBufferPool;
  Canvas* currentCanvas;
  Canvas* screenCanvas;
  Ref<Program> currentProgram;
  PrimitiveRange currentRange;
  Ref<Texture> defaultTexture;
  Ref<Program> defaultProgram;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLRENDER_H*/
///////////////////////////////////////////////////////////////////////

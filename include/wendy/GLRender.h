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
#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Light;
class Camera;

///////////////////////////////////////////////////////////////////////

/*! @defgroup renderer 3D rendering pipeline
 *
 *  These classes make up the core 3D renderering pipeline, providing such
 *  services as state management, render operation sorting and per-operation
 *  and per-queue multi-pass operations.
 *
 *  It also provides some basic high-level rendering primitives such as
 *  multi-style geometry meshes, a simple terrain renderer and an extensible
 *  particle system.
 */

///////////////////////////////////////////////////////////////////////

/*! @brief Cull mode enumeration.
 *  @ingroup renderer
 */
enum CullMode
{
  /*! Do not cull any geometry.
   */
  CULL_NONE,
  /*! Cull front-facing geometry (i.e. render back-facing geometry).
   */
  CULL_FRONT = GL_FRONT,
  /*! Cull back-facing geometry (i.e. render front-facing geometry).
   */
  CULL_BACK = GL_BACK,
  /*! Cull all cullable geometry (i.e. front and back faces).
   */
  CULL_BOTH = GL_FRONT_AND_BACK,
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render pass state object.
 *  @ingroup renderer
 *
 *  This class and its associated classes encapsulates most of the OpenGL
 *  rendering state, notable exceptions being the matrix stacks and object
 *  parameters such as those set by @c glTexParameter, as well as scene-level
 *  state such as fog.
 *
 *  @remarks If you mix Wendy rendering code with other OpenGL code that
 *  affects rendering state, you will need to call RenderPass::invalidateCache
 *  as appropriate to notify the Wendy rendering pipeline that its state cache
 *  is dirty.
 *
 *  @remarks Unless you're writing your own custom renderer, you will probably
 *  want to use this class and the RenderStyle class, and not use the
 *  TextureLayer and TextureStack classes independently.
 *
 *  @remarks Yes, it's big.
 */
class RenderPass : public TextureStack
{
public:
  /*! Constructor.
   */
  RenderPass(const String& groupName = "");
  /*! Applies the settings in this render pass to OpenGL and stores them in the
   *  internal cache.
   *
   *  @remarks If the cache is dirty, all relevant OpenGL states will be forced
   *  to known values and the cache will then be considered clean.
   */
  void apply(void) const;
  /*! @return @c true if this render pass has been changed since the last call
   *  to RenderPass::apply, otherwise @c false.
   */
  bool isDirty(void) const;
  /*! @return @c true if this render pass uses any form of culling, otherwise
   *  @c false.
   */
  bool isCulling(void) const;
  /*! @return @c true if this render pass uses any form of blending with the
   *  framebuffer, otherwise @c false.
   */
  bool isBlending(void) const;
  /*! @return @c true if this render pass uses depth buffer testing, otherwise
   *  @c false.
   */
  bool isDepthTesting(void) const;
  /*! @return @c true if this render pass writes to the depth buffer, otherwise
   *  @c false.
   */
  bool isDepthWriting(void) const;
  /*! @return @c true if this render pass uses stencil buffer testing,
   *  otherwise @c false.
   */
  bool isStencilTesting(void) const;
  /*! @return @c true if this render pass writes to the color buffer, otherwise
   *  @c false.
   */
  bool isColorWriting(void) const;
  /*! @return @c true if this render pass is affected by lighting, otherwise @c
   *  false.
   */
  bool isLit(void) const;
  /*! @return The width of lines, in percent of the height of the current
   * render target.
   */
  float getLineWidth(void) const;
  /*! @return The culling mode of this render pass.
   */
  CullMode getCullMode(void) const;
  /*! @return The polygon rendering mode of this render pass.
   *
   *  @remarks The value may be @c GL_POINT, @c GL_LINE or @c GL_FILL.
   *  @remarks This corresponds to the @c glPolygonMode command.
   */
  GLenum getPolygonMode(void) const;
  GLenum getSrcFactor(void) const;
  GLenum getDstFactor(void) const;
  GLenum getDepthFunction(void) const;
  GLenum getAlphaFunction(void) const;
  GLenum getStencilFunction(void) const;
  GLenum getStencilFailOperation(void) const;
  GLenum getDepthFailOperation(void) const;
  GLenum getDepthPassOperation(void) const;
  unsigned int getStencilReference(void) const;
  unsigned int getStencilMask(void) const;
  float getShininess(void) const;
  const ColorRGBA& getDefaultColor(void) const;
  const ColorRGBA& getAmbientColor(void) const;
  const ColorRGBA& getDiffuseColor(void) const;
  const ColorRGBA& getSpecularColor(void) const;
  /*! @return The name of the GLSL program used by this render pass, or the
   *  empty string if it doesn't use any GLSL program.
   */
  const String& getShaderProgramName(void) const;
  const String& getGroupName(void) const;
  /*! Sets whether this render pass is affected by lighting.
   *  @param enable Set to @c true to enable lighting, or @c false to disable
   *  it.
   */
  void setLit(bool enable);
  /*! Sets whether this render pass uses depth buffer testing.
   *  @param enable Set to @c true to enable depth buffer testing, or @c false
   *  to disable it.
   */
  void setDepthTesting(bool enable);
  /*! Sets whether this render pass writes to the depth buffer.
   *  @param enable Set to @c true to enable depth buffer writing, or @c false
   *  to disable it.
   */
  void setDepthWriting(bool enable);
  /*! Sets whether this render pass uses stencil buffer testing.
   *  @param enable Set to @c true to enable stencil buffer testing, or @c
   *  false to disable it.
   */
  void setStencilTesting(bool enable);
  /*! Sets the depth buffer testing function for this render pass.
   *  @param function The desired depth testing function.
   */
  void setDepthFunction(GLenum function);
  void setAlphaFunction(GLenum function);
  void setStencilFunction(GLenum function);
  void setStencilValues(unsigned int reference, unsigned int mask);
  void setStencilOperations(GLenum stencilFailed,
                            GLenum depthFailed,
                            GLenum depthPassed);
  /*! Sets whether writing to the color buffer is enabled.
   *  @param enabled @c true to enable writing to the color buffer, or @c false
   *  to disable it.
   */
  void setColorWriting(bool enabled);
  /*! Sets the specified line width.
   *  @param width The desired line width, in percent of the height of the
   *  current render target.
   */
  void setLineWidth(float width);
  void setCullMode(CullMode mode);
  /*! Sets the polygon rendering mode of this render pass.
   *  @param mode The desired polygon rendering mode.
   *  @arg GL_POINT
   *  @arg GL_LINE
   *  @arg GL_FILL
   *
   *  @remarks This corresponds to the @c glPolygonMode command.
   */
  void setPolygonMode(GLenum mode);
  void setBlendFactors(GLenum src, GLenum dst);
  void setShininess(float newValue);
  void setDefaultColor(const ColorRGBA& color);
  void setAmbientColor(const ColorRGBA& color);
  void setDiffuseColor(const ColorRGBA& color);
  void setSpecularColor(const ColorRGBA& color);
  /*! Sets the name of the GLSL program used by this render pass.
   *  @param newName The name of the desired vertex program, or the empty
   *  string to disable the use of GLSL programs.
   */
  void setShaderProgramName(const String& newName);
  /*! Resets all values in this render pass to their defaults.
   */
  void setDefaults(void);
  /*! Flags the cache as dirty.  All states will be forced to known values
   *  on the next call to RenderPass::apply.
   */
  static void invalidateCache(void);
private:
  class Data
  {
  public:
    Data(void);
    void setDefaults(void);
    mutable bool dirty;
    bool lighting;
    bool depthTesting;
    bool depthWriting;
    bool stencilTesting;
    bool colorWriting;
    float lineWidth;
    CullMode cullMode;
    GLenum polygonMode;
    GLenum srcFactor;
    GLenum dstFactor;
    GLenum depthFunction;
    GLenum alphaFunction;
    GLenum stencilFunction;
    unsigned int stencilRef;
    unsigned int stencilMask;
    GLenum stencilFailed;
    GLenum depthFailed;
    GLenum depthPassed;
    float shininess;
    ColorRGBA defaultColor;
    ColorRGBA ambientColor;
    ColorRGBA diffuseColor;
    ColorRGBA specularColor;
    String shaderProgramName;
  };
  void force(void) const;
  void setBooleanState(GLenum state, bool value) const;
  Data data;
  String groupName;
  static Data cache;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Sortable render pass stack object.
 *  @ingroup renderer
 *
 *  This is the highest-level render style object, encapsulating multiple
 *  render passes, each of which controls virtually all relevant state for a
 *  single render pass.
 *
 *  Most of the time, this is the class you want to use to manage your render
 *  states.
 */
class RenderStyle : public Decorated, public Resource<RenderStyle>
{
public:
  /*! Constructor.
   *  @param name The desired name of this render style, or the empty string to
   *  request a generated name.
   */
  RenderStyle(const String& name = "");
  /*! Creates a new render pass in this render style.
   *  @param[in] groupName The name of the group the render pass will belong
   *  to, or the empty string to place it in the default group.
   *  @return The newly created render pass.
   *  @remarks The passes are rendered in creation order.
   */
  RenderPass& createPass(const String& groupName = "");
  /*! Destroys all render passes in this render style.
   */
  void destroyPasses(void);
  /*! Applies the render pass with the specified index.
   *  @param index The index of the desired render pass.
   */
  void applyPass(unsigned int index) const;
  /*! Comparison operator to enable sorting.
   *  @param other The object to compare to.
   */
  bool operator < (const RenderStyle& other) const;
  /*! @return @c true if this render style uses framebuffer blending, otherwise
   *  @c false.
   */
  bool isBlending(void) const;
  /*! @param index The index of the desired render pass.
   *  @return The render pass at the specified index.
   */
  RenderPass& getPass(unsigned int index);
  /*! @param index The index of the desired render pass.
   *  @return The render pass at the specified index.
   */
  const RenderPass& getPass(unsigned int index) const;
  /*! @return The number of render passes in this render style.
   */
  unsigned int getPassCount(void) const;
private:
  typedef std::list<RenderPass> PassList;
  PassList passes;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation in the 3D pipeline.
 *  @ingroup renderer
 *
 *  This represents a single render operation, including a style and
 *  local-to-world transformation.
 *
 *  @remarks Note that this class does not include any references to a camera.
 *  The camera transformation is handled by the Camera class, and the
 *  RenderQueue to which your RenderOperation belongs contains a reference to
 *  the correct Camera.
 */
class RenderOperation
{
public:
  /*! Constructor.
   */
  RenderOperation(void);
  /*! Comparison operator to enable sorting.
   *  @param other The object to compare to.
   */
  bool operator < (const RenderOperation& other) const;
  /*! The vertex buffer that contains the geometry to render.
   */
  const VertexBuffer* vertexBuffer;
  /*! The index buffer that references the vertex buffer, or @c NULL
   *  if no index buffer is to be used.
   */
  const IndexBuffer* indexBuffer;
  /*! The render style to use.
   */
  const RenderStyle* style;
  /*! If an index buffer is set, this is the first element of the range of
   *  indices to be used for rendering.  Otherwise, it is the first vertex in
   *  the vertex buffer to be used for rendering.
   */
  unsigned int start;
  /*! If an index buffer is set, this is the number of elements in the index
   *  buffer to be used for rendering.  Otherwise, it is the number of vertices
   *  in the vertex buffer to be used for rendering.
   */
  unsigned int count;
  /*! The local-to-world transformation.
   */
  Matrix4 transform;
  /*! The geometry rendering mode to use.
   */
  GLenum renderMode;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation queue for the 3D pipeline.
 *  @ingroup renderer
 *
 *  This class is a container for all data needed to render a scene, most
 *  commonly gathered from a scene graph or similar structure, and can
 *  therefore be seen as a higher-level rendering operation.
 */
class RenderQueue
{
public:
  typedef std::vector<Light*> LightList;
  typedef std::vector<RenderOperation> OperationList;
  RenderQueue(const Camera& camera);
  void addLight(Light& light);
  void removeLights(void);
  RenderOperation& createOperation(void);
  void destroyOperations(void);
  void renderOperations(void) const;
  const Camera& getCamera(void) const;
  const LightList& getLights(void) const;
  const OperationList& getOperations(void) const;
private:
  typedef std::vector<const RenderOperation*> SortedOperationList;
  void sortOperations(void) const;
  LightList lights;
  OperationList operations;
  const Camera& camera;
  mutable SortedOperationList sortedOperations;
  mutable bool sorted;
};

///////////////////////////////////////////////////////////////////////

class RenderStage
{
public:
  virtual ~RenderStage(void);
  virtual void prepare(const RenderQueue& queue);
  virtual void render(const RenderQueue& queue);
};

///////////////////////////////////////////////////////////////////////

class RenderStageStack
{
public:
  void addStage(RenderStage& stage);
  void destroyStages(void);
  void prepare(const RenderQueue& queue);
  void render(const RenderQueue& queue);
private:
  typedef std::vector<RenderStage*> StageList;
  StageList stages;
};

///////////////////////////////////////////////////////////////////////

class Renderable
{
public:
  virtual ~Renderable(void);
  virtual void enqueue(RenderQueue& queue,
                       const Transform3& transform) const = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @brief The renderer singleton.
 *  @ingroup renderer
 *
 *  This is the central renderer class.
 */
class Renderer : public Trackable, public Singleton<Renderer>
{
public:
  void begin2D(const Vector2& resolution = Vector2(1.f, 1.f)) const;
  void begin3D(float FOV = 90.f,
               float aspect = 0.f,
	       float nearZ = 0.01f,
	       float farZ = 1000.f) const;
  void end(void) const;
  void drawPoint(const Vector2& point);
  void drawLine(const Segment2& segment);
  void drawBezier(const BezierCurve2& spline);
  void drawRectangle(const Rectangle& rectangle);
  void fillRectangle(const Rectangle& rectangle);
  /*! Allocates a range of indices of the specified type.
   */
  bool allocateIndices(IndexBufferRange& range,
		       unsigned int count,
                       IndexBuffer::Type type = IndexBuffer::UBYTE);
  /*! Allocates a range of vertices of the specified format.
   */
  bool allocateVertices(VertexBufferRange& range,
			unsigned int count,
                        const VertexFormat& format);
  const ColorRGBA& getColor(void) const;
  void setColor(const ColorRGBA& newColor);
  const RenderStyle& getDefaultStyle(void) const;
  /*! Creates the renderer singleton.
   */
  static bool create(void);
private:
  class IndexBufferSlot;
  class VertexBufferSlot;
  Renderer(void);
  bool init(void);
  void onContextFinish(void);
  static void onContextDestroy(void);
  typedef std::list<IndexBufferSlot> IndexBufferList;
  typedef std::list<VertexBufferSlot> VertexBufferList;
  Ptr<RenderStyle> defaultStyle;
  Ptr<Texture> defaultTexture;
  IndexBufferList indexBuffers;
  VertexBufferList vertexBuffers;
  RenderPass pass;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class Renderer::IndexBufferSlot
{
public:
  Ptr<IndexBuffer> indexBuffer;
  unsigned int available;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class Renderer::VertexBufferSlot
{
public:
  Ptr<VertexBuffer> vertexBuffer;
  unsigned int available;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLRENDER_H*/
///////////////////////////////////////////////////////////////////////

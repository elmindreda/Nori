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

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Vector.h>
#include <wendy/Matrix.h>
#include <wendy/Plane.h>
#include <wendy/Vertex.h>
#include <wendy/Timer.h>

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

/*! @brief %Render statistics.
 *  @ingroup opengl
 */
class Stats
{
public:
  class Frame
  {
  public:
    Frame(void);
    unsigned int passCount;
    unsigned int vertexCount;
    unsigned int pointCount;
    unsigned int lineCount;
    unsigned int triangleCount;
    Time duration;
  };
  typedef std::deque<Frame> FrameQueue;
  Stats(void);
  void addFrame(void);
  void addPasses(unsigned int count);
  void addPrimitives(PrimitiveType type, unsigned int count);
  float getFrameRate(void) const;
  unsigned int getFrameCount(void) const;
  const Frame& getFrame(void) const;
private:
  unsigned int frameCount;
  float frameRate;
  FrameQueue frames;
  Timer timer;
};

///////////////////////////////////////////////////////////////////////

/*! @brief The renderer singleton.
 *  @ingroup opengl
 *
 *  This is the central renderer class for the OpenGL layer.
 */
class Renderer : public Trackable, public Singleton<Renderer>
{
public:
  typedef std::vector<Plane> PlaneList;
  /*! Renders the current primitive range to the current canvas, using the
   *  current shader program and transforms.
   *  @pre A shader program must be set before calling this method.
   */
  void render(const PrimitiveRange& range);
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
                       IndexBuffer::Type type);
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
  SignalProxy1<void, Uniform&> reserveUniform(const String& name, Uniform::Type type);
  SignalProxy1<void, Sampler&> reserveSampler(const String& name, Sampler::Type type);
  /*! @return @c true if the specified uniform name is reserved,
   *  otherwise @c false.
   */
  bool isReservedUniform(const String& name) const;
  /*! @return @c true if the specified sampler name is reserved,
   *  otherwise @c false.
   */
  bool isReservedSampler(const String& name) const;
  /*! @return The context this renderer was created for.
   */
  Context& getContext(void) const;
  Program* getCurrentProgram(void) const;
  const PlaneList& getClipPlanes(void) const;
  const Mat4& getModelMatrix(void) const;
  const Mat4& getViewMatrix(void) const;
  const Mat4& getProjectionMatrix(void) const;
  void setModelMatrix(const Mat4& newMatrix);
  void setViewMatrix(const Mat4& newMatrix);
  /*! Sets the projection matrix.
   *  @param[in] newMatrix The desired projection matrix.
   */
  void setProjectionMatrix(const Mat4& newMatrix);
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
   *  the dimensions of the current viewport.
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
  bool setClipPlanes(const PlaneList& newPlanes);
  Stats* getStats(void) const;
  void setStats(Stats* newStats);
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
    Ref<IndexBuffer> indexBuffer;
    unsigned int available;
  };
  /*! @internal
   */
  struct VertexBufferSlot
  {
    Ref<VertexBuffer> vertexBuffer;
    unsigned int available;
  };
  /*! @internal
   */
  struct ReservedUniform
  {
    String name;
    Uniform::Type type;
    Signal1<void, Uniform&> signal;
  };
  /*! @internal
   */
  struct ReservedSampler
  {
    String name;
    Sampler::Type type;
    Signal1<void, Sampler&> signal;
  };
  Renderer(Context& context);
  bool init(void);
  void onContextFinish(void);
  typedef std::list<IndexBufferSlot> IndexBufferList;
  typedef std::list<VertexBufferSlot> VertexBufferList;
  typedef std::list<ReservedUniform> UniformList;
  typedef std::list<ReservedSampler> SamplerList;
  Context& context;
  Mat4 modelMatrix;
  Mat4 viewMatrix;
  Mat4 projectionMatrix;
  IndexBufferList indexBufferPool;
  VertexBufferList vertexBufferPool;
  UniformList reservedUniforms;
  SamplerList reservedSamplers;
  Ref<Program> currentProgram;
  PlaneList planes;
  Stats* stats;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLRENDER_H*/
///////////////////////////////////////////////////////////////////////

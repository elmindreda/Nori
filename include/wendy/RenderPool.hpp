///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_RENDERPOOL_HPP
#define WENDY_RENDERPOOL_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.hpp>
#include <wendy/Signal.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex pool.
 *  @ingroup renderer
 */
class VertexPool : public Trackable, public RefObject
{
public:
  /*! Allocates a range of temporary vertices of the specified format.
   *  @param[in] count The number of vertices to allocate.
   *  @param[in] format The format of vertices to allocate.
   *  @return @c The newly allocated vertex range.
   *
   *  @remarks The allocated vertex range is only valid until the end of the
   *  current frame.
   */
  VertexRange allocate(uint count, const VertexFormat& format);
  /*! @return The OpenGL context used by this pool.
   */
  RenderContext& context() const { return m_context; }
  /*! Creates a vertex pool.
   *  @param[in] context The OpenGL context to be used.
   *  @param[in] granularity The desired allocation granularity.
   *  @return The newly created vertex pool.
   */
  static Ref<VertexPool> create(RenderContext& context, size_t granularity = 1024);
private:
  VertexPool(RenderContext& context);
  VertexPool(const VertexPool&) = delete;
  bool init(size_t granularity);
  VertexPool& operator = (const VertexPool&) = delete;
  /*! @internal
   */
  struct Slot
  {
    Ref<VertexBuffer> buffer;
    uint available;
  };
  void onFrame();
  RenderContext& m_context;
  size_t m_granularity;
  std::vector<Slot> m_slots;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERPOOL_HPP*/
///////////////////////////////////////////////////////////////////////

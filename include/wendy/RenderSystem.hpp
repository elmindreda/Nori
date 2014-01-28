///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_RENDERSYSTEM_HPP
#define WENDY_RENDERSYSTEM_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class RenderSystem : public RefObject
{
public:
  enum Type
  {
    FORWARD
  };
  ResourceCache& cache() const;
  RenderContext& context() const;
  VertexPool& vertexPool() const;
  Type type() const;
protected:
  RenderSystem(VertexPool& pool, Type type);
private:
  RenderSystem(const RenderSystem&) = delete;
  RenderSystem& operator = (const RenderSystem&) = delete;
  Ref<VertexPool> m_pool;
  Type m_type;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSYSTEM_HPP*/
///////////////////////////////////////////////////////////////////////

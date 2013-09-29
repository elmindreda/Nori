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

#include <wendy/GLTexture.hpp>
#include <wendy/GLBuffer.hpp>
#include <wendy/GLProgram.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class System : public RefObject
{
public:
  enum Type
  {
    FORWARD
  };
  ResourceCache& cache() const;
  GL::Context& context() const;
  VertexPool& vertexPool() const;
  Type type() const;
protected:
  System(VertexPool& pool, Type type);
private:
  System(const System&) = delete;
  System& operator = (const System&) = delete;
  Ref<VertexPool> m_pool;
  Type m_type;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSYSTEM_HPP*/
///////////////////////////////////////////////////////////////////////

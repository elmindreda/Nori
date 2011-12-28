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
#ifndef WENDY_RENDERSYSTEM_H
#define WENDY_RENDERSYSTEM_H
///////////////////////////////////////////////////////////////////////

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLState.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

class System : public RefObject
{
public:
  enum Type
  {
    SIMPLE,
    FORWARD,
    DEFERRED
  };
  ResourceCache& getCache() const;
  GL::Context& getContext() const;
  GeometryPool& getGeometryPool() const;
  Type getType() const;
protected:
  System(GeometryPool& pool, Type type);
private:
  System(const System& source);
  System& operator = (const System& source);
  Ref<GeometryPool> pool;
  Type type;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSYSTEM_H*/
///////////////////////////////////////////////////////////////////////

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

#include <wendy/Config.h>

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <wendy/RenderPool.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

bool GeometryPool::allocateIndices(GL::IndexRange& range,
                                   unsigned int count,
                                   GL::IndexBuffer::Type type)
{
  if (!count)
  {
    range = GL::IndexRange();
    return true;
  }

  IndexBufferSlot* slot = NULL;

  for (auto i = indexBufferPool.begin();  i != indexBufferPool.end();  i++)
  {
    if (i->indexBuffer->getType() == type && i->available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    indexBufferPool.push_back(IndexBufferSlot());
    slot = &(indexBufferPool.back());

    const unsigned int actualCount = granularity * ((count + granularity - 1) / granularity);

    slot->indexBuffer = GL::IndexBuffer::create(context,
                                                actualCount,
                                                type,
                                                GL::IndexBuffer::DYNAMIC);
    if (!slot->indexBuffer)
    {
      indexBufferPool.pop_back();
      return false;
    }

    log("Allocated index pool of size %u", actualCount);

    slot->available = slot->indexBuffer->getCount();
  }

  range = GL::IndexRange(*(slot->indexBuffer),
                         slot->indexBuffer->getCount() - slot->available,
                         count);

  slot->available -= count;
  return true;
}

bool GeometryPool::allocateVertices(GL::VertexRange& range,
                                    unsigned int count,
                                    const VertexFormat& format)
{
  if (!count)
  {
    range = GL::VertexRange();
    return true;
  }

  VertexBufferSlot* slot = NULL;

  for (auto i = vertexBufferPool.begin();  i != vertexBufferPool.end();  i++)
  {
    if (i->vertexBuffer->getFormat() == format && i->available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    vertexBufferPool.push_back(VertexBufferSlot());
    slot = &(vertexBufferPool.back());

    const unsigned int actualCount = granularity * ((count + granularity - 1) / granularity);

    slot->vertexBuffer = GL::VertexBuffer::create(context,
                                                  actualCount,
                                                  format,
                                                  GL::VertexBuffer::DYNAMIC);
    if (!slot->vertexBuffer)
    {
      vertexBufferPool.pop_back();
      return false;
    }

    log("Allocated vertex pool of size %u format \'%s\'",
        actualCount,
        format.asString().c_str());

    slot->available = slot->vertexBuffer->getCount();
  }

  range = GL::VertexRange(*(slot->vertexBuffer),
                          slot->vertexBuffer->getCount() - slot->available,
                          count);

  slot->available -= count;
  return true;
}

GL::Context& GeometryPool::getContext() const
{
  return context;
}

Ref<GeometryPool> GeometryPool::create(GL::Context& context, size_t granularity)
{
  Ref<GeometryPool> pool(new GeometryPool(context));
  if (!pool->init(granularity))
    return NULL;

  return pool;
}

GeometryPool::GeometryPool(GL::Context& initContext):
  context(initContext),
  granularity(0)
{
  context.getFinishSignal().connect(*this, &GeometryPool::onContextFinish);
}

bool GeometryPool::init(size_t initGranularity)
{
  granularity = initGranularity;
  return true;
}

void GeometryPool::onContextFinish()
{
  for (auto i = indexBufferPool.begin();  i != indexBufferPool.end();  i++)
    i->available = i->indexBuffer->getCount();

  for (auto i = vertexBufferPool.begin();  i != vertexBufferPool.end();  i++)
    i->available = i->vertexBuffer->getCount();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

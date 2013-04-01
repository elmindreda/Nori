///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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

#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <wendy/RenderPool.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

bool VertexPool::allocateVertices(GL::VertexRange& range,
                                  uint count,
                                  const VertexFormat& format)
{
  if (!count)
  {
    range = GL::VertexRange();
    return true;
  }

  Slot* slot = NULL;

  for (auto s = slots.begin();  s != slots.end();  s++)
  {
    if (s->buffer->getFormat() == format && s->available >= count)
    {
      slot = &(*s);
      break;
    }
  }

  if (!slot)
  {
    slots.push_back(Slot());
    slot = &(slots.back());

    const uint actualCount = granularity * ((count + granularity - 1) / granularity);

    slot->buffer = GL::VertexBuffer::create(context,
                                            actualCount,
                                            format,
                                            GL::USAGE_DYNAMIC);
    if (!slot->buffer)
    {
      slots.pop_back();
      return false;
    }

    log("Allocated vertex pool of size %u format \'%s\'",
        actualCount,
        format.asString().c_str());

    slot->available = slot->buffer->getCount();
  }

  range = GL::VertexRange(*(slot->buffer),
                          slot->buffer->getCount() - slot->available,
                          count);

  slot->available -= count;
  return true;
}

GL::Context& VertexPool::getContext() const
{
  return context;
}

Ref<VertexPool> VertexPool::create(GL::Context& context, size_t granularity)
{
  Ref<VertexPool> pool(new VertexPool(context));
  if (!pool->init(granularity))
    return NULL;

  return pool;
}

VertexPool::VertexPool(GL::Context& initContext):
  context(initContext),
  granularity(0)
{
  context.getWindow().getFrameSignal().connect(*this, &VertexPool::onFrame);
}

bool VertexPool::init(size_t initGranularity)
{
  granularity = initGranularity;
  return true;
}

void VertexPool::onFrame()
{
  for (auto s = slots.begin();  s != slots.end();  s++)
    s->available = s->buffer->getCount();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

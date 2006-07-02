///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Log.h>
#include <moira/Vector.h>
#include <moira/Color.h>
#include <moira/Image.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLVertex.h>
#include <wendy/GLVertexBuffer.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

VertexBuffer::~VertexBuffer(void)
{
  if (locked)
    Log::writeWarning("Vertex buffer destroyed while locked");

  if (current == this)
    invalidateCurrent();

  if (bufferID != 0)
    glDeleteBuffersARB(1, &bufferID);
}

void VertexBuffer::apply(void) const
{
  const Byte* base = NULL;

  if (GLEW_ARB_vertex_buffer_object)
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);
  else
    base = data;

  const VertexComponent* component;

  component = format.findComponent(VertexComponent::VERTEX);
  if (component)
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(component->getElementCount(),
                    component->getType(),
		    (GLsizei) format.getSize(),
		    base + component->getOffset());
  }
  else
    glDisableClientState(GL_VERTEX_ARRAY);

  component = format.findComponent(VertexComponent::TEXCOORD);
  if (component)
  {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(component->getElementCount(),
                      component->getType(),
		      (GLsizei) format.getSize(),
		      base + component->getOffset());
  }
  else
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  component = format.findComponent(VertexComponent::COLOR);
  if (component)
  {
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(component->getElementCount(),
                   component->getType(),
		   (GLsizei) format.getSize(),
		   base + component->getOffset());
  }
  else
    glDisableClientState(GL_COLOR_ARRAY);

  component = format.findComponent(VertexComponent::NORMAL);
  if (component)
  {
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(component->getType(),
		    (GLsizei) format.getSize(),
		    base + component->getOffset());
  }
  else
    glDisableClientState(GL_NORMAL_ARRAY);

  current = const_cast<VertexBuffer*>(this);
}

void VertexBuffer::render(unsigned int mode,
                          unsigned int start,
                          unsigned int count) const
{
  if (getCurrent() != this)
    apply();

  if (!count)
    count = getCount();

  glDrawArrays(mode, start, count);
}

void* VertexBuffer::lock(void)
{
  if (locked)
  {
    Log::writeError("Vertex buffer already locked");
    return NULL;
  }

  void* mapping = NULL;

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);

    mapping = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);

    glPopClientAttrib();

    if (mapping == NULL)
    {
      Log::writeError("Unable to map vertex buffer object: %s", gluErrorString(glGetError()));
      return NULL;
    }
  }
  else
    mapping = data;

  locked = true;
  return mapping;
}

void VertexBuffer::unlock(void)
{
  if (!locked)
    return;

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);

    if (!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
      Log::writeWarning("Data for vertex buffer object was corrupted");

    glPopClientAttrib();
  }

  locked = false;
}

GLuint VertexBuffer::getGLID(void) const
{
  return bufferID;
}

VertexBuffer::Usage VertexBuffer::getUsage(void) const
{
  return usage;
}

const VertexFormat& VertexBuffer::getFormat(void) const
{
  return format;
}

unsigned int VertexBuffer::getCount(void) const
{
  return count;
}

VertexBuffer* VertexBuffer::createInstance(const std::string& name,
					   unsigned int count,
                                           const VertexFormat& format,
					   Usage usage)
{
  Ptr<VertexBuffer> buffer = new VertexBuffer(name);
  if (!buffer->init(format, count, usage))
    return NULL;

  return buffer.detachObject();
}

void VertexBuffer::invalidateCurrent(void)
{
  current = NULL;
}

VertexBuffer* VertexBuffer::getCurrent(void)
{
  return current;
}

VertexBuffer::VertexBuffer(const std::string& name):
  Managed<VertexBuffer>(name),
  locked(false),
  count(0),
  usage(STATIC),
  bufferID(0)
{
}

bool VertexBuffer::init(const VertexFormat& initFormat,
			unsigned int initCount,
			Usage initUsage)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create vertex buffer without OpenGL context");
    return false;
  }

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

    glGenBuffersARB(1, &bufferID);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB,
		    initCount * initFormat.getSize(),
		    NULL,
		    initUsage);

    glPopClientAttrib();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      Log::writeWarning("Error during vertex buffer object creation: %s", gluErrorString(error));
      return false;
    }
  }
  else
    data.resize(initCount * initFormat.getSize());

  format = initFormat;
  usage = initUsage;
  count = initCount;

  return true;
}

VertexBuffer* VertexBuffer::current = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

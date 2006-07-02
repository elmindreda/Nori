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
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Log.h>
#include <moira/Vector.h>
#include <moira/Color.h>
#include <moira/Image.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLIndexBuffer.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

namespace
{
  
size_t getTypeSize(IndexBuffer::Type type)
{
  switch (type)
  {
    case IndexBuffer::UINT:
      return sizeof(GLuint);
    case IndexBuffer::USHORT:
      return sizeof(GLushort);
    case IndexBuffer::BYTE:
      return sizeof(GLubyte);
    default:
      return 0;
  }
}

}

///////////////////////////////////////////////////////////////////////

IndexBuffer::~IndexBuffer(void)
{
  if (locked)
    Log::writeWarning("Vertex buffer destroyed while locked");

  if (current == this)
    invalidateCurrent();

  if (bufferID)
    glDeleteBuffersARB(1, &bufferID);
}

void IndexBuffer::apply(void) const
{
  if (GLEW_ARB_vertex_buffer_object)
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);

  current = const_cast<IndexBuffer*>(this);
}

void IndexBuffer::render(unsigned int mode, unsigned int count) const
{
  if (getCurrent() != this)
    apply();

  if (!count)
    count = getCount();

  const Byte* base = NULL;

  if (!GLEW_ARB_vertex_buffer_object)
    base = data;

  glDrawElements(mode, count, type, base);
}

void* IndexBuffer::lock(void)
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
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);

    mapping = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);

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

void IndexBuffer::unlock(void)
{
  if (!locked)
    return;

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);

    if (!glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB))
      Log::writeWarning("Data for vertex buffer object was corrupted");

    glPopClientAttrib();
  }

  locked = false;
}

GLuint IndexBuffer::getGLID(void) const
{
  return bufferID;
}

IndexBuffer::Type IndexBuffer::getType(void) const
{
  return type;
}

IndexBuffer::Usage IndexBuffer::getUsage(void) const
{
  return usage;
}

unsigned int IndexBuffer::getCount(void) const
{
  return count;
}

IndexBuffer* IndexBuffer::createInstance(const std::string& name,
					 unsigned int count,
					 Type type,
					 Usage usage)
{
  Ptr<IndexBuffer> buffer = new IndexBuffer(name);
  if (!buffer->init(count, type, usage))
    return NULL;

  return buffer.detachObject();
}

void IndexBuffer::invalidateCurrent(void)
{
  current = NULL;
}

IndexBuffer* IndexBuffer::getCurrent(void)
{
  return current;
}

IndexBuffer::IndexBuffer(const std::string& name):
  Managed<IndexBuffer>(name),
  locked(false),
  type(UINT),
  usage(STATIC),
  count(0),
  bufferID(0)
{
}

bool IndexBuffer::init(unsigned int initCount, Type initType, Usage initUsage)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create index buffer without OpenGL context");
    return false;
  }

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glGenBuffersARB(1, &bufferID);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
	            initCount * getTypeSize(initType),
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
    data.resize(initCount * getTypeSize(initType));

  type = initType;
  usage = initUsage;
  count = initCount;
  return true;
}

IndexBuffer* IndexBuffer::current = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

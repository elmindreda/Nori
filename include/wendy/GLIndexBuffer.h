///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@home.se>
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
#ifndef WEGLINDEXBUFFER_H
#define WEGLINDEXBUFFER_H
///////////////////////////////////////////////////////////////////////

#include <string>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class IndexBuffer : public Managed<IndexBuffer>
{
public:
  enum Type
  {
    /*! Indices are of type unsigned int.
     */
    UINT = GL_UNSIGNED_INT,
    /*! Indices are of type unsigned short.
     */
    USHORT = GL_UNSIGNED_SHORT,
    /*! Indices are of type unsigned char.
     */
    BYTE = GL_UNSIGNED_BYTE,
  };
  enum Usage
  {
    /*! Data will be specified once and used many times.
     */
    STATIC = GL_STATIC_DRAW_ARB,
    /*! Data will be repeatedly respecified.
     */
    DYNAMIC = GL_DYNAMIC_DRAW_ARB,
  };
  ~IndexBuffer(void);
  void apply(void) const;
  void render(unsigned int mode, unsigned int count = 0) const;
  void* lock(void);
  void unlock(void);
  GLuint getGLID(void) const;
  Type getType(void) const;
  Usage getUsage(void) const;
  unsigned int getCount(void) const;
  static IndexBuffer* createInstance(const std::string& name,
                                     unsigned int count,
				     Type type = UINT,
				     Usage usage = STATIC);
  static void invalidateCurrent(void);
  static IndexBuffer* getCurrent(void);
private:
  IndexBuffer(const std::string& name);
  bool init(unsigned int initCount, Type initType, Usage initUsage);
  bool locked;
  Type type;
  Usage usage;
  unsigned int count;
  GLuint bufferID;
  Block data;
  static IndexBuffer* current;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WEGLINDEXBUFFER_H*/
///////////////////////////////////////////////////////////////////////

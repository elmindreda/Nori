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
#ifndef WENDY_VERTEXBUFFER_H
#define WENDY_VERTEXBUFFER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class VertexBuffer : public Managed<VertexBuffer>
{
public:
  enum Usage
  {
    /*! Data will be specified once and used many times.
     */
    STATIC = GL_STATIC_DRAW_ARB,
    /*! Data will be repeatedly respecified.
     */
    DYNAMIC = GL_DYNAMIC_DRAW_ARB,
  };
  ~VertexBuffer(void);
  void apply(void) const;
  void render(unsigned int mode,
              unsigned int start = 0,
	      unsigned int count = 0) const;
  void* lock(void);
  void unlock(void);
  GLuint getGLID(void) const;
  Usage getUsage(void) const;
  const VertexFormat& getFormat(void) const;
  unsigned int getCount(void) const;
  static VertexBuffer* createInstance(const std::string& name,
				      unsigned int count,
                                      const VertexFormat& format,
				      Usage usage = STATIC); 
  static void invalidateCurrent(void);
  static VertexBuffer* getCurrent(void);
private:
  VertexBuffer(const std::string& name);
  bool init(const VertexFormat& initFormat,
            unsigned int initCount,
	    Usage initUsage);
  bool locked;
  VertexFormat format;
  unsigned int count;
  Usage usage;
  GLuint bufferID;
  ByteBlock data;
  static VertexBuffer* current;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_VERTEXBUFFER_H*/
///////////////////////////////////////////////////////////////////////

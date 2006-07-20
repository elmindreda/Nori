///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLPROGRAM_H
#define WENDY_GLPROGRAM_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

class VertexProgram : public Managed<VertexProgram>
{
public:
  ~VertexProgram(void);
  GLuint getGLID(void) const;
  bool getParameter(unsigned int index, Vector4& value) const;
  bool setParameter(unsigned int index, const Vector4& newValue);
  static VertexProgram* createInstance(const Path& path,
                                       const String& name = "");
  static VertexProgram* createInstance(const String& text,
                                       const String& name = "");
private:
  VertexProgram(const String& name);
  bool init(const String& text);
  GLuint programID;
};

///////////////////////////////////////////////////////////////////////

class FragmentProgram : public Managed<FragmentProgram>
{
public:
  ~FragmentProgram(void);
  GLuint getGLID(void) const;
  bool getParameter(unsigned int index, Vector4& value) const;
  bool setParameter(unsigned int index, const Vector4& newValue);
  static FragmentProgram* createInstance(const Path& path,
                                         const String& name = "");
  static FragmentProgram* createInstance(const String& text,
                                         const String& name = "");
private:
  FragmentProgram(const String& name);
  bool init(const String& text);
  GLuint programID;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLPROGRAM_H*/
///////////////////////////////////////////////////////////////////////

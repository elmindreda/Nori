///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLHELPER_H
#define WENDY_GLHELPER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

WENDY_CHECKFORMAT(1, bool checkGL(const char* format, ...));

GLenum convertToGL(IndexBuffer::Type type);
GLenum convertToGL(VertexComponent::Type type);
GLenum convertToGL(PixelFormat::Type type);
GLenum convertToGL(const PixelFormat& format, bool sRGB);
GLenum convertToGL(PixelFormat::Type type);
GLenum convertToGL(PixelFormat::Semantic semantic);
GLenum convertToGL(TextureType type);

GLboolean getBoolean(GLenum token);
GLint getInteger(GLenum token);
GLfloat getFloat(GLenum token);

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLHELPER_H*/
///////////////////////////////////////////////////////////////////////

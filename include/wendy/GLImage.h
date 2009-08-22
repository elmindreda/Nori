///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLIMAGE_H
#define WENDY_GLIMAGE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Image : public RefObject
{
public:
  virtual ~Image(void);
  virtual unsigned int getWidth(void) const = 0;
  virtual unsigned int getHeight(void) const = 0;
  virtual const ImageFormat& getFormat(void) const = 0;
protected:
  virtual void attach(int attachment) = 0;
  virtual void detach(void) = 0;
};

///////////////////////////////////////////////////////////////////////

typedef Ref<Image> ImageRef;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLIMAGE_H*/
///////////////////////////////////////////////////////////////////////

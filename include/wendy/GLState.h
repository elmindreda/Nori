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
#ifndef WENDY_GLSTATE_H
#define WENDY_GLSTATE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class StencilState
{
public:
  void apply(void) const;
  bool isEnabled(void) const;
  GLenum getFunction(void) const;
  GLenum getStencilFailOperation(void) const;
  GLenum getDepthFailOperation(void) const;
  GLenum getDepthPassOperation(void) const;
  unsigned int getReference(void) const;
  unsigned int getWriteMask(void) const;
  void setEnabled(bool newState);
  void setFunction(GLenum newFunction);
  void setReference(unsigned int newReference);
  void setWriteMask(unsigned int newMask);
  void setOperations(GLenum stencilFailed,
                     GLenum depthFailed,
                     GLenum depthPassed);
  void setDefaults(void);
  static void invalidateCache(void);
private:
  class Data
  {
  public:
    Data(void);
    void setDefaults(void);
    mutable bool dirty;
    bool enabled;
    GLenum function;
    unsigned int reference;
    unsigned int writeMask;
    GLenum stencilFailed;
    GLenum depthFailed;
    GLenum depthPassed;
  };
  void force(void) const;
  Data data;
  static Data cache;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSTATE_H*/
///////////////////////////////////////////////////////////////////////

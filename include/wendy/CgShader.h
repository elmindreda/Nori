///////////////////////////////////////////////////////////////////////
// Wendy Cg library
// Copyright (c) 2008 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_CGSHADER_H
#define WENDY_CGSHADER_H
///////////////////////////////////////////////////////////////////////
#if WENDY_USE_CG
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace Cg
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief Cg shader program.
 */
class Shader : public Resource<Shader>, public RefObject<Shader>
{
public:
  /*! Destructor.
   */
  ~Shader(void);
  Domain getDomain(void) const;
  static Shader* createInstance(Domain domain, const String& text, const String& name = "");
private:
  Shader(Domain domain, const String& name);
  bool init(const String& text);
  Domain domain;
  CGprogram programID;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace Cg*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /* WENDY_USE_CG */
///////////////////////////////////////////////////////////////////////
#endif /*WENDY_CGSHADER_H*/
///////////////////////////////////////////////////////////////////////

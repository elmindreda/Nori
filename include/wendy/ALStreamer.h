///////////////////////////////////////////////////////////////////////
// Wendy OpenAL library
// Copyright (c) 2007 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_ALSTREAMER_H
#define WENDY_ALSTREAMER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup openal
 */
class Streamer : public Managed<Streamer>
{
public:
  ~Streamer(void);
  static Streamer* createInstance(const Path& path, const String& name = "");
  static Streamer* createInstance(Stream& stream, const String& name = "");
private:
  Streamer(const String& name);
  bool init(const Path& path);
  bool init(Stream& stream);
};

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ALBUFFER_H*/
///////////////////////////////////////////////////////////////////////

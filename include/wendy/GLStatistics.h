///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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
#ifndef WENDY_GLSTATISTICS_H
#define WENDY_GLSTATISTICS_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Statistics : public Singleton<Statistics>, public Trackable
{
public:
  void addPasses(unsigned int count);
  void addPrimitives(GLenum mode, unsigned int count);
  float getFrameRate(void) const;
  unsigned int getFrameCount(void) const;
  unsigned int getPassCount(void) const;
  unsigned int getVertexCount(void) const;
  unsigned int getPointCount(void) const;
  unsigned int getLineCount(void) const;
  unsigned int getTriangleCount(void) const;
  static bool create(void);
private:
  Statistics(void);
  bool init(void);
  void onFinish(void);
  void onContextDestroy(void);
  unsigned int frameCount;
  unsigned int passCount;
  unsigned int vertexCount;
  unsigned int pointCount;
  unsigned int lineCount;
  unsigned int triangleCount;
  Timer timer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSTATISTICS_H*/
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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
#ifndef WEGLRENDER_H
#define WEGLRENDER_H
///////////////////////////////////////////////////////////////////////

#include <string>
#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

class Shader;
class Light;
class VertexBuffer;
class IndexBuffer;

///////////////////////////////////////////////////////////////////////

class RenderOperation
{
public:
  RenderOperation(void);
  bool operator < (const RenderOperation& other) const;
  const VertexBuffer* vertexBuffer;
  const IndexBuffer* indexBuffer;
  const Shader* shader;
  Matrix4 transform;
  GLenum renderMode;
};

///////////////////////////////////////////////////////////////////////

class RenderQueue
{
public:
  typedef std::vector<Light*> LightList;
  typedef std::vector<RenderOperation> OperationList;
  RenderQueue(void);
  void addLight(Light& light);
  void removeLights(void);
  void addOperation(RenderOperation& operation);
  void removeOperations(void);
  void renderOperations(void);
  const LightList& getLights(void) const;
  const OperationList& getOperations(void) const;
private:
  void sortOperations(void);
  LightList lights;
  OperationList operations;
  bool sorted;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WEGLRENDER_H*/
///////////////////////////////////////////////////////////////////////

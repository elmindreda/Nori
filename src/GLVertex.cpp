///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.h>

#include <wendy/GLVertex.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fv::format("3f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fn3fv::format("3f:normal 3f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2fv::format("2f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft2fv::format("2f:mapping 2f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft3fv::format("2f:mapping 3f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex4fc2ft3fv::format("4f:color 2f:mapping 3f:position");

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

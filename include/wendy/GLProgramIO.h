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
#ifndef WENDY_GLPROGRAMIO_H
#define WENDY_GLSPROGRAMO_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex shader codec.
 *  @ingroup io
 */
class VertexProgramCodec : public ResourceCodec<VertexProgram>
{
public:
  VertexProgramCodec(void);
  VertexProgram* read(const Path& path, const String& name = "");
  VertexProgram* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const VertexProgram& program);
  bool write(Stream& stream, const VertexProgram& program);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Fragment shader codec.
 *  @ingroup io
 */
class FragmentProgramCodec : public ResourceCodec<FragmentProgram>
{
public:
  FragmentProgramCodec(void);
  FragmentProgram* read(const Path& path, const String& name = "");
  FragmentProgram* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const FragmentProgram& program);
  bool write(Stream& stream, const FragmentProgram& program);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Shader program XML codec.
 *  @ingroup io
 */
class ProgramCodec : public ResourceCodec<Program>, public XML::Codec
{
public:
  ProgramCodec(void);
  Program* read(const Path& path, const String& name = "");
  Program* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Program& program);
  bool write(Stream& stream, const Program& program);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<Program> program;
  Ref<VertexProgram> vertexProgram;
  Ref<FragmentProgram> fragmentProgram;
  String programName;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLPROGRAMIO_H*/
///////////////////////////////////////////////////////////////////////

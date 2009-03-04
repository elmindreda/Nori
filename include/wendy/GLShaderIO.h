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
#ifndef WENDY_GLSHADERIO_H
#define WENDY_GLSHADERIO_H
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
class VertexShaderCodec : public ResourceCodec<VertexShader>
{
public:
  VertexShaderCodec(void);
  VertexShader* read(const Path& path, const String& name = "");
  VertexShader* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const VertexShader& program);
  bool write(Stream& stream, const VertexShader& program);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Fragment shader codec.
 *  @ingroup io
 */
class FragmentShaderCodec : public ResourceCodec<FragmentShader>
{
public:
  FragmentShaderCodec(void);
  FragmentShader* read(const Path& path, const String& name = "");
  FragmentShader* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const FragmentShader& program);
  bool write(Stream& stream, const FragmentShader& program);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Shader program XML codec.
 *  @ingroup io
 */
class ShaderProgramCodec : public ResourceCodec<ShaderProgram>, public XML::Codec
{
public:
  ShaderProgramCodec(void);
  ShaderProgram* read(const Path& path, const String& name = "");
  ShaderProgram* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const ShaderProgram& program);
  bool write(Stream& stream, const ShaderProgram& program);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<ShaderProgram> program;
  Ref<VertexShader> vertexShader;
  Ref<FragmentShader> fragmentShader;
  String programName;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSHADERIO_H*/
///////////////////////////////////////////////////////////////////////

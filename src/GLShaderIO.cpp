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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLShader.h>
#include <wendy/GLShaderIO.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int SHADER_PROGRAM_XML_VERSION = 1;

}

///////////////////////////////////////////////////////////////////////

VertexShaderCodec::VertexShaderCodec(void):
  ResourceCodec<VertexShader>("Vertex shader codec")
{
  addSuffix("vs");
  addSuffix("cg");
}

VertexShader* VertexShaderCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<VertexShader>::read(path, name);
}

VertexShader* VertexShaderCodec::read(Stream& stream, const String& name)
{
  TextStream textStream(stream, false);

  String text;
  textStream.readText(text, textStream.getSize());

  return VertexShader::createInstance(*GL::Context::get(), text, name);
}

bool VertexShaderCodec::write(const Path& path, const VertexShader& program)
{
  return ResourceCodec<VertexShader>::write(path, program);
}

bool VertexShaderCodec::write(Stream& stream, const VertexShader& program)
{
  const String& text = program.getText();

  return stream.writeItems(text.c_str(), text.size());
}

///////////////////////////////////////////////////////////////////////

FragmentShaderCodec::FragmentShaderCodec(void):
  ResourceCodec<FragmentShader>("Fragment shader codec") 
{
  addSuffix("fs");
  addSuffix("cg");
}

FragmentShader* FragmentShaderCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<FragmentShader>::read(path, name);
}

FragmentShader* FragmentShaderCodec::read(Stream& stream, const String& name)
{
  TextStream textStream(stream, false);

  String text;
  textStream.readText(text, textStream.getSize());

  return FragmentShader::createInstance(*GL::Context::get(), text, name);
}

bool FragmentShaderCodec::write(const Path& path, const FragmentShader& program)
{
  return ResourceCodec<FragmentShader>::write(path, program);
}

bool FragmentShaderCodec::write(Stream& stream, const FragmentShader& program)
{
  const String& text = program.getText();

  return stream.writeItems(text.c_str(), text.size());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

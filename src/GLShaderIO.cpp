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

  return VertexShader::createInstance(*Context::get(), text, name);
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

  return FragmentShader::createInstance(*Context::get(), text, name);
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

ShaderProgramCodec::ShaderProgramCodec(void):
  ResourceCodec<ShaderProgram>("XML shader program codec")
{
  addSuffix("program");
}

ShaderProgram* ShaderProgramCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<ShaderProgram>::read(path, name);
}

ShaderProgram* ShaderProgramCodec::read(Stream& stream, const String& name)
{
  programName = name;

  if (!XML::Codec::read(stream))
  {
    program = NULL;
    return NULL;
  }

  if (!program)
  {
    Log::writeError("No shader program specification found in file");
    return NULL;
  }

  return program.detachObject();
}

bool ShaderProgramCodec::write(const Path& path, const ShaderProgram& program)
{
  return ResourceCodec<ShaderProgram>::write(path, program);
}

bool ShaderProgramCodec::write(Stream& stream, const ShaderProgram& program)
{
  try
  {
    setStream(&stream);

    beginElement("program");
    addAttribute("version", (int) SHADER_PROGRAM_XML_VERSION);

    beginElement("vertex-shader");
    addAttribute("name", program.getVertexShader().getName());
    endElement();

    beginElement("fragment-shader");
    addAttribute("name", program.getFragmentShader().getName());
    endElement();

    endElement();
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write shader program %s: %s", program.getName().c_str(), exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool ShaderProgramCodec::onBeginElement(const String& name)
{
  if (name == "program")
  {
    if (program)
    {
      Log::writeError("Only one shader program per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != SHADER_PROGRAM_XML_VERSION)
    {
      Log::writeError("Shader program XML format version mismatch");
      return false;
    }

    return true;
  }

  if (name == "vertex-shader")
  {
    if (vertexShader)
    {
      Log::writeError("Cannot nest vertex shaders");
      return false;
    }

    String vertexShaderName = readString("name");
    if (!vertexShaderName.length())
      return true;

    vertexShader = VertexShader::readInstance(vertexShaderName);
    if (!vertexShader)
      return false;

    return true;
  }

  if (name == "fragment-shader")
  {
    if (fragmentShader)
    {
      Log::writeError("Cannot nest fragment shaders");
      return false;
    }

    String fragmentShaderName = readString("name");
    if (!fragmentShaderName.length())
      return true;

    fragmentShader = FragmentShader::readInstance(fragmentShaderName);
    if (!fragmentShader)
      return false;

    return true;
  }

  return true;
}

bool ShaderProgramCodec::onEndElement(const String& name)
{
  if (name == "program")
  {
    if (!vertexShader)
    {
      Log::writeError("Vertex shader missing for shader program %s",
		      programName.c_str());
      return false;
    }

    if (!fragmentShader)
    {
      Log::writeError("Fragment shader missing for shader program %s",
		      programName.c_str());
      return false;
    }

    program = ShaderProgram::createInstance(*Context::get(), *vertexShader, *fragmentShader, programName);
    if (!program)
      return false;

    vertexShader = NULL;
    fragmentShader = NULL;

    return true;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

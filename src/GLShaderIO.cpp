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
  ResourceCodec<VertexShader>("GLSL vertex shader codec")
{
  addSuffix("vs");
}

VertexShader* VertexShaderCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<VertexShader>::read(path, name);
}

VertexShader* VertexShaderCodec::read(Stream& stream, const String& name)
{
  Ptr<TextStream> textStream = new TextStream(&stream, false);

  String text;
  textStream->readText(text, textStream->getSize());

  return VertexShader::createInstance(text, name);
}

bool VertexShaderCodec::write(const Path& path, const VertexShader& program)
{
  return ResourceCodec<VertexShader>::write(path, program);
}

bool VertexShaderCodec::write(Stream& stream, const VertexShader& program)
{
  // TODO: The code.

  return false;
}

///////////////////////////////////////////////////////////////////////

FragmentShaderCodec::FragmentShaderCodec(void):
  ResourceCodec<FragmentShader>("GLSL fragment shader codec") 
{
  addSuffix("fs");
}

FragmentShader* FragmentShaderCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<FragmentShader>::read(path, name);
}

FragmentShader* FragmentShaderCodec::read(Stream& stream, const String& name)
{
  Ptr<TextStream> textStream = new TextStream(&stream, false);

  String text;
  textStream->readText(text, textStream->getSize());

  return FragmentShader::createInstance(text, name);
}

bool FragmentShaderCodec::write(const Path& path, const FragmentShader& program)
{
  return ResourceCodec<FragmentShader>::write(path, program);
}

bool FragmentShaderCodec::write(Stream& stream, const FragmentShader& program)
{
  // TODO: The code.

  return false;
}

///////////////////////////////////////////////////////////////////////

ShaderProgramCodec::ShaderProgramCodec(void):
  ResourceCodec<ShaderProgram>("XML GLSL program codec")
{
  addSuffix("program");
}

ShaderProgram* ShaderProgramCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<ShaderProgram>::read(path, name);
}

ShaderProgram* ShaderProgramCodec::read(Stream& stream, const String& name)
{
  currentShader = NULL;

  programName = name;

  if (!XML::Codec::read(stream))
    return NULL;

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

    for (unsigned int i = 0;  i < program.getShaderCount();  i++)
    {
      const Shader& shader = program.getShader(i);

      if (shader.getType() == Shader::VERTEX)
      {
	const VertexShader* vertexShader = dynamic_cast<const VertexShader*>(&shader);

	beginElement("vertex-shader");
	addAttribute("name", vertexShader->getName());
      }
      else if (shader.getType() == Shader::FRAGMENT)
      {
	const FragmentShader* fragmentShader = dynamic_cast<const FragmentShader*>(&shader);

	beginElement("fragment-shader");
	addAttribute("name", fragmentShader->getName());
      }
      else
	throw Exception("Unknown shader type");

      endElement();
    }

    endElement();
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write GLSL program %s: %s", program.getName().c_str(), exception.what());
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
      Log::writeError("Only one GLSL program per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != SHADER_PROGRAM_XML_VERSION)
    {
      Log::writeError("GLSL program XML format version mismatch");
      return false;
    }

    program = ShaderProgram::createInstance(programName);
    return true;
  }

  if (program)
  {
    if (name == "vertex-shader")
    {
      if (currentShader)
      {
	Log::writeError("Cannot nest GLSL shaders");
	return false;
      }

      String vertexShaderName = readString("name");
      if (!vertexShaderName.length())
	return true;

      currentShader = VertexShader::readInstance(vertexShaderName);
      if (!currentShader)
	return false;

      program->addShader(*currentShader);
      return true;
    }

    if (name == "fragment-shader")
    {
      if (currentShader)
      {
	Log::writeError("Cannot nest GLSL shaders");
	return false;
      }

      String fragmentShaderName = readString("name");
      if (!fragmentShaderName.length())
	return true;

      currentShader = FragmentShader::readInstance(fragmentShaderName);
      if (!currentShader)
	return false;

      program->addShader(*currentShader);
      return true;
    }
  }

  return true;
}

bool ShaderProgramCodec::onEndElement(const String& name)
{
  if (program)
  {
    if (name == "program")
    {
      if (!program->link())
	return false;

      return true;
    }

    if (currentShader)
    {
      if (name == "vertex-shader" || name == "fragment-shader")
      {
	currentShader = NULL;
	return true;
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

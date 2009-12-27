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

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>

#include <internal/GLProgramIO.h>

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

const unsigned int PROGRAM_XML_VERSION = 2;

}

///////////////////////////////////////////////////////////////////////

VertexProgramCodec::VertexProgramCodec(void):
  ResourceCodec<VertexProgram>("Vertex program codec")
{
  addSuffix("vp");
  addSuffix("cg");
}

VertexProgram* VertexProgramCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<VertexProgram>::read(path, name);
}

VertexProgram* VertexProgramCodec::read(Stream& stream, const String& name)
{
  TextStream textStream(stream, false);

  String text;
  textStream.readText(text, (size_t) textStream.getSize());

  return VertexProgram::createInstance(*Context::get(), text, name);
}

bool VertexProgramCodec::write(const Path& path, const VertexProgram& program)
{
  return ResourceCodec<VertexProgram>::write(path, program);
}

bool VertexProgramCodec::write(Stream& stream, const VertexProgram& program)
{
  const String& text = program.getText();

  return stream.writeItems(text.c_str(), text.size());
}

///////////////////////////////////////////////////////////////////////

FragmentProgramCodec::FragmentProgramCodec(void):
  ResourceCodec<FragmentProgram>("Fragment program codec")
{
  addSuffix("fp");
  addSuffix("cg");
}

FragmentProgram* FragmentProgramCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<FragmentProgram>::read(path, name);
}

FragmentProgram* FragmentProgramCodec::read(Stream& stream, const String& name)
{
  TextStream textStream(stream, false);

  String text;
  textStream.readText(text, (size_t) textStream.getSize());

  return FragmentProgram::createInstance(*Context::get(), text, name);
}

bool FragmentProgramCodec::write(const Path& path, const FragmentProgram& program)
{
  return ResourceCodec<FragmentProgram>::write(path, program);
}

bool FragmentProgramCodec::write(Stream& stream, const FragmentProgram& program)
{
  const String& text = program.getText();

  return stream.writeItems(text.c_str(), text.size());
}

///////////////////////////////////////////////////////////////////////

ProgramCodec::ProgramCodec(void):
  ResourceCodec<Program>("XML shader program codec")
{
  addSuffix("program");
}

Program* ProgramCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<Program>::read(path, name);
}

Program* ProgramCodec::read(Stream& stream, const String& name)
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

bool ProgramCodec::write(const Path& path, const Program& program)
{
  return ResourceCodec<Program>::write(path, program);
}

bool ProgramCodec::write(Stream& stream, const Program& program)
{
  try
  {
    setStream(&stream);

    beginElement("program");
    addAttribute("version", (int) PROGRAM_XML_VERSION);

    beginElement("vertex");
    addAttribute("name", program.getVertexProgram().getName());
    endElement();

    beginElement("fragment");
    addAttribute("name", program.getFragmentProgram().getName());
    endElement();

    endElement();
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write shader program \'%s\': %s", program.getName().c_str(), exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool ProgramCodec::onBeginElement(const String& name)
{
  if (name == "program")
  {
    if (program)
    {
      Log::writeError("Only one shader program per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != PROGRAM_XML_VERSION)
    {
      Log::writeError("Shader program XML format version mismatch");
      return false;
    }

    return true;
  }

  if (name == "vertex")
  {
    if (vertexProgram)
    {
      Log::writeError("Cannot nest vertex programs");
      return false;
    }

    String vertexProgramName = readString("name");
    if (!vertexProgramName.length())
    {
      Log::writeError("Vertex program name in shader program \'%s\' is empty", programName.c_str());
      return true;
    }

    vertexProgram = VertexProgram::readInstance(vertexProgramName);
    if (!vertexProgram)
    {
      Log::writeError("Cannot find vertex program \'%s\' for shader program \'%s\'",
                      vertexProgramName.c_str(),
		      programName.c_str());
      return false;
    }

    return true;
  }

  if (name == "fragment")
  {
    if (fragmentProgram)
    {
      Log::writeError("Cannot nest fragment programs");
      return false;
    }

    String fragmentProgramName = readString("name");
    if (!fragmentProgramName.length())
    {
      Log::writeError("Fragment program name in shader program \'%s\' is empty", programName.c_str());
      return true;
    }

    fragmentProgram = FragmentProgram::readInstance(fragmentProgramName);
    if (!fragmentProgram)
    {
      Log::writeError("Cannot find fragment program \'%s\' for shader program \'%s\'",
                      fragmentProgramName.c_str(),
		      programName.c_str());
      return false;
    }

    return true;
  }

  return true;
}

bool ProgramCodec::onEndElement(const String& name)
{
  if (name == "program")
  {
    if (!vertexProgram)
    {
      Log::writeError("Vertex program missing for shader program \'%s\'",
		      programName.c_str());
      return false;
    }

    if (!fragmentProgram)
    {
      Log::writeError("Fragment program missing for shader program \'%s\'",
		      programName.c_str());
      return false;
    }

    program = Program::createInstance(*Context::get(), *vertexProgram, *fragmentProgram, programName);
    if (!program)
      return false;

    vertexProgram = NULL;
    fragmentProgram = NULL;

    return true;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

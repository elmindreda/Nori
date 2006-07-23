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
#include <wendy/GLProgram.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

VertexProgram::~VertexProgram(void)
{
  if (programID != 0)
    glDeleteProgramsARB(1, &programID);
}

GLuint VertexProgram::getGLID(void) const
{
  return programID;
}

bool VertexProgram::getParameter(unsigned int index, Vector4& value) const
{
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, programID);
  glGetProgramLocalParameterfvARB(GL_VERTEX_PROGRAM_ARB, index, value);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Failed to set vertex program parameter: %s", gluErrorString(error));
    return false;
  }

  return true;
}

bool VertexProgram::setParameter(unsigned int index, const Vector4& newValue)
{
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, programID);
  glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, newValue);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Failed to set vertex program parameter: %s", gluErrorString(error));
    return false;
  }

  return true;
}

VertexProgram* VertexProgram::createInstance(const String& text,
                                             const String& name)
{
  Ptr<VertexProgram> program = new VertexProgram(name);
  if (!program->init(text))
    return NULL;

  return program.detachObject();
}

VertexProgram::VertexProgram(const String& name):
  Resource<VertexProgram>(name),
  programID(0)
{
}

bool VertexProgram::init(const String& text)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create vertex program without OpenGL context");
    return false;
  }

  if (!GLEW_ARB_vertex_program)
  {
    Log::writeError("Vertex programs are not supported by the current OpenGL context");
    return false;
  }

  glGenProgramsARB(1, &programID);
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, programID);
  glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
                     GL_PROGRAM_FORMAT_ASCII_ARB,
		     text.length(), text.c_str());

  const GLubyte* message = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

  GLenum error = glGetError();
  if (error == GL_NO_ERROR)
  {
    if (strlen((const char*) message))
      Log::writeWarning("Warnings during vertex program compilation: %s", message);
  }
  else
  {
    GLint position;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &position);

    Log::writeError("Failed to compile vertex program; error at %u: %s", position, message);
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

VertexProgramCodec::VertexProgramCodec(void):
  ResourceCodec<VertexProgram>("OpenGL ARB vertex program codec")
{
}

VertexProgram* VertexProgramCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<VertexProgram>::read(path, name);
}

VertexProgram* VertexProgramCodec::read(Stream& stream, const String& name)
{
  Ptr<TextStream> textStream = new TextStream(&stream, false);

  String text;
  textStream->readText(text, textStream->getSize());

  return VertexProgram::createInstance(text, name);
}

bool VertexProgramCodec::write(const Path& path, const VertexProgram& program)
{
  return ResourceCodec<VertexProgram>::write(path, program);
}

bool VertexProgramCodec::write(Stream& stream, const VertexProgram& program)
{
  // TODO: The code.

  return false;
}

///////////////////////////////////////////////////////////////////////

FragmentProgram::~FragmentProgram(void)
{
  if (programID != 0)
    glDeleteProgramsARB(1, &programID);
}

GLuint FragmentProgram::getGLID(void) const
{
  return programID;
}

bool FragmentProgram::getParameter(unsigned int index, Vector4& value) const
{
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, programID);
  glGetProgramLocalParameterfvARB(GL_FRAGMENT_PROGRAM_ARB, index, value);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Failed to set fragment program parameter: %s", gluErrorString(error));
    return false;
  }

  return true;
}

bool FragmentProgram::setParameter(unsigned int index, const Vector4& newValue)
{
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, programID);
  glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index, newValue);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Failed to set fragment program parameter: %s", gluErrorString(error));
    return false;
  }

  return true;
}

FragmentProgram* FragmentProgram::readInstance(const Path& path,
                                               const String& name)
{
  Stream* file = FileStream::createInstance(path, Stream::READABLE);
  Ptr<TextStream> stream = TextStream::createInstance(file);
  if (!stream)
    return NULL;

  String text;
  stream->readText(text, stream->getSize());

  return createInstance(text, name);
}

FragmentProgram* FragmentProgram::createInstance(const String& text,
                                                 const String& name)
{
  Ptr<FragmentProgram> program = new FragmentProgram(name);
  if (!program->init(text))
    return NULL;

  return program.detachObject();
}

FragmentProgram::FragmentProgram(const String& name):
  Managed<FragmentProgram>(name),
  programID(0)
{
}

bool FragmentProgram::init(const String& text)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create fragment program without OpenGL context");
    return false;
  }

  if (!GLEW_ARB_fragment_program)
  {
    Log::writeError("Fragment programs are not supported by the current OpenGL context");
    return false;
  }

  glGenProgramsARB(1, &programID);
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, programID);
  glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
                     GL_PROGRAM_FORMAT_ASCII_ARB,
		     text.length(), text.c_str());

  const GLubyte* message = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

  GLenum error = glGetError();
  if (error == GL_NO_ERROR)
  {
    if (strlen((const char*) message))
      Log::writeWarning("Warnings during fragment program compilation: %s", message);
  }
  else
  {
    GLint position;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &position);

    const GLubyte* message = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

    Log::writeError("Failed to compile fragment program; error at %u: %s", position, message);
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

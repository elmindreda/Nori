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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Block.h>

#include <wendy/OpenGL.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

#include <algorithm>
#include <cstring>
#include <sstream>
#include <map>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

unsigned int getElementCount(Attribute::Type type)
{
  switch (type)
  {
    case Attribute::FLOAT:
      return 1;
    case Attribute::VEC2:
      return 2;
    case Attribute::VEC3:
      return 3;
    case Attribute::VEC4:
      return 4;
  }

  panic("Invalid GLSL attribute type %u", type);
}

GLenum getElementType(Attribute::Type type)
{
  switch (type)
  {
    case Attribute::FLOAT:
    case Attribute::VEC2:
    case Attribute::VEC3:
    case Attribute::VEC4:
      return GL_FLOAT;
  }

  panic("Invalid GLSL attribute type %u", type);
}

bool isSupportedAttributeType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
      return true;
  }

  return false;
}

Attribute::Type convertAttributeType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
      return Attribute::FLOAT;
    case GL_FLOAT_VEC2:
      return Attribute::VEC2;
    case GL_FLOAT_VEC3:
      return Attribute::VEC3;
    case GL_FLOAT_VEC4:
      return Attribute::VEC4;
  }

  panic("Unsupported GLSL attribute type %u", type);
}

bool isSupportedSamplerType(GLenum type)
{
  switch (type)
  {
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_CUBE:
      return true;
  }

  return false;
}

Sampler::Type convertSamplerType(GLenum type)
{
  switch (type)
  {
    case GL_SAMPLER_1D:
      return Sampler::SAMPLER_1D;
    case GL_SAMPLER_2D:
      return Sampler::SAMPLER_2D;
    case GL_SAMPLER_3D:
      return Sampler::SAMPLER_3D;
    case GL_SAMPLER_2D_RECT_ARB:
      return Sampler::SAMPLER_RECT;
    case GL_SAMPLER_CUBE:
      return Sampler::SAMPLER_CUBE;
  }

  panic("Unsupported GLSL sampler type %u", type);
}

bool isSupportedUniformType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT3:
    case GL_FLOAT_MAT4:
      return true;
  }

  return false;
}

Uniform::Type convertUniformType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
      return Uniform::FLOAT;
    case GL_FLOAT_VEC2:
      return Uniform::VEC2;
    case GL_FLOAT_VEC3:
      return Uniform::VEC3;
    case GL_FLOAT_VEC4:

      return Uniform::VEC4;
    case GL_FLOAT_MAT2:
      return Uniform::MAT2;
    case GL_FLOAT_MAT3:
      return Uniform::MAT3;
    case GL_FLOAT_MAT4:
      return Uniform::MAT4;
  }

  panic("Unsupported GLSL uniform type %u", type);
}

bool readTextFile(ResourceCache& cache, String& text, const Path& path)
{
  std::ifstream stream;
  if (!cache.openFile(stream, path))
    return false;

  stream.seekg(0, std::ios::end);

  text.resize((unsigned int) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  return true;
}

GLuint createShader(GL::Context& context, GLenum type, const Shader& shader)
{
  String decl;

  if (shader.version > 100)
  {
    std::ostringstream stream;
    stream << shader.version;

    decl += "#version ";
    decl += stream.str();
    decl += "\n";
  }

  decl += "#line 0 0\n";
  decl += context.getSharedProgramStateDeclaration();

  String main;
  main += "#line 0 1\n";
  main += shader.text;

  GLsizei lengths[2];
  const GLchar* strings[2];

  lengths[0] = decl.length();
  strings[0] = (const GLchar*) decl.c_str();

  lengths[1] = main.length();
  strings[1] = (const GLchar*) main.c_str();

  GLuint shaderID = glCreateShader(type);
  glShaderSource(shaderID, 2, strings, lengths);
  glCompileShader(shaderID);

  GLint status;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);

  GLint length;
  glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);

  String infoLog;

  if (length > 0)
  {
    infoLog.resize(length);
    glGetShaderInfoLog(shaderID, length, NULL, &infoLog[0]);
  }

  if (!status)
  {
    if (length > 0)
    {
      logError("Failed to compile shader \'%s\':\n%s",
               shader.path.asString().c_str(),
               infoLog.c_str());
    }
    else
      checkGL("Failed to compile shader \'%s\'", shader.path.asString().c_str());

    glDeleteShader(shaderID);
    return 0;
  }

  if (length > 1)
  {
    logWarning("Warning(s) compiling shader \'%s\':\n%s",
               shader.path.asString().c_str(),
               infoLog.c_str());
  }

  if (!checkGL("Failed to create object for shader \'%s\'",
               shader.path.asString().c_str()))
  {
    glDeleteShader(shaderID);
    return 0;
  }

  return shaderID;
}

String getProgramInfoLog(GLuint programID)
{
  GLint length;
  glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);

  if (length > 0)
  {
    String infoLog;
    infoLog.resize(length);

    glGetProgramInfoLog(programID, length, NULL, &infoLog[0]);

    return infoLog;
  }

  return String();
}

const unsigned int PROGRAM_XML_VERSION = 4;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Shader::Shader(const char* initText, const Path& initPath, unsigned int initVersion):
  text(initText),
  path(initPath),
  version(initVersion)
{
}

///////////////////////////////////////////////////////////////////////

bool Attribute::operator == (const char* string) const
{
  return name == string;
}

bool Attribute::isScalar() const
{
  return type == FLOAT;
}

bool Attribute::isVector() const
{
  if (type == VEC2 || type == VEC3 || type == VEC4)
    return true;

  return false;
}

Attribute::Type Attribute::getType() const
{
  return type;
}

const String& Attribute::getName() const
{
  return name;
}

void Attribute::bind(size_t stride, size_t offset)
{
  glVertexAttribPointer(location,
                        getElementCount(type),
                        getElementType(type),
                        GL_FALSE,
                        stride,
                        (const void*) offset);

#if WENDY_DEBUG
  checkGL("Failed to set attribute \'%s\'", name.c_str());
#endif
}

const char* Attribute::getTypeName(Type type)
{
  switch (type)
  {
    case FLOAT:
      return "float";
    case VEC2:
      return "vec2";
    case VEC3:
      return "vec3";
    case VEC4:
      return "vec4";
  }

  panic("Invalid GLSL attribute type %u", type);
}

///////////////////////////////////////////////////////////////////////

void Sampler::bind(unsigned int unit)
{
  glUniform1i(location, unit);

#if WENDY_DEBUG
  checkGL("Failed to set sampler \'%s\'", name.c_str());
#endif
}

bool Sampler::operator == (const char* string) const
{
  return name == string;
}

bool Sampler::isShared() const
{
  return sharedID != INVALID_SHARED_STATE_ID;
}

Sampler::Type Sampler::getType() const
{
  return type;
}

const String& Sampler::getName() const
{
  return name;
}

int Sampler::getSharedID() const
{
  return sharedID;
}

const char* Sampler::getTypeName(Type type)
{
  switch (type)
  {
    case SAMPLER_1D:
      return "sampler1D";
    case SAMPLER_2D:
      return "sampler2D";
    case SAMPLER_3D:
      return "sampler3D";
    case SAMPLER_RECT:
      return "sampler2DRect";
    case SAMPLER_CUBE:
      return "samplerCube";
  }

  panic("Invalid GLSL sampler type %u", type);
}

///////////////////////////////////////////////////////////////////////

void Uniform::copyFrom(const void* data)
{
  switch (type)
  {
    case FLOAT:
      glUniform1fv(location, 1, (const float*) data);
      break;
    case VEC2:
      glUniform2fv(location, 1, (const float*) data);
      break;
    case VEC3:
      glUniform3fv(location, 1, (const float*) data);
      break;
    case VEC4:
      glUniform4fv(location, 1, (const float*) data);
      break;
    case MAT2:
      glUniformMatrix2fv(location, 1, GL_FALSE, (const float*) data);
      break;
    case MAT3:
      glUniformMatrix3fv(location, 1, GL_FALSE, (const float*) data);
      break;
    case MAT4:
      glUniformMatrix4fv(location, 1, GL_FALSE, (const float*) data);
      break;
  }

#if WENDY_DEBUG
  checkGL("Failed to set uniform \'%s\'", name.c_str());
#endif
}

bool Uniform::operator == (const char* string) const
{
  return name == string;
}

bool Uniform::isShared() const
{
  return sharedID != INVALID_SHARED_STATE_ID;
}

bool Uniform::isScalar() const
{
  return type == FLOAT;
}

bool Uniform::isVector() const
{
  if (type == VEC2 || type == VEC3 || type == VEC4)
    return true;

  return false;
}

bool Uniform::isMatrix() const
{
  if (type == MAT2 || type == MAT3 || type == MAT4)
    return true;

  return false;
}

Uniform::Type Uniform::getType() const
{
  return type;
}

const String& Uniform::getName() const
{
  return name;
}

unsigned int Uniform::getElementCount() const
{
  switch (type)
  {
    case Uniform::FLOAT:
      return 1;
    case Uniform::VEC2:
      return 2;
    case Uniform::VEC3:
      return 3;
    case Uniform::VEC4:
      return 4;
    case Uniform::MAT2:
      return 2 * 2;
    case Uniform::MAT3:
      return 3 * 3;
    case Uniform::MAT4:
      return 4 * 4;
  }

  panic("Invalid GLSL uniform type %u", type);
}

int Uniform::getSharedID() const
{
  return sharedID;
}

const char* Uniform::getTypeName(Type type)
{
  switch (type)
  {
    case FLOAT:
      return "float";
    case VEC2:
      return "vec2";
    case VEC3:
      return "vec3";
    case VEC4:
      return "vec4";
    case MAT2:
      return "mat2";
    case MAT3:
      return "mat3";
    case MAT4:
      return "mat4";
  }

  panic("Invalid GLSL uniform type %u", type);
}

///////////////////////////////////////////////////////////////////////

Program::~Program()
{
  if (vertexShaderID)
    glDeleteShader(vertexShaderID);

  if (fragmentShaderID)
    glDeleteShader(fragmentShaderID);

  if (geometryShaderID)
    glDeleteShader(geometryShaderID);

  if (tessCtrlShaderID)
    glDeleteShader(tessCtrlShaderID);

  if (tessEvalShaderID)
    glDeleteShader(tessEvalShaderID);

  if (programID)
    glDeleteProgram(programID);
}

Attribute* Program::findAttribute(const char* name)
{
  AttributeList::iterator i = std::find(attributes.begin(), attributes.end(), name);
  if (i == attributes.end())
    return NULL;

  return &(*i);
}

const Attribute* Program::findAttribute(const char* name) const
{
  AttributeList::const_iterator i = std::find(attributes.begin(), attributes.end(), name);
  if (i == attributes.end())
    return NULL;

  return &(*i);
}

Sampler* Program::findSampler(const char* name)
{
  SamplerList::iterator s = std::find(samplers.begin(), samplers.end(), name);
  if (s == samplers.end())
    return NULL;

  return &(*s);
}

const Sampler* Program::findSampler(const char* name) const
{
  SamplerList::const_iterator s = std::find(samplers.begin(), samplers.end(), name);
  if (s == samplers.end())
    return NULL;

  return &(*s);
}

Uniform* Program::findUniform(const char* name)
{
  UniformList::iterator i = std::find(uniforms.begin(), uniforms.end(), name);
  if (i == uniforms.end())
    return NULL;

  return &(*i);
}

const Uniform* Program::findUniform(const char* name) const
{
  UniformList::const_iterator i = std::find(uniforms.begin(), uniforms.end(), name);
  if (i == uniforms.end())
    return NULL;

  return &(*i);
}

bool Program::isCurrent() const
{
  return context.getCurrentProgram() == this;
}

bool Program::hasGeometryShader() const
{
  return geometryShaderID != 0;
}

bool Program::hasTessellation() const
{
  return tessCtrlShaderID && tessEvalShaderID;
}

unsigned int Program::getAttributeCount() const
{
  return attributes.size();
}

Attribute& Program::getAttribute(unsigned int index)
{
  return attributes[index];
}

const Attribute& Program::getAttribute(unsigned int index) const
{
  return attributes[index];
}

unsigned int Program::getSamplerCount() const
{
  return samplers.size();
}

Sampler& Program::getSampler(unsigned int index)
{
  return samplers[index];
}

const Sampler& Program::getSampler(unsigned int index) const
{
  return samplers[index];
}

unsigned int Program::getUniformCount() const
{
  return uniforms.size();
}

Uniform& Program::getUniform(unsigned int index)
{
  return uniforms[index];
}

const Uniform& Program::getUniform(unsigned int index) const
{
  return uniforms[index];
}

Context& Program::getContext() const
{
  return context;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader)
{
  Ref<Program> program(new Program(info, context));

  if (!program->attachShader(vertexShader, GL_VERTEX_SHADER))
    return NULL;
  if (!program->attachShader(fragmentShader, GL_FRAGMENT_SHADER))
    return NULL;

  if (!program->link())
    return NULL;

  return program;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader,
                             const Shader& geometryShader)
{
  Ref<Program> program(new Program(info, context));

  if (!program->attachShader(vertexShader, GL_VERTEX_SHADER))
    return NULL;
  if (!program->attachShader(fragmentShader, GL_FRAGMENT_SHADER))
    return NULL;
  if (!program->attachShader(geometryShader, GL_GEOMETRY_SHADER))
    return NULL;

  if (!program->link())
    return NULL;

  return program;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader,
                             const Shader& tessCtrlShader,
                             const Shader& tessEvalShader)
{
  Ref<Program> program(new Program(info, context));

  if (!program->attachShader(vertexShader, GL_VERTEX_SHADER))
    return NULL;
  if (!program->attachShader(fragmentShader, GL_FRAGMENT_SHADER))
    return NULL;
  if (!program->attachShader(tessCtrlShader, GL_TESS_CONTROL_SHADER))
    return NULL;
  if (!program->attachShader(tessEvalShader, GL_TESS_EVALUATION_SHADER))
    return NULL;

  if (!program->link())
    return NULL;

  return program;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader,
                             const Shader& geometryShader,
                             const Shader& tessCtrlShader,
                             const Shader& tessEvalShader)
{
  Ref<Program> program(new Program(info, context));

  if (!program->attachShader(vertexShader, GL_VERTEX_SHADER))
    return NULL;
  if (!program->attachShader(fragmentShader, GL_FRAGMENT_SHADER))
    return NULL;
  if (!program->attachShader(geometryShader, GL_GEOMETRY_SHADER))
    return NULL;
  if (!program->attachShader(tessCtrlShader, GL_TESS_CONTROL_SHADER))
    return NULL;
  if (!program->attachShader(tessEvalShader, GL_TESS_EVALUATION_SHADER))
    return NULL;

  if (!program->link())
    return NULL;

  return program;
}

Ref<Program> Program::read(Context& context, const Path& path)
{
  ProgramReader reader(context);
  return reader.read(path);
}

Program::Program(const ResourceInfo& info, Context& initContext):
  Resource(info),
  context(initContext),
  vertexShaderID(0),
  fragmentShaderID(0),
  geometryShaderID(0),
  tessCtrlShaderID(0),
  tessEvalShaderID(0),
  programID(0)
{
}

Program::Program(const Program& source):
  Resource(source),
  context(source.context),
  vertexShaderID(0),
  fragmentShaderID(0),
  geometryShaderID(0),
  tessCtrlShaderID(0),
  tessEvalShaderID(0),
  programID(0)
{
}

bool Program::attachShader(const Shader& shader, unsigned int type)
{
  GLuint shaderID = createShader(context, type, shader);
  if (!shaderID)
    return false;

  switch (type)
  {
    case GL_VERTEX_SHADER:
      vertexShaderID = shaderID;
      break;
    case GL_TESS_CONTROL_SHADER:
      tessCtrlShaderID = shaderID;
      break;
    case GL_TESS_EVALUATION_SHADER:
      tessEvalShaderID = shaderID;
      break;
    case GL_GEOMETRY_SHADER:
      geometryShaderID = shaderID;
      break;
    case GL_FRAGMENT_SHADER:
      fragmentShaderID = shaderID;
      break;
    default:
      panic("Invalid shader type");
  }

  return true;
}

bool Program::link()
{
  programID = glCreateProgram();
  if (!programID)
  {
    logError("Failed to create OpenGL object for program \'%s\'",
             getPath().asString().c_str());
    return false;
  }

  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);

  if (geometryShaderID)
  {
    if (!GLEW_ARB_geometry_shader4 && context.getVersion() < Version(3,2))
    {
      logError("Context does not support geometry shaders; cannot link program \'%s\'",
               getPath().asString().c_str());
      return false;
    }

    glAttachShader(programID, geometryShaderID);
  }

  if (tessCtrlShaderID && tessEvalShaderID)
  {
    if (!GLEW_ARB_tessellation_shader && context.getVersion() < Version(4,0))
    {
      logError("Context does not support :essellation shaders; cannot link program \'%s\'",
               getPath().asString().c_str());
      return false;
    }

    glAttachShader(programID, tessCtrlShaderID);
    glAttachShader(programID, tessEvalShaderID);
  }

  glLinkProgram(programID);

  String infoLog = getProgramInfoLog(programID);

  int status;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);

  if (!status)
  {
    logError("Failed to link program \'%s\':\n%s",
             path.asString().c_str(),
             infoLog.c_str());
    return false;
  }

  if (infoLog.length() > 1)
  {
    logWarning("Warning(s) when linking program \'%s\':\n%s",
               path.asString().c_str(),
               infoLog.c_str());
  }

  if (!checkGL("Failed to create object for program \'%s\'",
               getPath().asString().c_str()))
  {
    return false;
  }

  if (!retrieveUniforms())
    return false;

  if (!retrieveAttributes())
    return false;

  return true;
}

bool Program::retrieveUniforms()
{
  GLint uniformCount;
  glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &uniformCount);

  uniforms.reserve(uniformCount);

  GLint maxNameLength;
  glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

  char* uniformName = new char [maxNameLength + 1];

  for (int i = 0;  i < uniformCount;  i++)
  {
    GLsizei nameLength;
    GLint uniformSize;
    GLenum uniformType;

    glGetActiveUniform(programID,
                       i,
                       maxNameLength + 1,
                       &nameLength,
                       &uniformSize,
                       &uniformType,
                       uniformName);

    if (std::strncmp(uniformName, "gl_", 3) == 0)
    {
      logWarning("Program \'%s\' uses built-in uniform \'%s\'",
                 getPath().asString().c_str(),
                 uniformName);
      continue;
    }

    if (isSupportedUniformType(uniformType))
    {
      uniforms.push_back(Uniform());
      Uniform& uniform = uniforms.back();
      uniform.name = uniformName;
      uniform.type = convertUniformType(uniformType);
      uniform.location = glGetUniformLocation(programID, uniformName);
      uniform.sharedID = context.getSharedUniformID(uniform.name.c_str(), uniform.type);
    }
    else if (isSupportedSamplerType(uniformType))
    {
      samplers.push_back(Sampler());
      Sampler& sampler = samplers.back();
      sampler.name = uniformName;
      sampler.type = convertSamplerType(uniformType);
      sampler.location = glGetUniformLocation(programID, uniformName);
      sampler.sharedID = context.getSharedSamplerID(sampler.name.c_str(), sampler.type);
    }
    else
      logWarning("Skipping uniform \'%s\' of unsupported type", uniformName);
  }

  delete [] uniformName;

  if (!checkGL("Failed to retrieve uniforms for program \'%s\'",
               getPath().asString().c_str()))
  {
    return false;
  }

  return true;
}

bool Program::retrieveAttributes()
{
  GLint attributeCount;
  glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTES, &attributeCount);

  attributes.reserve(attributeCount);

  GLint maxNameLength;
  glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);

  char* attributeName = new char [maxNameLength + 1];

  for (int i = 0;  i < attributeCount;  i++)
  {
    GLsizei nameLength;
    GLint attributeSize;
    GLenum attributeType;

    glGetActiveAttrib(programID,
                      i,
                      maxNameLength + 1,
                      &nameLength,
                      &attributeSize,
                      &attributeType,
                      attributeName);

    if (!isSupportedAttributeType(attributeType))
    {
      logWarning("Skipping attribute \'%s\' of unsupported type", attributeName);
      continue;
    }

    attributes.push_back(Attribute());
    Attribute& attribute = attributes.back();
    attribute.name = attributeName;
    attribute.type = convertAttributeType(attributeType);
    attribute.location = glGetAttribLocation(programID, attributeName);
  }

  delete [] attributeName;

  if (!checkGL("Failed to retrieve attributes for program \'%s\'",
               getPath().asString().c_str()))
  {
    return false;
  }

  return true;
}

void Program::bind()
{
  glUseProgram(programID);

  typedef AttributeList::const_iterator It;

  for (It a = attributes.begin();  a != attributes.end();  a++)
    glEnableVertexAttribArray(a->location);
}

void Program::unbind()
{
  typedef AttributeList::const_iterator It;

  for (It a = attributes.begin();  a != attributes.end();  a++)
    glDisableVertexAttribArray(a->location);
}

Program& Program::operator = (const Program& source)
{
  return *this;
}

bool Program::isValid() const
{
  glValidateProgram(programID);

  int status;
  glGetProgramiv(programID, GL_VALIDATE_STATUS, &status);
  if (!status)
  {
    String infoLog = getProgramInfoLog(programID);
    logError("Failed to validate program \'%s\':\n%s",
             path.asString().c_str(),
             infoLog.c_str());

    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

void ProgramInterface::addSampler(const char* name, Sampler::Type type)
{
  samplers.push_back(SamplerList::value_type(name, type));
}

void ProgramInterface::addUniform(const char* name, Uniform::Type type)
{
  uniforms.push_back(UniformList::value_type(name, type));
}

void ProgramInterface::addAttribute(const char* name, Attribute::Type type)
{
  attributes.push_back(AttributeList::value_type(name, type));
}

bool ProgramInterface::matches(const Program& program, bool verbose) const
{
  for (size_t i = 0;  i < samplers.size();  i++)
  {
    const SamplerList::value_type& entry = samplers[i];

    const Sampler* sampler = program.findSampler(entry.first.c_str());
    if (!sampler)
    {
      if (verbose)
      {
        logError("Sampler \'%s\' missing in program \'%s\'",
                 entry.first.c_str(),
                 program.getPath().asString().c_str());
      }

      return false;
    }

    if (sampler->getType() != entry.second)
    {
      if (verbose)
      {
        logError("Sampler \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
                 entry.first.c_str(),
                 program.getPath().asString().c_str(),
                 Sampler::getTypeName(entry.second));
      }

      return false;
    }
  }

  for (size_t i = 0;  i < uniforms.size();  i++)
  {
    const UniformList::value_type& entry = uniforms[i];

    const Uniform* uniform = program.findUniform(entry.first.c_str());
    if (!uniform)
    {
      if (verbose)
      {
        logError("Uniform \'%s\' missing in program \'%s\'",
                 entry.first.c_str(),
                 program.getPath().asString().c_str());
      }

      return false;
    }

    if (uniform->getType() != entry.second)
    {
      if (verbose)
      {
        logError("Uniform \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
                 entry.first.c_str(),
                 program.getPath().asString().c_str(),
                 Uniform::getTypeName(entry.second));
      }

      return false;
    }
  }

  for (size_t i = 0;  i < attributes.size();  i++)
  {
    const AttributeList::value_type& entry = attributes[i];

    const Attribute* attribute = program.findAttribute(entry.first.c_str());
    if (!attribute)
    {
      if (verbose)
      {
        logError("Attribute \'%s\' missing in program \'%s\'",
                 entry.first.c_str(),
                 program.getPath().asString().c_str());
      }

      return false;
    }

    if (attribute->getType() != entry.second)
    {
      if (verbose)
      {
        logError("Attribute \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
                 entry.first.c_str(),
                 program.getPath().asString().c_str(),
                 Attribute::getTypeName(entry.second));
      }

      return false;
    }
  }

  return true;
}

bool ProgramInterface::matches(const VertexFormat& format, bool verbose) const
{
  if (format.getComponentCount() != attributes.size())
    return false;

  for (size_t i = 0;  i < attributes.size();  i++)
  {
    const AttributeList::value_type& entry = attributes[i];

    const VertexComponent* component = format.findComponent(entry.first.c_str());
    if (!component)
      return false;

    if (component->getType() != VertexComponent::FLOAT32)
      return false;

    if ((component->getElementCount() == 1 && entry.second != Attribute::FLOAT) ||
        (component->getElementCount() == 2 && entry.second != Attribute::VEC2) ||
        (component->getElementCount() == 3 && entry.second != Attribute::VEC3) ||
        (component->getElementCount() == 4 && entry.second != Attribute::VEC4))
    {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

ProgramReader::ProgramReader(Context& initContext):
  ResourceReader(initContext.getCache()),
  context(initContext)
{
}

Ref<Program> ProgramReader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<Program*>(cached);

  std::ifstream stream;
  if (!getCache().openFile(stream, path))
    return NULL;

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load GLSL program \'%s\': %s",
             path.asString().c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("program");
  if (!root || root.attribute("version").as_uint() != PROGRAM_XML_VERSION)
  {
    logError("GLSL program file format mismatch in \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  std::map<String, Shader> shaders;

  const char* names[] =
  {
    "vertex",
    "fragment",
    "geometry",
    "control",
    "evaluation",
  };

  for (size_t i = 0;  i < sizeof(names) / sizeof(names[0]);  i++)
  {
    if (pugi::xml_node s = root.child(names[i]))
    {
      const Path shaderPath(s.attribute("path").value());
      if (shaderPath.isEmpty())
      {
        logError("Path for %s shader in GLSL program \'%s\' is empty",
                names[i],
                path.asString().c_str());
        return NULL;
      }

      String text;
      if (!readTextFile(getCache(), text, shaderPath))
      {
        logError("Failed to load %s shader \'%s\' for GLSL program \'%s\'",
                names[i],
                shaderPath.asString().c_str(),
                path.asString().c_str());
        return NULL;
      }

      const unsigned int version = max(s.attribute("glsl-version").as_int(), 100);

      shaders[names[i]] = Shader(text.c_str(), shaderPath, version);
    }
  }

  if (!shaders.count("vertex"))
  {
    logError("Vertex shader missing in GLSL program \'%s\'",
              path.asString().c_str());
    return NULL;
  }

  if (!shaders.count("fragment"))
  {
    logError("Fragment shader missing in GLSL program \'%s\'",
              path.asString().c_str());
    return NULL;
  }

  bool tessellation = false;

  if (shaders.count("control"))
  {
    if (shaders.count("evaluation"))
      tessellation = true;
    else
    {
      logError("Both tessellation control and evaluation shader (or neither) "
               "are required in GLSL program \'%s\'",
               path.asString().c_str());
      return NULL;
    }
  }

  Ref<Program> program;

  if (shaders.count("geometry"))
  {
    if (tessellation)
    {
      program = Program::create(ResourceInfo(getCache(), path),
                                context,
                                shaders["vertex"],
                                shaders["fragment"],
                                shaders["geometry"],
                                shaders["control"],
                                shaders["evaluation"]);
    }
    else
    {
      program = Program::create(ResourceInfo(getCache(), path),
                                context,
                                shaders["vertex"],
                                shaders["fragment"],
                                shaders["geometry"]);
    }
  }
  else
  {
    if (tessellation)
    {
      program = Program::create(ResourceInfo(getCache(), path),
                                context,
                                shaders["vertex"],
                                shaders["fragment"],
                                shaders["control"],
                                shaders["evaluation"]);
    }
    else
    {
      program = Program::create(ResourceInfo(getCache(), path),
                                context,
                                shaders["vertex"],
                                shaders["fragment"]);
    }
  }

  return program;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

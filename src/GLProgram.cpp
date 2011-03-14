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

#include <algorithm>
#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

unsigned int getElementCount(Uniform::Type type)
{
  switch (type)
  {
    case Uniform::FLOAT:
    case Uniform::BOOL:
    case Uniform::INT:
    case Uniform::SAMPLER_1D:
    case Uniform::SAMPLER_2D:
    case Uniform::SAMPLER_3D:
    case Uniform::SAMPLER_RECT:
    case Uniform::SAMPLER_CUBE:
      return 1;
    case Uniform::FLOAT_VEC2:
    case Uniform::BOOL_VEC2:
    case Uniform::INT_VEC2:
      return 2;
    case Uniform::FLOAT_VEC3:
    case Uniform::BOOL_VEC3:
    case Uniform::INT_VEC3:
      return 3;
    case Uniform::FLOAT_VEC4:
    case Uniform::BOOL_VEC4:
    case Uniform::INT_VEC4:
      return 4;
    case Uniform::FLOAT_MAT2:
      return 2 * 2;
    case Uniform::FLOAT_MAT3:
      return 3 * 3;
    case Uniform::FLOAT_MAT4:
      return 4 * 4;
  }
}

unsigned int getElementCount(Attribute::Type type)
{
  switch (type)
  {
    case Attribute::FLOAT:
      return 1;
    case Attribute::FLOAT_VEC2:
      return 2;
    case Attribute::FLOAT_VEC3:
      return 3;
    case Attribute::FLOAT_VEC4:
      return 4;
    default:
      throw Exception("Invalid attribute type");
  }
}

GLenum getElementType(Attribute::Type type)
{
  switch (type)
  {
    case Attribute::FLOAT:
    case Attribute::FLOAT_VEC2:
    case Attribute::FLOAT_VEC3:
    case Attribute::FLOAT_VEC4:
      return GL_FLOAT;
    default:
      throw Exception("Invalid attribute type");
  }
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
    default:
      return false;
  }
}

Attribute::Type convertAttributeType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
      return Attribute::FLOAT;
    case GL_FLOAT_VEC2:
      return Attribute::FLOAT_VEC2;
    case GL_FLOAT_VEC3:
      return Attribute::FLOAT_VEC3;
    case GL_FLOAT_VEC4:
      return Attribute::FLOAT_VEC4;
    default:
      throw Exception("Unsupported GLSL attribute type");
  }
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
    case GL_BOOL:
    case GL_BOOL_VEC2:
    case GL_BOOL_VEC3:
    case GL_BOOL_VEC4:
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_CUBE:
      return true;

    default:
      return false;
  }
}

Uniform::Type convertUniformType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
      return Uniform::FLOAT;
    case GL_FLOAT_VEC2:
      return Uniform::FLOAT_VEC2;
    case GL_FLOAT_VEC3:
      return Uniform::FLOAT_VEC3;
    case GL_FLOAT_VEC4:

      return Uniform::FLOAT_VEC4;
    case GL_FLOAT_MAT2:
      return Uniform::FLOAT_MAT2;
    case GL_FLOAT_MAT3:
      return Uniform::FLOAT_MAT3;
    case GL_FLOAT_MAT4:
      return Uniform::FLOAT_MAT4;

    case GL_BOOL:
      return Uniform::BOOL;
    case GL_BOOL_VEC2:
      return Uniform::BOOL_VEC2;
    case GL_BOOL_VEC3:
      return Uniform::BOOL_VEC3;
    case GL_BOOL_VEC4:
      return Uniform::BOOL_VEC4;

    case GL_INT:
      return Uniform::INT;
    case GL_INT_VEC2:
      return Uniform::INT_VEC2;
    case GL_INT_VEC3:
      return Uniform::INT_VEC3;
    case GL_INT_VEC4:
      return Uniform::INT_VEC4;

    case GL_SAMPLER_1D:
      return Uniform::SAMPLER_1D;
    case GL_SAMPLER_2D:
      return Uniform::SAMPLER_2D;
    case GL_SAMPLER_3D:
      return Uniform::SAMPLER_3D;
    case GL_SAMPLER_2D_RECT_ARB:
      return Uniform::SAMPLER_RECT;
    case GL_SAMPLER_CUBE:
      return Uniform::SAMPLER_CUBE;

    default:
      throw Exception("Unsupported GLSL uniform type");
  }
}

const char* getTypeName(Uniform::Type type)
{
  switch (type)
  {
    case Uniform::FLOAT:
      return "float";
    case Uniform::FLOAT_VEC2:
      return "vec2";
    case Uniform::FLOAT_VEC3:
      return "vec3";
    case Uniform::FLOAT_VEC4:
      return "vec4";
    case Uniform::FLOAT_MAT2:
      return "mat2";
    case Uniform::FLOAT_MAT3:
      return "mat3";
    case Uniform::FLOAT_MAT4:
      return "mat4";
    case Uniform::BOOL:
      return "bool";
    case Uniform::BOOL_VEC2:
      return "bvec2";
    case Uniform::BOOL_VEC3:
      return "bvec3";
    case Uniform::BOOL_VEC4:
      return "bvec4";
    case Uniform::INT:
      return "int";
    case Uniform::INT_VEC2:
      return "ivec2";
    case Uniform::INT_VEC3:
      return "ivec3";
    case Uniform::INT_VEC4:
      return "ivec4";
    case Uniform::SAMPLER_1D:
      return "sampler1D";
    case Uniform::SAMPLER_2D:
      return "sampler2D";
    case Uniform::SAMPLER_3D:
      return "sampler3D";
    case Uniform::SAMPLER_RECT:
      return "sampler2DRect";
    case Uniform::SAMPLER_CUBE:
      return "samplerCube";
    default:
      throw Exception("Invalid uniform type");
  }
}

const char* getTypeName(Attribute::Type type)
{
  switch (type)
  {
    case Attribute::FLOAT:
      return "float";
    case Attribute::FLOAT_VEC2:
      return "vec2";
    case Attribute::FLOAT_VEC3:
      return "vec3";
    case Attribute::FLOAT_VEC4:
      return "vec4";
    default:
      throw Exception("Invalid attribute type");
  }
}

bool readTextFile(ResourceIndex& index, String& text, const Path& path)
{
  std::ifstream stream;
  if (!index.openFile(stream, path))
    return false;

  stream.seekg(0, std::ios::end);

  text.resize(stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  return true;
}

GLuint createShader(GLenum type, const Shader& shader)
{
  GLuint shaderID = glCreateShader(type);

  GLsizei length = shader.text.length();
  const GLchar* string = (const GLchar*) shader.text.c_str();

  glShaderSource(shaderID, 1, &string, &length);

  glCompileShader(shaderID);

  GLint status;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);

  if (!status)
  {
    GLint length;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);

    if (length > 0)
    {
      String infoLog;
      infoLog.resize(length);

      glGetShaderInfoLog(shaderID, length, NULL, &infoLog[0]);

      logError("Failed to compile shader \'%s\':\n%s",
               shader.path.asString().c_str(),
               infoLog.c_str());
    }
    else
      logError("Failed to compile shader \'%s\'",
               shader.path.asString().c_str());

    glDeleteShader(shaderID);
    return 0;
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

const unsigned int PROGRAM_XML_VERSION = 3;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Shader::Shader(void)
{
}

Shader::Shader(const String& initText, const Path& initPath):
  text(initText),
  path(initPath)
{
}

///////////////////////////////////////////////////////////////////////

bool Attribute::operator == (const String& string) const
{
  return name == string;
}

bool Attribute::isScalar(void) const
{
  return type == FLOAT;
}

bool Attribute::isVector(void) const
{
  if (type == FLOAT_VEC2 || type == FLOAT_VEC3 || type == FLOAT_VEC4)
    return true;

  return false;
}

Attribute::Type Attribute::getType(void) const
{
  return type;
}

const String& Attribute::getName(void) const
{
  return name;
}

void Attribute::enable(size_t stride, size_t offset)
{
  glEnableVertexAttribArray(location);

#if WENDY_DEBUG
  checkGL("Failed to enable attribute \'%s\' of program \'%s\'",
          name.c_str(),
          program->getPath().asString().c_str());
#endif

  glVertexAttribPointer(location,
                        getElementCount(type),
                        getAttributeBaseType(type),
                        GL_FALSE,
                        stride,
                        (const void*) offset);

#if WENDY_DEBUG
  checkGL("Failed to set attribute \'%s\' of program \'%s\'",
          name.c_str(),
          program->getPath().asString().c_str());
#endif
}

Program& Attribute::getProgram(void) const
{
  return *program;
}

Attribute::Attribute(Program& initProgram):
  program(&initProgram)
{
}

///////////////////////////////////////////////////////////////////////

bool Uniform::operator == (const String& string) const
{
  return name == string;
}

bool Uniform::isScalar(void) const
{
  return type == FLOAT;
}

bool Uniform::isVector(void) const
{
  if (type == FLOAT_VEC2 || type == FLOAT_VEC3 || type == FLOAT_VEC4)
    return true;

  return false;
}

bool Uniform::isMatrix(void) const
{
  if (type == FLOAT_MAT2 || type == FLOAT_MAT3 || type == FLOAT_MAT4)
    return true;

  return false;
}

bool Uniform::isSampler(void) const
{
  if (type == SAMPLER_1D || type == SAMPLER_2D || type == SAMPLER_3D)
    return true;

  if (type == SAMPLER_RECT || type == SAMPLER_CUBE)
    return true;

  return false;
}

Uniform::Type Uniform::getType(void) const
{
  return type;
}

const String& Uniform::getName(void) const
{
  return name;
}

Program& Uniform::getProgram(void) const
{
  return *program;
}

Uniform::Uniform(Program& initProgram):
  program(&initProgram)
{
}

///////////////////////////////////////////////////////////////////////

Program::~Program(void)
{
  if (vertexShaderID)
    glDeleteShader(vertexShaderID);

  if (fragmentShaderID)
    glDeleteShader(fragmentShaderID);

  if (programID)
    glDeleteProgram(programID);
}

Attribute* Program::findAttribute(const String& name)
{
  AttributeList::iterator i = std::find(attributes.begin(), attributes.end(), name);
  if (i == attributes.end())
    return NULL;

  return &(*i);
}

const Attribute* Program::findAttribute(const String& name) const
{
  AttributeList::const_iterator i = std::find(attributes.begin(), attributes.end(), name);
  if (i == attributes.end())
    return NULL;

  return &(*i);
}

Uniform* Program::findUniform(const String& name)
{
  UniformList::iterator i = std::find(uniforms.begin(), uniforms.end(), name);
  if (i == uniforms.end())
    return NULL;

  return &(*i);
}

const Uniform* Program::findUniform(const String& name) const
{
  UniformList::const_iterator i = std::find(uniforms.begin(), uniforms.end(), name);
  if (i == uniforms.end())
    return NULL;

  return &(*i);
}

bool Program::isCurrent(void) const
{
  return context.getCurrentProgram() == this;
}

unsigned int Program::getAttributeCount(void) const
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

unsigned int Program::getUniformCount(void) const
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

Context& Program::getContext(void) const
{
  return context;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader)
{
  Ref<Program> program(new Program(info, context));
  if (!program->init(vertexShader, fragmentShader))
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
  context(initContext)
{
}

Program::Program(const Program& source):
  Resource(source),
  context(source.context),
  vertexShaderID(0),
  fragmentShaderID(0),
  programID(0)
{
}

bool Program::init(const Shader& vertexShader, const Shader& fragmentShader)
{
  vertexShaderID = createShader(GL_VERTEX_SHADER, vertexShader);
  if (!vertexShaderID)
    return false;

  fragmentShaderID = createShader(GL_FRAGMENT_SHADER, fragmentShader);
  if (!fragmentShaderID)
    return false;

  programID = glCreateProgram();

  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);

  int status;

  glLinkProgram(programID);

  glGetProgramiv(programID, GL_LINK_STATUS, &status);
  if (!status)
  {
    String infoLog = getProgramInfoLog(programID);
    logError("Failed to link program \'%s\':\n%s",
             path.asString().c_str(),
             infoLog.c_str());

    return false;
  }

  glValidateProgram(programID);

  glGetProgramiv(programID, GL_VALIDATE_STATUS, &status);
  if (!status)
  {
    String infoLog = getProgramInfoLog(programID);
    logError("Failed to validate program \'%s\':\n%s",
             path.asString().c_str(),
             infoLog.c_str());

    return false;
  }

  if (!checkGL("Failed to create object for program \'%s\'",
               getPath().asString().c_str()))
  {
    return false;
  }

  if (!createUniforms())
    return false;

  if (!createAttributes())
    return false;

  return true;
}

bool Program::createUniforms(void)
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
      continue;

    if (!isSupportedUniformType(uniformType))
    {
      logWarning("Skipping uniform \'%s\' of unsupported type", uniformName);
      continue;
    }

    uniforms.push_back(Uniform(*this));
    Uniform& uniform = uniforms.back();
    uniform.name = uniformName;
    uniform.type = convertUniformType(uniformType);
    uniform.location = glGetUniformLocation(programID, uniformName);
  }

  delete [] uniformName;

  if (!checkGL("Failed to retrieve uniforms for program \'%s\'",
               getPath().asString().c_str()))
  {
    return false;
  }

  return true;
}

bool Program::createAttributes(void)
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

    attributes.push_back(Attribute(*this));
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

Program& Program::operator = (const Program& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

void ProgramInterface::addUniform(const String& name, Uniform::Type type)
{
  uniforms.push_back(UniformList::value_type(name, type));
}

void ProgramInterface::addAttribute(const String& name, Attribute::Type type)
{
  attributes.push_back(AttributeList::value_type(name, type));
}

bool ProgramInterface::matches(const Program& program, bool verbose) const
{
  for (size_t i = 0;  i < uniforms.size();  i++)
  {
    const UniformList::value_type& entry = uniforms[i];

    const Uniform* uniform = program.findUniform(entry.first);
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
                 getTypeName(entry.second));
      }

      return false;
    }
  }

  for (size_t i = 0;  i < attributes.size();  i++)
  {
    const AttributeList::value_type& entry = attributes[i];

    const Attribute* attribute = program.findAttribute(entry.first);
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
                 getTypeName(entry.second));
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

    const VertexComponent* component = format.findComponent(entry.first);
    if (!component)
      return false;

    if (component->getType() != VertexComponent::FLOAT32)
      return false;

    if ((component->getElementCount() == 1 && entry.second != Attribute::FLOAT) ||
        (component->getElementCount() == 2 && entry.second != Attribute::FLOAT_VEC2) ||
        (component->getElementCount() == 3 && entry.second != Attribute::FLOAT_VEC3) ||
        (component->getElementCount() == 4 && entry.second != Attribute::FLOAT_VEC4))
    {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

ProgramReader::ProgramReader(Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext),
  info(initContext.getIndex())
{
}

Ref<Program> ProgramReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Program*>(cache);

  info.path = path;

  std::ifstream stream;
  if (!getIndex().openFile(stream, info.path))
    return NULL;

  if (!XML::Reader::read(stream))
  {
    vertexShader = NULL;
    fragmentShader = NULL;
    program = NULL;
    return NULL;
  }

  vertexShader = NULL;
  fragmentShader = NULL;

  if (!program)
  {
    logError("No shader program specification found in file");
    return NULL;
  }

  return program.detachObject();
}

bool ProgramReader::onBeginElement(const String& name)
{
  if (name == "program")
  {
    if (program)
    {
      logError("Only one shader program per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != PROGRAM_XML_VERSION)
    {
      logError("Shader program XML format version mismatch");
      return false;
    }

    return true;
  }

  if (name == "vertex")
  {
    if (vertexShader)
    {
      logError("Program specification \'%s\' contains more than one vertex program",
               info.path.asString().c_str());
      return false;
    }

    Path path(readString("path"));
    if (path.isEmpty())
    {
      logError("Vertex program path in shader program \'%s\' is empty",
               info.path.asString().c_str());
      return true;
    }

    String text;
    if (!readTextFile(getIndex(), text, path))
    {
      logError("Cannot find vertex program \'%s\' for shader program \'%s\'",
               path.asString().c_str(),
               info.path.asString().c_str());
      return false;
    }

    vertexShader = new Shader(text, path);
    return true;
  }

  if (name == "fragment")
  {
    if (fragmentShader)
    {
      logError("Program specification \'%s\' contains more than one fragment shader",
               info.path.asString().c_str());
      return false;
    }

    Path path(readString("path"));
    if (path.isEmpty())
    {
      logError("Fragment shader path in program \'%s\' is empty",
               info.path.asString().c_str());
      return true;
    }

    String text;
    if (!readTextFile(getIndex(), text, path))
    {
      logError("Cannot find fragment shader \'%s\' for program \'%s\'",
               path.asString().c_str(),
               info.path.asString().c_str());
      return false;
    }

    fragmentShader = new Shader(text, path);
    return true;
  }

  return true;
}

bool ProgramReader::onEndElement(const String& name)
{
  if (name == "program")
  {
    if (!vertexShader)
    {
      logError("Vertex shader missing for program \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    if (!fragmentShader)
    {
      logError("Fragment shader missing for program \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    program = Program::create(info, context, *vertexShader, *fragmentShader);
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

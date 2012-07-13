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

#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.h>
#include <internal/GLParser.h>

#include <algorithm>

#include <cstring>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLenum getElementType(AttributeType type)
{
  switch (type)
  {
    case ATTRIBUTE_FLOAT:
    case ATTRIBUTE_VEC2:
    case ATTRIBUTE_VEC3:
    case ATTRIBUTE_VEC4:
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

AttributeType convertAttributeType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
      return ATTRIBUTE_FLOAT;
    case GL_FLOAT_VEC2:
      return ATTRIBUTE_VEC2;
    case GL_FLOAT_VEC3:
      return ATTRIBUTE_VEC3;
    case GL_FLOAT_VEC4:
      return ATTRIBUTE_VEC4;
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

SamplerType convertSamplerType(GLenum type)
{
  switch (type)
  {
    case GL_SAMPLER_1D:
      return SAMPLER_1D;
    case GL_SAMPLER_2D:
      return SAMPLER_2D;
    case GL_SAMPLER_3D:
      return SAMPLER_3D;
    case GL_SAMPLER_2D_RECT_ARB:
      return SAMPLER_RECT;
    case GL_SAMPLER_CUBE:
      return SAMPLER_CUBE;
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

const char* getTypeName(ShaderType type)
{
  switch (type)
  {
    case VERTEX_SHADER:
      return "vertex";
    case FRAGMENT_SHADER:
      return "fragment";
  }

  panic("Invalid shader type %i", type);
}

UniformType convertUniformType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
      return UNIFORM_FLOAT;
    case GL_FLOAT_VEC2:
      return UNIFORM_VEC2;
    case GL_FLOAT_VEC3:
      return UNIFORM_VEC3;
    case GL_FLOAT_VEC4:

      return UNIFORM_VEC4;
    case GL_FLOAT_MAT2:
      return UNIFORM_MAT2;
    case GL_FLOAT_MAT3:
      return UNIFORM_MAT3;
    case GL_FLOAT_MAT4:
      return UNIFORM_MAT4;
  }

  panic("Unsupported GLSL uniform type %u", type);
}

GLenum convertToGL(ShaderType type)
{
  switch (type)
  {
    case VERTEX_SHADER:
      return GL_VERTEX_SHADER;
    case FRAGMENT_SHADER:
      return GL_FRAGMENT_SHADER;
  }

  panic("Invalid GLSL shader type %i", type);
}

String asName(const ShaderDefines& defines)
{
  String name;

  for (auto d = defines.begin();  d != defines.end();  d++)
  {
    name += " ";
    name += d->first;
    name += ":";
    name += d->second;
  }

  return name;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Shader::~Shader()
{
  if (shaderID)
    glDeleteShader(shaderID);
}

bool Shader::isVertexShader() const
{
  return type == VERTEX_SHADER;
}

bool Shader::isFragmentShader() const
{
  return type == FRAGMENT_SHADER;
}

ShaderType Shader::getType() const
{
  return type;
}

Context& Shader::getContext() const
{
  return context;
}

Ref<Shader> Shader::create(const ResourceInfo& info,
                          Context& context,
                          ShaderType type,
                          const String& text,
                          const ShaderDefines& defines)
{
  Ref<Shader> shader(new Shader(info, context, type));
  if (!shader->init(text, defines))
    return NULL;

  return shader;
}

Ref<Shader> Shader::read(Context& context,
                        ShaderType type,
                        const String& textName,
                        const ShaderDefines& defines)
{
  ResourceCache& cache = context.getCache();

  String name;
  name += textName;
  name += asName(defines);

  if (Ref<Shader> shader = cache.find<Shader>(name))
    return shader;

  const Path path = cache.findFile(textName);
  if (path.isEmpty())
  {
    logError("Failed to find shader \'%s\'", textName.c_str());
    return NULL;
  }

  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open shader file \'%s\'", path.asString().c_str());
    return NULL;
  }

  String text;

  stream.seekg(0, std::ios::end);
  text.resize((uint) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());

  return create(ResourceInfo(cache, name, path), context, type, text, defines);
}

Shader::Shader(const ResourceInfo& info,
               Context& initContext,
               ShaderType initType):
  Resource(info),
  context(initContext),
  type(initType),
  shaderID(0)
{
}

bool Shader::init(const String& text, const ShaderDefines& defines)
{
  Preprocessor spp(getCache());

  try
  {
    spp.parse(getName().c_str(), text.c_str());
  }
  catch (Exception& e)
  {
    logError("Failed to preprocess shader: %s", e.what());
    return false;
  }

  String shader;

  if (spp.hasVersion())
  {
    shader += "#version ";
    shader += spp.getVersion();
    shader += "\n";
  }

  for (auto d = defines.begin();  d != defines.end();  d++)
  {
    shader += "#define ";
    shader += d->first;
    shader += " ";
    shader += d->second;
    shader += "\n";
  }

  shader += "#line 0 0 /*shared program state*/\n";
  shader += context.getSharedProgramStateDeclaration();
  shader += spp.getOutput();

  GLsizei lengths[1];
  const GLchar* strings[1];

  lengths[0] = shader.length();
  strings[0] = (const GLchar*) shader.c_str();

  shaderID = glCreateShader(convertToGL(type));
  glShaderSource(shaderID, 1, strings, lengths);
  glCompileShader(shaderID);

  String infoLog;

  // Retrieve shader info log
  {
    GLint infoLogLength;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 1)
    {
      infoLog.resize(infoLogLength);
      glGetShaderInfoLog(shaderID, infoLogLength, NULL, &infoLog[0]);
    }
  }

  // Check shader compilation status
  {
    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (status)
    {
      if (!infoLog.empty())
      {
        logWarning("Warning(s) compiling shader \'%s\':\n%s%s",
                  getName().c_str(),
                  spp.getNameList().c_str(),
                  infoLog.c_str());
      }
    }
    else
    {
      if (infoLog.empty())
        checkGL("Failed to compile shader \'%s\'", getName().c_str());
      else
      {
        logError("Failed to compile shader \'%s\':\n%s%s",
                getName().c_str(),
                spp.getNameList().c_str(),
                infoLog.c_str());
      }

      return false;
    }
  }

  if (!checkGL("Failed to create object for shader \'%s\'", getName().c_str()))
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

bool Attribute::operator == (const char* string) const
{
  return name == string;
}

bool Attribute::isScalar() const
{
  return type == ATTRIBUTE_FLOAT;
}

bool Attribute::isVector() const
{
  if (type == ATTRIBUTE_VEC2 || type == ATTRIBUTE_VEC3 || type == ATTRIBUTE_VEC4)
    return true;

  return false;
}

AttributeType Attribute::getType() const
{
  return type;
}

const String& Attribute::getName() const
{
  return name;
}

uint Attribute::getElementCount() const
{
  switch (type)
  {
    case ATTRIBUTE_FLOAT:
      return 1;
    case ATTRIBUTE_VEC2:
      return 2;
    case ATTRIBUTE_VEC3:
      return 3;
    case ATTRIBUTE_VEC4:
      return 4;
  }

  panic("Invalid GLSL attribute type %u", type);
}

void Attribute::bind(size_t stride, size_t offset)
{
  glVertexAttribPointer(location,
                        getElementCount(),
                        getElementType(type),
                        GL_FALSE,
                        stride,
                        (const void*) offset);

#if WENDY_DEBUG
  checkGL("Failed to set attribute \'%s\'", name.c_str());
#endif
}

const char* Attribute::getTypeName(AttributeType type)
{
  switch (type)
  {
    case ATTRIBUTE_FLOAT:
      return "float";
    case ATTRIBUTE_VEC2:
      return "vec2";
    case ATTRIBUTE_VEC3:
      return "vec3";
    case ATTRIBUTE_VEC4:
      return "vec4";
  }

  panic("Invalid GLSL attribute type %u", type);
}

///////////////////////////////////////////////////////////////////////

void Sampler::bind(uint unit)
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

SamplerType Sampler::getType() const
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

const char* Sampler::getTypeName(SamplerType type)
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
    case UNIFORM_FLOAT:
      glUniform1fv(location, 1, (const float*) data);
      break;
    case UNIFORM_VEC2:
      glUniform2fv(location, 1, (const float*) data);
      break;
    case UNIFORM_VEC3:
      glUniform3fv(location, 1, (const float*) data);
      break;
    case UNIFORM_VEC4:
      glUniform4fv(location, 1, (const float*) data);
      break;
    case UNIFORM_MAT2:
      glUniformMatrix2fv(location, 1, GL_FALSE, (const float*) data);
      break;
    case UNIFORM_MAT3:
      glUniformMatrix3fv(location, 1, GL_FALSE, (const float*) data);
      break;
    case UNIFORM_MAT4:
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
  return type == UNIFORM_FLOAT;
}

bool Uniform::isVector() const
{
  if (type == UNIFORM_VEC2 || type == UNIFORM_VEC3 || type == UNIFORM_VEC4)
    return true;

  return false;
}

bool Uniform::isMatrix() const
{
  if (type == UNIFORM_MAT2 || type == UNIFORM_MAT3 || type == UNIFORM_MAT4)
    return true;

  return false;
}

UniformType Uniform::getType() const
{
  return type;
}

const String& Uniform::getName() const
{
  return name;
}

uint Uniform::getElementCount() const
{
  switch (type)
  {
    case UNIFORM_FLOAT:
      return 1;
    case UNIFORM_VEC2:
      return 2;
    case UNIFORM_VEC3:
      return 3;
    case UNIFORM_VEC4:
      return 4;
    case UNIFORM_MAT2:
      return 2 * 2;
    case UNIFORM_MAT3:
      return 3 * 3;
    case UNIFORM_MAT4:
      return 4 * 4;
  }

  panic("Invalid GLSL uniform type %u", type);
}

int Uniform::getSharedID() const
{
  return sharedID;
}

const char* Uniform::getTypeName(UniformType type)
{
  switch (type)
  {
    case UNIFORM_FLOAT:
      return "float";
    case UNIFORM_VEC2:
      return "vec2";
    case UNIFORM_VEC3:
      return "vec3";
    case UNIFORM_VEC4:
      return "vec4";
    case UNIFORM_MAT2:
      return "mat2";
    case UNIFORM_MAT3:
      return "mat3";
    case UNIFORM_MAT4:
      return "mat4";
  }

  panic("Invalid GLSL uniform type %u", type);
}

///////////////////////////////////////////////////////////////////////

Program::~Program()
{
  if (programID)
    glDeleteProgram(programID);

  if (Stats* stats = context.getStats())
    stats->removeProgram();
}

Attribute* Program::findAttribute(const char* name)
{
  auto a = std::find(attributes.begin(), attributes.end(), name);
  if (a == attributes.end())
    return NULL;

  return &(*a);
}

const Attribute* Program::findAttribute(const char* name) const
{
  auto a = std::find(attributes.begin(), attributes.end(), name);
  if (a == attributes.end())
    return NULL;

  return &(*a);
}

Sampler* Program::findSampler(const char* name)
{
  auto s = std::find(samplers.begin(), samplers.end(), name);
  if (s == samplers.end())
    return NULL;

  return &(*s);
}

const Sampler* Program::findSampler(const char* name) const
{
  auto s = std::find(samplers.begin(), samplers.end(), name);
  if (s == samplers.end())
    return NULL;

  return &(*s);
}

Uniform* Program::findUniform(const char* name)
{
  auto u = std::find(uniforms.begin(), uniforms.end(), name);
  if (u == uniforms.end())
    return NULL;

  return &(*u);
}

const Uniform* Program::findUniform(const char* name) const
{
  auto u = std::find(uniforms.begin(), uniforms.end(), name);
  if (u == uniforms.end())
    return NULL;

  return &(*u);
}

uint Program::getAttributeCount() const
{
  return attributes.size();
}

Attribute& Program::getAttribute(uint index)
{
  return attributes[index];
}

const Attribute& Program::getAttribute(uint index) const
{
  return attributes[index];
}

uint Program::getSamplerCount() const
{
  return samplers.size();
}

Sampler& Program::getSampler(uint index)
{
  return samplers[index];
}

const Sampler& Program::getSampler(uint index) const
{
  return samplers[index];
}

uint Program::getUniformCount() const
{
  return uniforms.size();
}

Uniform& Program::getUniform(uint index)
{
  return uniforms[index];
}

const Uniform& Program::getUniform(uint index) const
{
  return uniforms[index];
}

Context& Program::getContext() const
{
  return context;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             Shader& vertexShader,
                             Shader& fragmentShader)
{
  Ref<Program> program(new Program(info, context));
  if (!program->init(vertexShader, fragmentShader))
    return NULL;

  return program;
}

Ref<Program> Program::read(Context& context,
                           const String& vertexShaderName,
                           const String& fragmentShaderName,
                           const ShaderDefines& defines)
{
  ResourceCache& cache = context.getCache();

  String name;
  name += "vs:";
  name += vertexShaderName;
  name += " fs:";
  name += fragmentShaderName;
  name += asName(defines);

  if (Ref<Program> program = cache.find<Program>(name))
    return program;

  Ref<Shader> vertexShader = Shader::read(context,
                                          VERTEX_SHADER,
                                          vertexShaderName,
                                          defines);
  if (!vertexShader)
    return NULL;

  Ref<Shader> fragmentShader = Shader::read(context,
                                            FRAGMENT_SHADER,
                                            fragmentShaderName,
                                            defines);
  if (!fragmentShader)
    return NULL;

  return create(ResourceInfo(cache, name),
                context,
                *vertexShader,
                *fragmentShader);
}

Program::Program(const ResourceInfo& info, Context& initContext):
  Resource(info),
  context(initContext),
  programID(0)
{
  if (Stats* stats = context.getStats())
    stats->addProgram();
}

Program::Program(const Program& source):
  Resource(source),
  context(source.context)
{
  panic("GLSL programs may not be copied");
}

bool Program::init(Shader& initVertexShader, Shader& initFragmentShader)
{
  vertexShader = &initVertexShader;
  fragmentShader = &initFragmentShader;

  if (!vertexShader->isVertexShader())
  {
    logError("Shader \'%s\' for program \'%s\' is not a vertex shader",
             vertexShader->getName().c_str(),
             getName().c_str());
    return false;
  }

  if (!fragmentShader->isFragmentShader())
  {
    logError("Shader \'%s\' for program \'%s\' is not a fragment shader",
             fragmentShader->getName().c_str(),
             getName().c_str());
    return false;
  }

  programID = glCreateProgram();
  if (!programID)
  {
    logError("Failed to create OpenGL object for program \'%s\'",
             getName().c_str());
    return false;
  }

  glAttachShader(programID, vertexShader->shaderID);
  glAttachShader(programID, fragmentShader->shaderID);

  glLinkProgram(programID);

  const String infoLog = getInfoLog();

  int status;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);

  if (!status)
  {
    logError("Failed to link program \'%s\':\n%s",
             getName().c_str(),
             infoLog.c_str());
    return false;
  }

  if (infoLog.length() > 1)
  {
    logWarning("Warning(s) when linking program \'%s\':\n%s",
               getName().c_str(),
               infoLog.c_str());
  }

  if (!checkGL("Failed to create object for program \'%s\'", getName().c_str()))
    return false;

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
                 getName().c_str(),
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
               getName().c_str()))
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
               getName().c_str()))
  {
    return false;
  }

  return true;
}

void Program::bind()
{
  glUseProgram(programID);

  for (auto a = attributes.begin();  a != attributes.end();  a++)
    glEnableVertexAttribArray(a->location);
}

void Program::unbind()
{
  for (auto a = attributes.begin();  a != attributes.end();  a++)
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
    const String infoLog = getInfoLog();
    logError("Failed to validate program \'%s\':\n%s",
             getName().c_str(),
             infoLog.c_str());

    return false;
  }

  return true;
}

String Program::getInfoLog() const
{
  String infoLog;

  GLint infoLogLength;
  glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

  if (infoLogLength > 1)
  {
    infoLog.resize(infoLogLength);
    glGetProgramInfoLog(programID, infoLogLength, NULL, &infoLog[0]);
  }

  return infoLog;
}

///////////////////////////////////////////////////////////////////////

void ProgramInterface::addSampler(const char* name, SamplerType type)
{
  samplers.push_back(std::make_pair(name, type));
}

void ProgramInterface::addUniform(const char* name, UniformType type)
{
  uniforms.push_back(std::make_pair(name, type));
}

void ProgramInterface::addAttribute(const char* name, AttributeType type)
{
  attributes.push_back(std::make_pair(name, type));
}

void ProgramInterface::addAttributes(const VertexFormat& format)
{
  for (size_t i = 0;  i < format.getComponentCount();  i++)
  {
    AttributeType type;

    switch (format[i].getElementCount())
    {
      case 1:
        type = ATTRIBUTE_FLOAT;
        break;
      case 2:
        type = ATTRIBUTE_VEC2;
        break;
      case 3:
        type = ATTRIBUTE_VEC3;
        break;
      case 4:
        type = ATTRIBUTE_VEC4;
        break;
      default:
        panic("Invalid vertex format component element count");
    }

    addAttribute(format[i].getName().c_str(), type);
  }
}

bool ProgramInterface::matches(const Program& program, bool verbose) const
{
  for (auto s = samplers.begin();  s != samplers.end();  s++)
  {
    const Sampler* sampler = program.findSampler(s->first.c_str());
    if (!sampler)
    {
      if (verbose)
      {
        logError("Sampler \'%s\' missing in program \'%s\'",
                 s->first.c_str(),
                 program.getName().c_str());
      }

      return false;
    }

    if (sampler->getType() != s->second)
    {
      if (verbose)
      {
        logError("Sampler \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
                 s->first.c_str(),
                 program.getName().c_str(),
                 Sampler::getTypeName(s->second));
      }

      return false;
    }
  }

  for (auto u = uniforms.begin();  u != uniforms.end();  u++)
  {
    const Uniform* uniform = program.findUniform(u->first.c_str());
    if (!uniform)
    {
      if (verbose)
      {
        logError("Uniform \'%s\' missing in program \'%s\'",
                 u->first.c_str(),
                 program.getName().c_str());
      }

      return false;
    }

    if (uniform->getType() != u->second)
    {
      if (verbose)
      {
        logError("Uniform \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
                 u->first.c_str(),
                 program.getName().c_str(),
                 Uniform::getTypeName(u->second));
      }

      return false;
    }
  }

  for (size_t i = 0;  i < program.getAttributeCount();  i++)
  {
    const Attribute& attribute = program.getAttribute(i);

    size_t index;

    for (index = 0;  index < attributes.size();  index++)
    {
      if (attributes[index].first == attribute.getName())
        break;
    }

    if (index == attributes.size())
    {
      if (verbose)
      {
        logError("Attribute \'%s\' is not provided to program \'%s\'",
                 attribute.getName().c_str(),
                 program.getName().c_str());
      }

      return false;
    }

    if (attribute.getType() != attributes[index].second)
    {
      if (verbose)
      {
        logError("Attribute \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
                attributes[index].first.c_str(),
                program.getName().c_str(),
                Attribute::getTypeName(attributes[index].second));
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

  for (auto a = attributes.begin();  a != attributes.end();  a++)
  {
    const VertexComponent* component = format.findComponent(a->first.c_str());
    if (!component)
      return false;

    if (component->getType() != VertexComponent::FLOAT32)
      return false;

    if ((component->getElementCount() == 1 && a->second != ATTRIBUTE_FLOAT) ||
        (component->getElementCount() == 2 && a->second != ATTRIBUTE_VEC2) ||
        (component->getElementCount() == 3 && a->second != ATTRIBUTE_VEC3) ||
        (component->getElementCount() == 4 && a->second != ATTRIBUTE_VEC4))
    {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

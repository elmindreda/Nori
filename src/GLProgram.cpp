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

GLenum elementTypeOf(AttributeType type)
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

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Shader::~Shader()
{
  if (m_shaderID)
    glDeleteShader(m_shaderID);
}

Ref<Shader> Shader::create(const ResourceInfo& info,
                          Context& context,
                          ShaderType type,
                          const String& text)
{
  Ref<Shader> shader(new Shader(info, context, type));
  if (!shader->init(text))
    return NULL;

  return shader;
}

Ref<Shader> Shader::read(Context& context,
                        ShaderType type,
                        const String& name)
{
  ResourceCache& cache = context.cache();

  if (Ref<Shader> shader = cache.find<Shader>(name))
    return shader;

  const Path path = cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find shader %s", name.c_str());
    return NULL;
  }

  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open shader file %s", path.asString().c_str());
    return NULL;
  }

  String text;

  stream.seekg(0, std::ios::end);
  text.resize((uint) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());

  return create(ResourceInfo(cache, name, path), context, type, text);
}

Shader::Shader(const ResourceInfo& info,
               Context& context,
               ShaderType type):
  Resource(info),
  m_context(context),
  m_type(type),
  m_shaderID(0)
{
}

bool Shader::init(const String& text)
{
  Preprocessor spp(cache());

  try
  {
    spp.parse(name().c_str(), text.c_str());
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

  shader += "#line 0 0 /*shared program state*/\n";
  shader += m_context.sharedProgramStateDeclaration();
  shader += spp.getOutput();

  GLsizei lengths[1];
  const GLchar* strings[1];

  lengths[0] = shader.length();
  strings[0] = (const GLchar*) shader.c_str();

  m_shaderID = glCreateShader(convertToGL(m_type));
  glShaderSource(m_shaderID, 1, strings, lengths);
  glCompileShader(m_shaderID);

  String infoLog;

  // Retrieve shader info log
  {
    GLint infoLogLength;
    glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 1)
    {
      infoLog.resize(infoLogLength);
      glGetShaderInfoLog(m_shaderID, infoLogLength, NULL, &infoLog[0]);
    }
  }

  // Check shader compilation status
  {
    GLint status;
    glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &status);
    if (status)
    {
      if (!infoLog.empty())
      {
        logWarning("Warning(s) compiling shader %s:\n%s%s",
                   name().c_str(),
                   spp.getNameList().c_str(),
                   infoLog.c_str());
      }
    }
    else
    {
      if (infoLog.empty())
        checkGL("Failed to compile shader %s", name().c_str());
      else
      {
        logError("Failed to compile shader %s:\n%s%s",
                 name().c_str(),
                 spp.getNameList().c_str(),
                 infoLog.c_str());
      }

      return false;
    }
  }

  if (!checkGL("Failed to create object for shader %s", name().c_str()))
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

bool Attribute::isVector() const
{
  return m_type == ATTRIBUTE_VEC2 ||
         m_type == ATTRIBUTE_VEC3 ||
         m_type == ATTRIBUTE_VEC4;
}

uint Attribute::elementCount() const
{
  switch (m_type)
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

  panic("Invalid GLSL attribute type %u", m_type);
}

void Attribute::bind(size_t stride, size_t offset)
{
  glVertexAttribPointer(m_location,
                        elementCount(),
                        elementTypeOf(m_type),
                        GL_FALSE,
                        stride,
                        (const void*) offset);

#if WENDY_DEBUG
  checkGL("Failed to set attribute %s", m_name.c_str());
#endif
}

const char* Attribute::typeName(AttributeType type)
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
  glUniform1i(m_location, unit);

#if WENDY_DEBUG
  checkGL("Failed to set sampler %s", m_name.c_str());
#endif
}

bool Sampler::isShared() const
{
  return m_sharedID != INVALID_SHARED_STATE_ID;
}

const char* Sampler::typeName(SamplerType type)
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
  switch (m_type)
  {
    case UNIFORM_FLOAT:
      glUniform1fv(m_location, 1, (const float*) data);
      break;
    case UNIFORM_VEC2:
      glUniform2fv(m_location, 1, (const float*) data);
      break;
    case UNIFORM_VEC3:
      glUniform3fv(m_location, 1, (const float*) data);
      break;
    case UNIFORM_VEC4:
      glUniform4fv(m_location, 1, (const float*) data);
      break;
    case UNIFORM_MAT2:
      glUniformMatrix2fv(m_location, 1, GL_FALSE, (const float*) data);
      break;
    case UNIFORM_MAT3:
      glUniformMatrix3fv(m_location, 1, GL_FALSE, (const float*) data);
      break;
    case UNIFORM_MAT4:
      glUniformMatrix4fv(m_location, 1, GL_FALSE, (const float*) data);
      break;
  }

#if WENDY_DEBUG
  checkGL("Failed to set uniform %s", m_name.c_str());
#endif
}

bool Uniform::isShared() const
{
  return m_sharedID != INVALID_SHARED_STATE_ID;
}

bool Uniform::isVector() const
{
  return m_type == UNIFORM_VEC2 ||
         m_type == UNIFORM_VEC3 ||
         m_type == UNIFORM_VEC4;
}

bool Uniform::isMatrix() const
{
  return m_type == UNIFORM_MAT2 ||
         m_type == UNIFORM_MAT3 ||
         m_type == UNIFORM_MAT4;
}

uint Uniform::elementCount() const
{
  switch (m_type)
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

  panic("Invalid GLSL uniform type %u", m_type);
}

const char* Uniform::typeName(UniformType type)
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
  if (m_programID)
    glDeleteProgram(m_programID);

  if (Stats* stats = m_context.stats())
    stats->removeProgram();
}

Attribute* Program::findAttribute(const char* name)
{
  auto a = std::find(m_attributes.begin(), m_attributes.end(), name);
  if (a == m_attributes.end())
    return NULL;

  return &(*a);
}

const Attribute* Program::findAttribute(const char* name) const
{
  auto a = std::find(m_attributes.begin(), m_attributes.end(), name);
  if (a == m_attributes.end())
    return NULL;

  return &(*a);
}

Sampler* Program::findSampler(const char* name)
{
  auto s = std::find(m_samplers.begin(), m_samplers.end(), name);
  if (s == m_samplers.end())
    return NULL;

  return &(*s);
}

const Sampler* Program::findSampler(const char* name) const
{
  auto s = std::find(m_samplers.begin(), m_samplers.end(), name);
  if (s == m_samplers.end())
    return NULL;

  return &(*s);
}

Uniform* Program::findUniform(const char* name)
{
  auto u = std::find(m_uniforms.begin(), m_uniforms.end(), name);
  if (u == m_uniforms.end())
    return NULL;

  return &(*u);
}

const Uniform* Program::findUniform(const char* name) const
{
  auto u = std::find(m_uniforms.begin(), m_uniforms.end(), name);
  if (u == m_uniforms.end())
    return NULL;

  return &(*u);
}

uint Program::attributeCount() const
{
  return m_attributes.size();
}

Attribute& Program::attribute(uint index)
{
  return m_attributes[index];
}

const Attribute& Program::attribute(uint index) const
{
  return m_attributes[index];
}

uint Program::samplerCount() const
{
  return m_samplers.size();
}

Sampler& Program::sampler(uint index)
{
  return m_samplers[index];
}

const Sampler& Program::sampler(uint index) const
{
  return m_samplers[index];
}

uint Program::uniformCount() const
{
  return m_uniforms.size();
}

Uniform& Program::uniform(uint index)
{
  return m_uniforms[index];
}

const Uniform& Program::uniform(uint index) const
{
  return m_uniforms[index];
}

Context& Program::context() const
{
  return m_context;
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
                           const String& fragmentShaderName)
{
  ResourceCache& cache = context.cache();

  String name;
  name += "vs:";
  name += vertexShaderName;
  name += " fs:";
  name += fragmentShaderName;

  if (Ref<Program> program = cache.find<Program>(name))
    return program;

  Ref<Shader> vertexShader = Shader::read(context,
                                          VERTEX_SHADER,
                                          vertexShaderName);
  if (!vertexShader)
    return NULL;

  Ref<Shader> fragmentShader = Shader::read(context,
                                            FRAGMENT_SHADER,
                                            fragmentShaderName);
  if (!fragmentShader)
    return NULL;

  return create(ResourceInfo(cache, name),
                context,
                *vertexShader,
                *fragmentShader);
}

Program::Program(const ResourceInfo& info, Context& context):
  Resource(info),
  m_context(context),
  m_programID(0)
{
  if (Stats* stats = m_context.stats())
    stats->addProgram();
}

Program::Program(const Program& source):
  Resource(source),
  m_context(source.m_context)
{
  panic("GLSL programs may not be copied");
}

bool Program::init(Shader& vertexShader, Shader& fragmentShader)
{
  m_vertexShader = &vertexShader;
  m_fragmentShader = &fragmentShader;

  if (!m_vertexShader->isVertexShader())
  {
    logError("Shader %s for program %s is not a vertex shader",
             m_vertexShader->name().c_str(),
             name().c_str());
    return false;
  }

  if (!m_fragmentShader->isFragmentShader())
  {
    logError("Shader %s for program %s is not a fragment shader",
             m_fragmentShader->name().c_str(),
             name().c_str());
    return false;
  }

  m_programID = glCreateProgram();
  if (!m_programID)
  {
    logError("Failed to create OpenGL object for program %s", name().c_str());
    return false;
  }

  glAttachShader(m_programID, m_vertexShader->m_shaderID);
  glAttachShader(m_programID, m_fragmentShader->m_shaderID);

  glLinkProgram(m_programID);

  const String info = infoLog();

  int status;
  glGetProgramiv(m_programID, GL_LINK_STATUS, &status);

  if (!status)
  {
    logError("Failed to link program %s:\n%s", name().c_str(), info.c_str());
    return false;
  }

  if (info.length() > 1)
  {
    logWarning("Warning(s) when linking program %s:\n%s",
               name().c_str(),
               info.c_str());
  }

  if (!checkGL("Failed to create object for program %s", name().c_str()))
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
  glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &uniformCount);

  m_uniforms.reserve(uniformCount);

  GLint maxNameLength;
  glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

  char* uniformName = new char [maxNameLength + 1];

  for (int i = 0;  i < uniformCount;  i++)
  {
    GLsizei nameLength;
    GLint uniformSize;
    GLenum uniformType;

    glGetActiveUniform(m_programID,
                       i,
                       maxNameLength + 1,
                       &nameLength,
                       &uniformSize,
                       &uniformType,
                       uniformName);

    if (std::strncmp(uniformName, "gl_", 3) == 0)
    {
      logWarning("Program %s uses built-in uniform %s",
                 name().c_str(),
                 uniformName);
      continue;
    }

    if (isSupportedUniformType(uniformType))
    {
      m_uniforms.push_back(Uniform());
      Uniform& uniform = m_uniforms.back();
      uniform.m_name = uniformName;
      uniform.m_type = convertUniformType(uniformType);
      uniform.m_location = glGetUniformLocation(m_programID, uniformName);
      uniform.m_sharedID = m_context.sharedUniformID(uniformName, uniform.type());
    }
    else if (isSupportedSamplerType(uniformType))
    {
      m_samplers.push_back(Sampler());
      Sampler& sampler = m_samplers.back();
      sampler.m_name = uniformName;
      sampler.m_type = convertSamplerType(uniformType);
      sampler.m_location = glGetUniformLocation(m_programID, uniformName);
      sampler.m_sharedID = m_context.sharedSamplerID(uniformName, sampler.type());
    }
    else
      logWarning("Skipping uniform %s of unsupported type", uniformName);
  }

  delete [] uniformName;

  if (!checkGL("Failed to retrieve uniforms for program %s", name().c_str()))
    return false;

  return true;
}

bool Program::retrieveAttributes()
{
  GLint attributeCount;
  glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTES, &attributeCount);

  m_attributes.reserve(attributeCount);

  GLint maxNameLength;
  glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);

  char* attributeName = new char [maxNameLength + 1];

  for (int i = 0;  i < attributeCount;  i++)
  {
    GLsizei nameLength;
    GLint attributeSize;
    GLenum attributeType;

    glGetActiveAttrib(m_programID,
                      i,
                      maxNameLength + 1,
                      &nameLength,
                      &attributeSize,
                      &attributeType,
                      attributeName);

    if (!isSupportedAttributeType(attributeType))
    {
      logWarning("Skipping attribute %s of unsupported type", attributeName);
      continue;
    }

    m_attributes.push_back(Attribute());
    Attribute& attribute = m_attributes.back();
    attribute.m_name = attributeName;
    attribute.m_type = convertAttributeType(attributeType);
    attribute.m_location = glGetAttribLocation(m_programID, attributeName);
  }

  delete [] attributeName;

  if (!checkGL("Failed to retrieve attributes for program %s", name().c_str()))
    return false;

  return true;
}

void Program::bind()
{
  glUseProgram(m_programID);

  for (auto& a : m_attributes)
    glEnableVertexAttribArray(a.m_location);
}

void Program::unbind()
{
  for (auto& a : m_attributes)
    glDisableVertexAttribArray(a.m_location);
}

Program& Program::operator = (const Program& source)
{
  return *this;
}

bool Program::isValid() const
{
  glValidateProgram(m_programID);

  int status;
  glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &status);
  if (!status)
  {
    logError("Failed to validate program %s:\n%s",
             name().c_str(),
             infoLog().c_str());

    return false;
  }

  return true;
}

String Program::infoLog() const
{
  String result;
  GLint length;

  glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &length);
  if (length > 1)
  {
    result.resize(length);
    glGetProgramInfoLog(m_programID, length, NULL, &result[0]);
  }

  return result;
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
  for (auto& c : format.components())
  {
    AttributeType type;

    switch (c.elementCount())
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

    addAttribute(c.name().c_str(), type);
  }
}

bool ProgramInterface::matches(const Program& program, bool verbose) const
{
  for (auto& s : samplers)
  {
    const Sampler* sampler = program.findSampler(s.first.c_str());
    if (!sampler)
    {
      if (verbose)
      {
        logError("Sampler %s missing in program %s",
                 s.first.c_str(),
                 program.name().c_str());
      }

      return false;
    }

    if (sampler->type() != s.second)
    {
      if (verbose)
      {
        logError("Sampler %s in program %s has incorrect type; should be %s",
                 s.first.c_str(),
                 program.name().c_str(),
                 Sampler::typeName(s.second));
      }

      return false;
    }
  }

  for (auto& u : uniforms)
  {
    const Uniform* uniform = program.findUniform(u.first.c_str());
    if (!uniform)
    {
      if (verbose)
      {
        logError("Uniform %s missing in program %s",
                 u.first.c_str(),
                 program.name().c_str());
      }

      return false;
    }

    if (uniform->type() != u.second)
    {
      if (verbose)
      {
        logError("Uniform %s in program %s has incorrect type; should be %s",
                 u.first.c_str(),
                 program.name().c_str(),
                 Uniform::typeName(u.second));
      }

      return false;
    }
  }

  for (size_t i = 0;  i < program.attributeCount();  i++)
  {
    const Attribute& attribute = program.attribute(i);

    size_t index;

    for (index = 0;  index < attributes.size();  index++)
    {
      if (attributes[index].first == attribute.name())
        break;
    }

    if (index == attributes.size())
    {
      if (verbose)
      {
        logError("Attribute %s is not provided to program %s",
                 attribute.name().c_str(),
                 program.name().c_str());
      }

      return false;
    }

    if (attribute.type() != attributes[index].second)
    {
      if (verbose)
      {
        logError("Attribute %s in program %s has incorrect type; should be %s",
                attributes[index].first.c_str(),
                program.name().c_str(),
                Attribute::typeName(attributes[index].second));
      }

      return false;
    }
  }

  return true;
}

bool ProgramInterface::matches(const VertexFormat& format, bool verbose) const
{
  if (format.components().size() != attributes.size())
    return false;

  for (auto& a : attributes)
  {
    const VertexComponent* component = format.findComponent(a.first.c_str());
    if (!component)
      return false;

    if ((component->elementCount() == 1 && a.second != ATTRIBUTE_FLOAT) ||
        (component->elementCount() == 2 && a.second != ATTRIBUTE_VEC2) ||
        (component->elementCount() == 3 && a.second != ATTRIBUTE_VEC3) ||
        (component->elementCount() == 4 && a.second != ATTRIBUTE_VEC4))
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

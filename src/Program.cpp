///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Texture.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/Program.hpp>
#include <nori/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/OpenGL.hpp>

#include <algorithm>
#include <fstream>

#include <cstring>

#include <pugixml.hpp>

namespace nori
{

namespace
{

const struct
{
  bool scalar;
  bool vector;
  bool matrix;
  bool sampler;
  uint elementCount;
  GLenum nativeType;
  const char* name;
} uniformTypes[] =
{
  { false, false, false, true,   1, GL_SAMPLER_1D,      "sampler1D" },
  { false, false, false, true,   1, GL_SAMPLER_2D,      "sampler2D" },
  { false, false, false, true,   1, GL_SAMPLER_3D,      "sampler3D" },
  { false, false, false, true,   1, GL_SAMPLER_2D_RECT, "sampler2DRect" },
  { false, false, false, true,   1, GL_SAMPLER_CUBE,    "samplerCube" },
  {  true, false, false, false,  1, GL_INT,             "int" },
  {  true, false, false, false,  1, GL_UNSIGNED_INT,    "unsigned int" },
  {  true, false, false, false,  1, GL_FLOAT,           "float" },
  { false,  true, false, false,  2, GL_FLOAT_VEC2,      "vec2" },
  { false,  true, false, false,  3, GL_FLOAT_VEC3,      "vec3" },
  { false,  true, false, false,  4, GL_FLOAT_VEC4,      "vec4" },
  { false, false,  true, false,  4, GL_FLOAT_MAT2,      "mat2" },
  { false, false,  true, false,  9, GL_FLOAT_MAT3,      "mat3" },
  { false, false,  true, false, 16, GL_FLOAT_MAT4,      "mat4" }
};

const struct
{
  bool scalar;
  bool vector;
  uint elementCount;
  GLenum elementType;
  GLenum nativeType;
  const char* name;
} attributeTypes[] =
{
  {  true, false, 1, GL_FLOAT, GL_FLOAT,      "float" },
  { false,  true, 2, GL_FLOAT, GL_FLOAT_VEC2, "vec2" },
  { false,  true, 3, GL_FLOAT, GL_FLOAT_VEC3, "vec3" },
  { false,  true, 4, GL_FLOAT, GL_FLOAT_VEC4, "vec4" },
};

AttributeType convertAttributeType(GLenum type)
{
  for (uint i = 0;  i < sizeof(attributeTypes) / sizeof(attributeTypes[0]);  i++)
  {
    if (attributeTypes[i].nativeType == type)
      return AttributeType(i);
  }

  return AttributeType(-1);
}

bool isSupportedAttributeType(GLenum type)
{
  return convertAttributeType(type) != -1;
}

UniformType convertUniformType(GLenum type)
{
  for (uint i = 0;  i < sizeof(uniformTypes) / sizeof(uniformTypes[0]);  i++)
  {
    if (uniformTypes[i].nativeType == type)
      return UniformType(i);
  }

  return UniformType(-1);
}

bool isSupportedUniformType(GLenum type)
{
  return convertUniformType(type) != -1;
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

Shader::~Shader()
{
  if (m_shaderID)
    glDeleteShader(m_shaderID);
}

Ref<Shader> Shader::create(const ResourceInfo& info,
                          RenderContext& context,
                          ShaderType type,
                          const std::string& text)
{
  Ref<Shader> shader(new Shader(info, context, type));
  if (!shader->init(text))
    return nullptr;

  return shader;
}

Ref<Shader> Shader::read(RenderContext& context,
                         ShaderType type,
                         const std::string& name)
{
  ResourceCache& cache = context.cache();

  if (Ref<Shader> shader = cache.find<Shader>(name))
    return shader;

  const Path path = cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find shader %s", name.c_str());
    return nullptr;
  }

  std::ifstream stream(path.name());
  if (stream.fail())
  {
    logError("Failed to open shader file %s", path.name().c_str());
    return nullptr;
  }

  std::string text;

  stream.seekg(0, std::ios::end);
  text.resize((uint) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());

  return create(ResourceInfo(cache, name, path), context, type, text);
}

Shader::Shader(const ResourceInfo& info,
               RenderContext& context,
               ShaderType type):
  Resource(info),
  m_context(context),
  m_type(type),
  m_shaderID(0)
{
}

bool Shader::init(const std::string& text)
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

  std::string shader;

  if (spp.hasVersion())
  {
    shader += "#version ";
    shader += spp.version();
    shader += "\n";
  }

  shader += "#line 0 0 /*shared program state*/\n";
  shader += m_context.sharedProgramStateDeclaration();
  shader += spp.output();

  GLsizei lengths[1];
  const GLchar* strings[1];

  lengths[0] = (GLsizei) shader.length();
  strings[0] = (const GLchar*) shader.c_str();

  m_shaderID = glCreateShader(convertToGL(m_type));

  if (m_context.debug())
    glObjectLabel(GL_SHADER, m_shaderID, name().length(), name().c_str());

  glShaderSource(m_shaderID, 1, strings, lengths);
  glCompileShader(m_shaderID);

  std::string infoLog;

  // Retrieve shader info log
  {
    GLint infoLogLength;
    glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 1)
    {
      infoLog.resize(infoLogLength);
      glGetShaderInfoLog(m_shaderID, infoLogLength, nullptr, &infoLog[0]);
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
                   spp.names().c_str(),
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
                 spp.names().c_str(),
                 infoLog.c_str());
      }

      return false;
    }
  }

  if (!checkGL("Failed to create object for shader %s", name().c_str()))
    return false;

  return true;
}

bool Attribute::isScalar() const
{
  return attributeTypes[m_type].scalar;
}

bool Attribute::isVector() const
{
  return attributeTypes[m_type].vector;
}

uint Attribute::elementCount() const
{
  return attributeTypes[m_type].elementCount;
}

void Attribute::bind(size_t stride, size_t offset)
{
  glVertexAttribPointer(m_location,
                        attributeTypes[m_type].elementCount,
                        attributeTypes[m_type].elementType,
                        GL_FALSE,
                        (GLsizei) stride,
                        (const void*) offset);

#if NORI_DEBUG
  checkGL("Failed to set attribute %s", m_name.c_str());
#endif
}

void Uniform::copyFrom(const void* data)
{
  switch (m_type)
  {
    case UNIFORM_INT:
      glUniform1iv(m_location, 1, (const int*) data);
      break;
    case UNIFORM_UINT:
      glUniform1uiv(m_location, 1, (const uint*) data);
      break;
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

#if NORI_DEBUG
  checkGL("Failed to set uniform %s", m_name.c_str());
#endif
}

bool Uniform::isScalar() const
{
  return uniformTypes[m_type].scalar;
}

bool Uniform::isVector() const
{
  return uniformTypes[m_type].vector;
}

bool Uniform::isMatrix() const
{
  return uniformTypes[m_type].matrix;
}

bool Uniform::isSampler() const
{
  return uniformTypes[m_type].sampler;
}

uint Uniform::elementCount() const
{
  return uniformTypes[m_type].elementCount;
}

const char* stringCast(AttributeType type)
{
  return attributeTypes[type].name;
}

const char* stringCast(UniformType type)
{
  return uniformTypes[type].name;
}

Program::~Program()
{
  if (m_programID)
    glDeleteProgram(m_programID);

  if (RenderStats* stats = m_context.stats())
    stats->removeProgram();
}

Attribute* Program::findAttribute(const char* name)
{
  auto a = std::find(m_attributes.begin(), m_attributes.end(), name);
  if (a == m_attributes.end())
    return nullptr;

  return &(*a);
}

const Attribute* Program::findAttribute(const char* name) const
{
  auto a = std::find(m_attributes.begin(), m_attributes.end(), name);
  if (a == m_attributes.end())
    return nullptr;

  return &(*a);
}

Uniform* Program::findUniform(const char* name)
{
  auto u = std::find(m_uniforms.begin(), m_uniforms.end(), name);
  if (u == m_uniforms.end())
    return nullptr;

  return &(*u);
}

const Uniform* Program::findUniform(const char* name) const
{
  auto u = std::find(m_uniforms.begin(), m_uniforms.end(), name);
  if (u == m_uniforms.end())
    return nullptr;

  return &(*u);
}

uint Program::attributeCount() const
{
  return uint(m_attributes.size());
}

Attribute& Program::attribute(uint index)
{
  assert(index < m_attributes.size());
  return m_attributes[index];
}

const Attribute& Program::attribute(uint index) const
{
  assert(index < m_attributes.size());
  return m_attributes[index];
}

uint Program::uniformCount() const
{
  return uint(m_uniforms.size());
}

Uniform& Program::uniform(uint index)
{
  assert(index < m_uniforms.size());
  return m_uniforms[index];
}

const Uniform& Program::uniform(uint index) const
{
  assert(index < m_uniforms.size());
  return m_uniforms[index];
}

RenderContext& Program::context() const
{
  return m_context;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             RenderContext& context,
                             Shader& vertexShader,
                             Shader& fragmentShader)
{
  Ref<Program> program(new Program(info, context));
  if (!program->init(vertexShader, fragmentShader))
    return nullptr;

  return program;
}

Ref<Program> Program::read(RenderContext& context,
                           const std::string& vertexShaderName,
                           const std::string& fragmentShaderName)
{
  ResourceCache& cache = context.cache();

  std::string name;
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
    return nullptr;

  Ref<Shader> fragmentShader = Shader::read(context,
                                            FRAGMENT_SHADER,
                                            fragmentShaderName);
  if (!fragmentShader)
    return nullptr;

  return create(ResourceInfo(cache, name),
                context,
                *vertexShader,
                *fragmentShader);
}

Program::Program(const ResourceInfo& info, RenderContext& context):
  Resource(info),
  m_context(context),
  m_programID(0)
{
  if (RenderStats* stats = m_context.stats())
    stats->addProgram();
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

  if (m_context.debug())
    glObjectLabel(GL_PROGRAM, m_programID, name().length(), name().c_str());

  glAttachShader(m_programID, m_vertexShader->m_shaderID);
  glAttachShader(m_programID, m_fragmentShader->m_shaderID);

  glLinkProgram(m_programID);

  const std::string info = infoLog();

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
  m_context.setProgram(this);

  GLint uniformCount;
  glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &uniformCount);

  m_uniforms.reserve(uniformCount);

  GLint maxNameLength;
  glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

  char* uniformName = new char [maxNameLength + 1];
  uint samplerIndex = 0;

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

      if (uniform.isSampler())
      {
        glUniform1i(uniform.m_location, samplerIndex);
        samplerIndex++;
      }
    }
    else
      logWarning("Skipping uniform %s of unsupported type", uniformName);
  }

  delete [] uniformName;

  m_context.setProgram(nullptr);

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

  for (const Attribute& a : m_attributes)
    glEnableVertexAttribArray(a.m_location);
}

void Program::unbind()
{
  for (const Attribute& a : m_attributes)
    glDisableVertexAttribArray(a.m_location);
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

std::string Program::infoLog() const
{
  std::string result;
  GLint length;

  glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &length);
  if (length > 1)
  {
    result.resize(length);
    glGetProgramInfoLog(m_programID, length, nullptr, &result[0]);
  }

  return result;
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
  for (const VertexComponent& c : format.components())
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
                 stringCast(u.second));
      }

      return false;
    }
  }

  for (uint i = 0;  i < program.attributeCount();  i++)
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
                stringCast(attributes[index].second));
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

} /*namespace nori*/


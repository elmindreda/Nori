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

#include <sstream>

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

Mapper<Shader::Type, GLenum> shaderTypeMap;

}

///////////////////////////////////////////////////////////////////////

Shader::~Shader(void)
{
  if (shaderID)
    glDeleteObjectARB(shaderID);
}

Shader::Type Shader::getType(void) const
{
  return type;
}

Shader::Shader(Type initType):
  shaderID(0),
  type(initType)
{
}

bool Shader::init(const String& text)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create GLSL shader without OpenGL context");
    return false;
  }

  if (shaderTypeMap.isEmpty())
  {
    shaderTypeMap[Shader::VERTEX] = GL_VERTEX_SHADER_ARB;
    shaderTypeMap[Shader::FRAGMENT] = GL_FRAGMENT_SHADER_ARB;
  }

  if (!GLEW_ARB_shader_objects ||
      (type == VERTEX && !GLEW_ARB_vertex_shader) ||
      (type == FRAGMENT && !GLEW_ARB_fragment_shader))
  {
    Log::writeError("GLSL shaders are not supported by the current OpenGL context");
    return false;
  }

  shaderID = glCreateShaderObjectARB(shaderTypeMap[type]);
  if (!shaderID)
  {
    Log::writeError("Failed to create GLSL shader object");
    return false;
  }

  const char* string = text.c_str();

  glShaderSourceARB(shaderID, 1, &string, NULL);
  glCompileShaderARB(shaderID);

  int status;

  glGetObjectParameterivARB(shaderID, GL_OBJECT_COMPILE_STATUS_ARB, &status);

  if (!status)
  {
    GLint length;
    glGetObjectParameterivARB(shaderID,
                              GL_OBJECT_INFO_LOG_LENGTH_ARB,
			      &length);

    Block message(length + 1);

    glGetInfoLogARB(shaderID, message.getSize(), &length, (GLcharARB*) message.getData());
    message[length] = '\0';

    Log::writeError("Failed to compile GLSL shader: %s", message.getData());
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

VertexShader* VertexShader::createInstance(const String& text,
				           const String& name)
{
  Ptr<VertexShader> shader = new VertexShader(name);
  if (!shader->init(text))
    return false;

  return shader.detachObject();
}

VertexShader::VertexShader(const String& name):
  Resource<VertexShader>(name),
  Shader(VERTEX)
{
}

///////////////////////////////////////////////////////////////////////

FragmentShader* FragmentShader::createInstance(const String& text,
				               const String& name)
{
  Ptr<FragmentShader> shader = new FragmentShader(name);
  if (!shader->init(text))
    return false;

  return shader.detachObject();
}

FragmentShader::FragmentShader(const String& name):
  Resource<FragmentShader>(name),
  Shader(FRAGMENT)
{
}

///////////////////////////////////////////////////////////////////////

bool ShaderAttribute::isArray(void) const
{
  return count > 1;
}

bool ShaderAttribute::isVector(void) const
{
  if (type == FLOAT_VEC2 ||
      type == FLOAT_VEC3 ||
      type == FLOAT_VEC4)
    return true;

  return false;
}

bool ShaderAttribute::isMatrix(void) const
{
  if (type == FLOAT_MAT2 ||
      type == FLOAT_MAT3 ||
      type == FLOAT_MAT4)
    return true;

  return false;
}

ShaderAttribute::Type ShaderAttribute::getType(void) const
{
  return type;
}

const String& ShaderAttribute::getName(void) const
{
  return name;
}

unsigned int ShaderAttribute::getIndex(void) const
{
  return index;
}

unsigned int ShaderAttribute::getElementCount(void) const
{
  return count;
}

ShaderProgram& ShaderAttribute::getProgram(void) const
{
  return program;
}

ShaderAttribute::ShaderAttribute(ShaderProgram& initProgram):
  program(initProgram)
{
}

ShaderAttribute::ShaderAttribute(const ShaderAttribute& source):
  program(source.program)
{
  // NOTE: Not implemented.
}

ShaderAttribute& ShaderAttribute::operator = (const ShaderAttribute& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

bool ShaderUniform::isArray(void) const
{
  return count > 1;
}

bool ShaderUniform::isVector(void) const
{
  if (type == INT_VEC2 ||
      type == INT_VEC3 ||
      type == INT_VEC4 ||
      type == BOOL_VEC2 ||
      type == BOOL_VEC3 ||
      type == BOOL_VEC4 ||
      type == FLOAT_VEC2 ||
      type == FLOAT_VEC3 ||
      type == FLOAT_VEC4)
    return true;

  return false;
}

bool ShaderUniform::isMatrix(void) const
{
  if (type == FLOAT_MAT2 || type == FLOAT_MAT3 || type == FLOAT_MAT4)
    return true;

  return false;
}

bool ShaderUniform::isSampler(void) const
{
  if (type == SAMPLER_1D ||
      type == SAMPLER_2D ||
      type == SAMPLER_3D ||
      type == SAMPLER_CUBE ||
      type == SAMPLER_1D_SHADOW ||
      type == SAMPLER_2D_SHADOW)
    return true;

  return false;
}

ShaderUniform::Type ShaderUniform::getType(void) const
{
  return type;
}

const String& ShaderUniform::getName(void) const
{
  return name;
}

unsigned int ShaderUniform::getElementCount(void) const
{
  return count;
}

void ShaderUniform::setValue(int newValue, unsigned int index)
{
  program.apply();
  glUniform1iARB(locations[index], newValue);
}

void ShaderUniform::setValue(bool newValue, unsigned int index)
{
  program.apply();
  glUniform1iARB(locations[index], newValue);
}

void ShaderUniform::setValue(float newValue, unsigned int index)
{
  program.apply();
  glUniform1fARB(locations[index], newValue);
}

void ShaderUniform::setValue(const Vector2& newValue, unsigned int index)
{
  program.apply();
  glUniform2fvARB(locations[index], 1, newValue);
}

void ShaderUniform::setValue(const Vector3& newValue, unsigned int index)
{
  program.apply();
  glUniform3fvARB(locations[index], 1, newValue);
}

void ShaderUniform::setValue(const Vector4& newValue, unsigned int index)
{
  program.apply();
  glUniform4fvARB(locations[index], 1, newValue);
}

void ShaderUniform::setValue(const Matrix2& newValue, unsigned int index)
{
  program.apply();
  glUniformMatrix2fvARB(locations[index], 1, GL_FALSE, newValue);
}

void ShaderUniform::setValue(const Matrix3& newValue, unsigned int index)
{
  program.apply();
  glUniformMatrix3fvARB(locations[index], 1, GL_FALSE, newValue);
}

void ShaderUniform::setValue(const Matrix4& newValue, unsigned int index)
{
  program.apply();
  glUniformMatrix4fvARB(locations[index], 1, GL_FALSE, newValue);
}

ShaderProgram& ShaderUniform::getProgram(void) const
{
  return program;
}

ShaderUniform::ShaderUniform(ShaderProgram& initProgram):
  program(initProgram)
{
}

ShaderUniform::ShaderUniform(const ShaderUniform& source):
  program(source.program)
{
  // NOTE: Not implemented.
}

ShaderUniform& ShaderUniform::operator = (const ShaderUniform& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

ShaderProgram::~ShaderProgram(void)
{
  destroyUniforms();
  destroyAttributes();

  while (!shaders.empty())
  {
    removeShader(*shaders.back());
    shaders.pop_back();
  }

  if (current == this)
    applyFixedFunction();

  if (programID)
    glDeleteObjectARB(programID);
}

void ShaderProgram::addShader(Shader& shader)
{
  if (std::find(shaders.begin(), shaders.end(), &shader) != shaders.end())
    return;

  glAttachObjectARB(programID, shader.shaderID);
  shaders.push_back(&shader);
  modified = true;
}

void ShaderProgram::removeShader(Shader& shader)
{
  ShaderList::iterator i = std::find(shaders.begin(), shaders.end(), &shader);
  if (i == shaders.end())
    return;

  glDetachObjectARB(programID, shader.shaderID);
  shaders.erase(i);
  modified = true;
}

bool ShaderProgram::link(void)
{
  destroyUniforms();

  glLinkProgramARB(programID);

  GLint status;
  glGetObjectParameterivARB(programID,
                            GL_OBJECT_LINK_STATUS_ARB,
			    &status);

  if (!status)
  {
    GLint length;
    glGetObjectParameterivARB(programID,
                              GL_OBJECT_INFO_LOG_LENGTH_ARB,
			      &length);

    Block message(length + 1);

    glGetInfoLogARB(programID, message.getSize(), &length, (GLcharARB*) message.getData());
    message[length] = '\0';

    Log::writeError("Failed to link GLSL program %s: %s", getName().c_str(), message.getData());
    return false;
  }

  if (!apply())
    return false;

  if (!createUniforms())
    return false;

  if (!createAttributes())
    return false;

  modified = false;
  return true;
}

bool ShaderProgram::apply(void) const
{
  if (current == this)
    return true;

  glUseProgramObjectARB(programID);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Use of GLSL program %s failed: %s", getName().c_str(), gluErrorString(error));
    return false;
  }

  current = const_cast<ShaderProgram*>(this);
  return true;
}

bool ShaderProgram::isValid(void) const
{
  glValidateProgramARB(programID);

  GLint status;
  glGetObjectParameterivARB(programID,
                            GL_OBJECT_VALIDATE_STATUS_ARB,
			    &status);

  if (!status)
  {
    GLint length;
    glGetObjectParameterivARB(programID,
                              GL_OBJECT_INFO_LOG_LENGTH_ARB,
			      &length);

    Block message(length + 1);

    glGetInfoLogARB(programID, message.getSize(), &length, (GLcharARB*) message.getData());
    message[length] = '\0';

    Log::writeError("Validation of GLSL program %s failed: %s", getName().c_str(), message.getData());
    return false;
  }

  return true;
}

bool ShaderProgram::isModified(void) const
{
  return modified;
}

unsigned int ShaderProgram::getShaderCount(void) const
{
  return shaders.size();
}

Shader& ShaderProgram::getShader(unsigned int index)
{
  return *shaders[index];
}

const Shader& ShaderProgram::getShader(unsigned int index) const
{
  return *shaders[index];
}

unsigned int ShaderProgram::getUniformCount(void) const
{
  return uniforms.size();
}

ShaderUniform& ShaderProgram::getUniform(unsigned int index)
{
  return *uniforms[index];
}

const ShaderUniform& ShaderProgram::getUniform(unsigned int index) const
{
  return *uniforms[index];
}

ShaderUniform* ShaderProgram::getUniform(const String& name)
{
  for (UniformList::iterator i = uniforms.begin();  i != uniforms.end();  i++)
  {
    if ((*i)->name == name)
      return *i;
  }

  return NULL;
}

const ShaderUniform* ShaderProgram::getUniform(const String& name) const
{
  for (UniformList::const_iterator i = uniforms.begin();  i != uniforms.end();  i++)
  {
    if ((*i)->name == name)
      return *i;
  }

  return NULL;
}

ShaderProgram* ShaderProgram::createInstance(const String& name)
{
  Ptr<ShaderProgram> program = new ShaderProgram(name);
  if (!program->init())
    return NULL;

  return program.detachObject();
}

void ShaderProgram::applyFixedFunction(void)
{
  glUseProgramObjectARB(0);
  current = NULL;
}

ShaderProgram* ShaderProgram::getCurrent(void)
{
  return current;
}

ShaderProgram::ShaderProgram(const String& name):
  Resource<ShaderProgram>(name),
  programID(0),
  modified(false)
{
}

bool ShaderProgram::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create GLSL program without OpenGL context");
    return false;
  }

  if (!GLEW_ARB_shader_objects)
  {
    Log::writeError("GLSL programs are not supported by the current OpenGL context");
    return false;
  }

  programID = glCreateProgramObjectARB();
  if (!programID)
  {
    Log::writeError("Failed to create object for GLSL program %s", getName().c_str());
    return false;
  }

  return true;
}

bool ShaderProgram::createUniforms(void)
{
  GLint uniformCount;
  glGetObjectParameterivARB(programID,
                            GL_OBJECT_ACTIVE_UNIFORMS_ARB,
			    &uniformCount);

  GLint maxNameLength;
  glGetObjectParameterivARB(programID,
                            GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB,
			    &maxNameLength);

  Block name(maxNameLength);

  for (unsigned int index = 0;  index < uniformCount;  index++)
  {
    GLenum type;
    GLsizei count, length;

    glGetActiveUniformARB(programID,
                          index,
			  name.getSize(),
			  &length,
			  &count,
			  &type,
			  (GLcharARB*) name.getData());

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      Log::writeError("Failed to retrieve uniform %u in GLSL program %s: %s",
                      index, getName().c_str(), gluErrorString(error));
      return false;
    }

    if (!length)
    {
      Log::writeWarning("No information available for uniform %u in GLSL program %s",
                        index, getName().c_str());
      continue;
    }

    if (length > 3 && strncmp((const char*) name.getData(), "gl_", 3) == 0)
      continue;

    std::vector<GLint> locations;

    if (count > 1)
    {
      for (unsigned int i = 0;  i < count;  i++)
      {
	std::stringstream elementName;
	elementName << name.getData() << '[' << i << ']';

	GLint location = glGetUniformLocation(programID, elementName.str().c_str());
	if (location == -1)
	{
	  Log::writeError("Failed to retrieve location of uniform %s in GLSL program %s",
			  elementName.str().c_str(), getName().c_str());
	  return false;
	}

	locations.push_back(location);
      }
    }
    else
    {
      GLint location = glGetUniformLocation(programID, (GLcharARB*) name.getData());
      if (location == -1)
      {
	Log::writeError("Failed to retrieve location of uniform %s in GLSL program %s",
			name.getData(), getName().c_str());
	return false;
      }

      locations.push_back(location);
    }

    ShaderUniform* uniform = new ShaderUniform(*this);
    uniforms.push_back(uniform);

    uniform->name.assign((char*) name.getData(), length);
    uniform->type = (ShaderUniform::Type) type;
    uniform->count = count;
    uniform->locations = locations;
  }

  return true;
}

bool ShaderProgram::createAttributes(void)
{
  GLint attributeCount;
  glGetObjectParameterivARB(programID,
                            GL_OBJECT_ACTIVE_ATTRIBUTES_ARB,
			    &attributeCount);

  GLint maxNameLength;
  glGetObjectParameterivARB(programID,
                            GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB,
			    &maxNameLength);

  Block name(maxNameLength);

  for (unsigned int index = 0;  index < attributeCount;  index++)
  {
    GLenum type;
    GLsizei count, length;

    glGetActiveAttribARB(programID,
			 index,
			 name.getSize(),
			 &length,
			 &count,
			 &type,
			 (GLcharARB*) name.getData());

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      Log::writeError("Failed to retrieve attribute %u in GLSL program %s: %s",
                      index, getName().c_str(), gluErrorString(error));
      return false;
    }

    if (!length)
    {
      Log::writeWarning("No information available for attribute %u in GLSL program %s",
                        index, getName().c_str());
      continue;
    }

    if (length > 3 && strncmp((const char*) name.getData(), "gl_", 3) == 0)
      continue;

    GLint location = glGetAttribLocation(programID, (GLcharARB*) name.getData());
    if (location == -1)
    {
      Log::writeError("Failed to retrieve location of attribute %s in GLSL program %s",
		      name.getData(), getName().c_str());
      return false;
    }

    ShaderAttribute* attribute = new ShaderAttribute(*this);
    attributes.push_back(attribute);

    attribute->name.assign((char*) name.getData(), length);
    attribute->type = (ShaderAttribute::Type) type;
    attribute->count = count;
    attribute->index = location;
  }

  return true;
}

void ShaderProgram::destroyUniforms(void)
{
  while (!uniforms.empty())
  {
    delete uniforms.back();
    uniforms.pop_back();
  }
}

void ShaderProgram::destroyAttributes(void)
{
  while (!attributes.empty())
  {
    delete attributes.back();
    attributes.pop_back();
  }
}

ShaderProgram* ShaderProgram::current = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

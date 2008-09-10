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

#include <cstring>
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

Shader::Shader(Type initType, const String& initText):
  type(initType),
  text(initText)
{
}

Shader::Type Shader::getType(void) const
{
  return type;
}

const String& Shader::getText(void) const
{
  return text;
}

///////////////////////////////////////////////////////////////////////

VertexShader* VertexShader::createInstance(const String& text, const String& name)
{
  if (!GLEW_ARB_vertex_shader)
    return NULL;

  return new VertexShader(text, name);
}

VertexShader::VertexShader(const String& text, const String& name):
  Resource<VertexShader>(name),
  Shader(VERTEX, text)
{
}

///////////////////////////////////////////////////////////////////////

FragmentShader* FragmentShader::createInstance(const String& text, const String& name)
{
  if (!GLEW_ARB_fragment_shader)
    return NULL;

  return new FragmentShader(text, name);
}

FragmentShader::FragmentShader(const String& text, const String& name):
  Resource<FragmentShader>(name),
  Shader(FRAGMENT, text)
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
  glUniform1iARB(locations[index], newValue);
}

void ShaderUniform::setValue(bool newValue, unsigned int index)
{
  glUniform1iARB(locations[index], newValue);
}

void ShaderUniform::setValue(float newValue, unsigned int index)
{
  glUniform1fARB(locations[index], newValue);
}

void ShaderUniform::setValue(const Vector2& newValue, unsigned int index)
{
  glUniform2fvARB(locations[index], 1, newValue);
}

void ShaderUniform::setValue(const Vector3& newValue, unsigned int index)
{
  glUniform3fvARB(locations[index], 1, newValue);
}

void ShaderUniform::setValue(const Vector4& newValue, unsigned int index)
{
  glUniform4fvARB(locations[index], 1, newValue);
}

void ShaderUniform::setValue(const Matrix2& newValue, unsigned int index)
{
  glUniformMatrix2fvARB(locations[index], 1, GL_FALSE, newValue);
}

void ShaderUniform::setValue(const Matrix3& newValue, unsigned int index)
{
  glUniformMatrix3fvARB(locations[index], 1, GL_FALSE, newValue);
}

void ShaderUniform::setValue(const Matrix4& newValue, unsigned int index)
{
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
  while (!uniforms.empty())
  {
    delete uniforms.back();
    uniforms.pop_back();
  }

  while (!attributes.empty())
  {
    delete attributes.back();
    attributes.pop_back();
  }

  if (current == this)
    applyFixedFunction();

  if (vertexID)
    glDeleteObjectARB(vertexID);

  if (fragmentID)
    glDeleteObjectARB(fragmentID);

  if (programID)
    glDeleteObjectARB(programID);
}

bool ShaderProgram::apply(void) const
{
  if (current == this)
    return true;

  glUseProgramObjectARB(programID);

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Use of permutation %s of GLSL program %s failed: %s",
                    getName().c_str(),
		    program.getName().c_str(),
		    gluErrorString(error));
    return false;
  }
#endif

  current = const_cast<ShaderProgram*>(this);
  return true;
}

bool ShaderProgram::isValid(void) const
{
  glValidateProgramARB(programID);

  GLint status;
  glGetObjectParameterivARB(programID, GL_OBJECT_VALIDATE_STATUS_ARB, &status);

  GLint length;
  glGetObjectParameterivARB(programID, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);

  if (length > 1)
  {
    Block message(length);

    glGetInfoLogARB(programID, (GLsizei) message.getSize(), NULL, (GLcharARB*) message.getData());

    if (status)
      Log::writeWarning("Warnings during validation of GLSL program %s: %s",
			getName().c_str(),
			message.getData());
    else
      Log::writeError("Validation of GLSL program %s failed: %s",
		      getName().c_str(),
		      message.getData());
  }

  return status ? true : false;
}

unsigned int ShaderProgram::getUniformCount(void) const
{
  return (unsigned int) uniforms.size();
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

const VertexShader& ShaderProgram::getVertexShader(void) const
{
  return *vertexShader;
}

const FragmentShader& ShaderProgram::getFragmentShader(void) const
{
  return *fragmentShader;
}

ShaderProgram* ShaderProgram::createInstance(VertexShader& vertexShader,
				             FragmentShader& fragmentShader,
					     const String& name)
{
  Ptr<ShaderProgram> program = new ShaderProgram(name);
  if (!program->init(vertexShader, fragmentShader))
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
  vertexID(0),
  fragmentID(0)
{
}

bool ShaderProgram::init(VertexShader& initVertexShader,
			 FragmentShader& initFragmentShader)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create GLSL program without OpenGL context");
    return false;
  }

  if (!GLEW_ARB_shading_language_100)
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

  vertexShader = &initVertexShader;
  fragmentShader = &initFragmentShader;

  vertexID = createShader(*vertexShader);
  if (!vertexID)
    return false;

  fragmentID = createShader(*fragmentShader);
  if (!fragmentID)
    return false;

  glLinkProgramARB(programID);

  GLint status;
  glGetObjectParameterivARB(programID, GL_OBJECT_LINK_STATUS_ARB, &status);

  GLint length;
  glGetObjectParameterivARB(programID, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);

  if (length > 1)
  {
    Block message(length);

    glGetInfoLogARB(programID, (GLsizei) message.getSize(), NULL, (GLcharARB*) message.getData());

    if (status)
      Log::writeWarning("Warnings when linking GLSL program %s: %s",
			getName().c_str(),
			message.getData());
    else
      Log::writeError("Failed to link GLSL program %s: %s",
		      getName().c_str(),
		      message.getData());
  }

  if (!status)
    return false;
  
  if (!apply())
    return false;

  if (!createUniforms())
    return false;

  if (!createAttributes())
    return false;

  return true;
}

GLhandleARB ShaderProgram::createShader(const Shader& shader)
{
  if (shaderTypeMap.isEmpty())
  {
    shaderTypeMap[Shader::VERTEX] = GL_VERTEX_SHADER_ARB;
    shaderTypeMap[Shader::FRAGMENT] = GL_FRAGMENT_SHADER_ARB;
  }

  GLhandleARB shaderID = glCreateShaderObjectARB(shaderTypeMap[shader.getType()]);
  if (!shaderID)
  {
    Log::writeError("Failed to create GLSL shader object");
    return 0;
  }

  const char* string = shader.getText().c_str();

  glShaderSourceARB(shaderID, 1, &string, NULL);
  glCompileShaderARB(shaderID);

  GLint status;
  glGetObjectParameterivARB(shaderID, GL_OBJECT_COMPILE_STATUS_ARB, &status);

  GLint length;
  glGetObjectParameterivARB(shaderID, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);

  if (length > 1)
  {
    Block message(length);

    glGetInfoLogARB(shaderID, (GLsizei) message.getSize(), NULL, (GLcharARB*) message.getData());

    if (status)
      Log::writeWarning("Warnings when compiling GLSL shader %s: %s",
                        getName().c_str(),
			message.getData());
    else
      Log::writeError("Failed to compile GLSL shader %s: %s",
                      getName().c_str(),
		      message.getData());
  }

  if (!status)
  {
    glDeleteObjectARB(shaderID);
    return 0;
  }

  glAttachObjectARB(programID, shaderID);
  return shaderID;
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

  Block uniformName(maxNameLength);

  for (unsigned int index = 0;  index < uniformCount;  index++)
  {
    GLenum type;
    GLsizei count, length;

    glGetActiveUniformARB(programID,
                          index,
			  uniformName.getSize(),
			  &length,
			  &count,
			  &type,
			  (GLcharARB*) uniformName.getData());

#if _DEBUG
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      Log::writeError("Failed to retrieve uniform %u in GLSL program %s: %s",
                      index, getName().c_str(), gluErrorString(error));
      return false;
    }
#endif

    if (!length)
    {
      Log::writeWarning("No information available for uniform %u in GLSL program %s",
                        index, getName().c_str());
      continue;
    }

    if (length > 3 && std::strncmp((const char*) uniformName.getData(), "gl_", 3) == 0)
      continue;

    std::vector<GLint> locations;

    if (count > 1)
    {
      for (unsigned int i = 0;  i < count;  i++)
      {
	std::stringstream elementName;
	elementName << uniformName.getData() << '[' << i << ']';

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
      GLint location = glGetUniformLocation(programID, (GLcharARB*) uniformName.getData());
      if (location == -1)
      {
	Log::writeError("Failed to retrieve location of uniform %s in GLSL program %s",
			uniformName.getData(), getName().c_str());
	return false;
      }

      locations.push_back(location);
    }

    ShaderUniform* uniform = new ShaderUniform(*this);
    uniforms.push_back(uniform);

    uniform->name.assign((char*) uniformName.getData(), length);
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

  Block attributeName(maxNameLength);

  for (unsigned int index = 0;  index < attributeCount;  index++)
  {
    GLenum type;
    GLsizei count, length;

    glGetActiveAttribARB(programID,
			 index,
			 attributeName.getSize(),
			 &length,
			 &count,
			 &type,
			 (GLcharARB*) attributeName.getData());

#if _DEBUG
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      Log::writeError("Failed to retrieve attribute %u in GLSL program %s: %s",
                      index, getName().c_str(), gluErrorString(error));
      return false;
    }
#endif

    if (!length)
    {
      Log::writeWarning("No information available for attribute %u in GLSL program %s",
                        index, getName().c_str());
      continue;
    }

    if (length > 3 && std::strncmp((const char*) attributeName.getData(), "gl_", 3) == 0)
      continue;

    GLint location = glGetAttribLocation(programID, (GLcharARB*) attributeName.getData());
    if (location == -1)
    {
      Log::writeError("Failed to retrieve location of attribute %s in GLSL program %s",
		      attributeName.getData(), getName().c_str());
      return false;
    }

    ShaderAttribute* attribute = new ShaderAttribute(*this);
    attributes.push_back(attribute);

    attribute->name.assign((char*) attributeName.getData(), length);
    attribute->type = (ShaderAttribute::Type) type;
    attribute->count = count;
    attribute->index = location;
  }

  return true;
}

ShaderProgram* ShaderProgram::current = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

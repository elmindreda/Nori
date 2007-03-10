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
#include <wendy/GLLight.h>
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

Shader::Shader(Type initType, const String& initText):
  type(initType),
  text(initText),
  lighting(false)
{
  if (text.find("wendyLighting") != String::npos)
    lighting = true;
}

bool Shader::isUsingLighting(void) const
{
  return lighting;
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

VertexShader::VertexShader(const String& text, const String& name):
  Resource<VertexShader>(name),
  Shader(VERTEX, text)
{
}

///////////////////////////////////////////////////////////////////////

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

ShaderPermutation& ShaderAttribute::getPermutation(void) const
{
  return permutation;
}

ShaderAttribute::ShaderAttribute(ShaderPermutation& initPermutation):
  permutation(initPermutation)
{
}

ShaderAttribute::ShaderAttribute(const ShaderAttribute& source):
  permutation(source.permutation)
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

ShaderPermutation& ShaderUniform::getPermutation(void) const
{
  return permutation;
}

ShaderUniform::ShaderUniform(ShaderPermutation& initPermutation):
  permutation(initPermutation)
{
}

ShaderUniform::ShaderUniform(const ShaderUniform& source):
  permutation(source.permutation)
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
  while (!permutations.empty())
  {
    delete permutations.back();
    permutations.pop_back();
  }
}

bool ShaderProgram::apply(void)
{
  String name;
  LightState::getCurrent().getPermutationName(name);

  ShaderPermutation* permutation = findPermutation(name);
  if (!permutation)
  {
    permutation = createPermutation(LightState::getCurrent());
    if (!permutation)
      return false;
  }

  appliedSignal.emit(*permutation);
  return permutation->apply();
}

bool ShaderProgram::isUsingLighting(void) const
{
  if (vertexShader->isUsingLighting() || fragmentShader->isUsingLighting())
    return true;

  return false;
}

const VertexShader& ShaderProgram::getVertexShader(void) const
{
  return *vertexShader;
}

const FragmentShader& ShaderProgram::getFragmentShader(void) const
{
  return *fragmentShader;
}

SignalProxy1<void, ShaderPermutation&> ShaderProgram::getPermutationCreatedSignal(void)
{
  return createdSignal;
}

SignalProxy1<void, ShaderPermutation&> ShaderProgram::getPermutationAppliedSignal(void)
{
  return appliedSignal;
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
  ShaderPermutation::applyFixedFunction();
}

ShaderProgram::ShaderProgram(const String& name):
  Resource<ShaderProgram>(name)
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

  vertexShader = &initVertexShader;
  fragmentShader = &initFragmentShader;

  if (!createPermutation(LightState()))
    return false;

  return true;
}

ShaderPermutation* ShaderProgram::createPermutation(const LightState& lights)
{
  ShaderPermutation* permutation = new ShaderPermutation(*this);
  if (!permutation->init(lights))
  {
    delete permutation;
    return NULL;
  }

  permutations.push_back(permutation);
  createdSignal.emit(*permutation);
  return permutation;
}

ShaderPermutation* ShaderProgram::findPermutation(const String& name)
{
  for (unsigned int i = 0;  i < permutations.size();  i++)
  {
    if (permutations[i]->name == name)
      return permutations[i];
  }

  return NULL;
}

const ShaderPermutation* ShaderProgram::findPermutation(const String& name) const
{
  for (unsigned int i = 0;  i < permutations.size();  i++)
  {
    if (permutations[i]->name == name)
      return permutations[i];
  }

  return NULL;
}

///////////////////////////////////////////////////////////////////////

bool ShaderPermutation::isValid(void) const
{
  glValidateProgramARB(programID);

  GLint status;
  glGetObjectParameterivARB(programID, GL_OBJECT_VALIDATE_STATUS_ARB, &status);

  GLint length;
  glGetObjectParameterivARB(programID, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);

  if (length > 1)
  {
    Block message(length);

    glGetInfoLogARB(programID, message.getSize(), NULL, (GLcharARB*) message.getData());

    if (status)
      Log::writeWarning("Warnings during validation of variant %s of GLSL program %s: %s",
			name.c_str(),
			program.getName().c_str(),
			message.getData());
    else
      Log::writeError("Validation of variant %s of GLSL program %s failed: %s",
		      name.c_str(),
		      program.getName().c_str(),
		      message.getData());
  }

  return status ? true : false;
}

const String& ShaderPermutation::getName(void) const
{
  return name;
}

unsigned int ShaderPermutation::getUniformCount(void) const
{
  return uniforms.size();
}

ShaderUniform& ShaderPermutation::getUniform(unsigned int index)
{
  return *uniforms[index];
}

const ShaderUniform& ShaderPermutation::getUniform(unsigned int index) const
{
  return *uniforms[index];
}

ShaderUniform* ShaderPermutation::getUniform(const String& name)
{
  for (UniformList::iterator i = uniforms.begin();  i != uniforms.end();  i++)
  {
    if ((*i)->name == name)
      return *i;
  }

  return NULL;
}

const ShaderUniform* ShaderPermutation::getUniform(const String& name) const
{
  for (UniformList::const_iterator i = uniforms.begin();  i != uniforms.end();  i++)
  {
    if ((*i)->name == name)
      return *i;
  }

  return NULL;
}

ShaderPermutation* ShaderPermutation::getCurrent(void)
{
  return current;
}

ShaderPermutation::ShaderPermutation(ShaderProgram& initProgram):
  program(initProgram),
  programID(0),
  vertexID(0),
  fragmentID(0)
{
}

ShaderPermutation::~ShaderPermutation(void)
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

bool ShaderPermutation::init(const LightState& state)
{
  state.getPermutationName(name);

  programID = glCreateProgramObjectARB();
  if (!programID)
  {
    Log::writeError("Failed to create object for GLSL program %s",
                    program.getName().c_str());
    return false;
  }

  vertexID = createShader(program.getVertexShader(), state);
  if (!vertexID)
    return false;

  fragmentID = createShader(program.getFragmentShader(), state);
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

    glGetInfoLogARB(programID, message.getSize(), NULL, (GLcharARB*) message.getData());

    if (status)
      Log::writeWarning("Warnings when linking variant %s of GLSL program %s: %s",
                        name.c_str(),
			program.getName().c_str(),
			message.getData());
    else
      Log::writeError("Failed to link variant %s of GLSL program %s: %s",
                      name.c_str(),
		      program.getName().c_str(),
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

bool ShaderPermutation::apply(void) const
{
  if (current == this)
    return true;

  glUseProgramObjectARB(programID);

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Use of permutation %s of GLSL program %s failed: %s",
                    name.c_str(),
		    program.getName().c_str(),
		    gluErrorString(error));
    return false;
  }
#endif

  current = const_cast<ShaderPermutation*>(this);
  return true;
}

void ShaderPermutation::applyFixedFunction(void)
{
  glUseProgramObjectARB(0);
  current = NULL;
}

GLhandleARB ShaderPermutation::createShader(const Shader& shader, const LightState& state)
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

  String text;

  if (shader.isUsingLighting())
  {
    text.append(state.getPermutationText());
    text.append("\n\n");
  }

  text.append(shader.getText());

  const char* string = text.c_str();

  glShaderSourceARB(shaderID, 1, &string, NULL);
  glCompileShaderARB(shaderID);

  int status;
  glGetObjectParameterivARB(shaderID, GL_OBJECT_COMPILE_STATUS_ARB, &status);

  GLint length;
  glGetObjectParameterivARB(shaderID, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);

  if (length > 1)
  {
    Block message(length);

    glGetInfoLogARB(shaderID, message.getSize(), NULL, (GLcharARB*) message.getData());

    if (status)
      Log::writeWarning("Warnings when compiling GLSL shader: %s", message.getData());
    else
      Log::writeError("Failed to compile GLSL shader: %s", message.getData());
  }

  if (!status)
  {
    glDeleteObjectARB(shaderID);
    return 0;
  }

  glAttachObjectARB(programID, shaderID);
  return shaderID;
}

bool ShaderPermutation::createUniforms(void)
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
                      index, name.c_str(), gluErrorString(error));
      return false;
    }
#endif

    if (!length)
    {
      Log::writeWarning("No information available for uniform %u in GLSL program %s",
                        index, name.c_str());
      continue;
    }

    if (length > 3 && strncmp((const char*) uniformName.getData(), "gl_", 3) == 0)
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
			  elementName.str().c_str(), name.c_str());
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
			uniformName.getData(), name.c_str());
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

bool ShaderPermutation::createAttributes(void)
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
                      index, name.c_str(), gluErrorString(error));
      return false;
    }
#endif

    if (!length)
    {
      Log::writeWarning("No information available for attribute %u in GLSL program %s",
                        index, name.c_str());
      continue;
    }

    if (length > 3 && strncmp((const char*) attributeName.getData(), "gl_", 3) == 0)
      continue;

    GLint location = glGetAttribLocation(programID, (GLcharARB*) attributeName.getData());
    if (location == -1)
    {
      Log::writeError("Failed to retrieve location of attribute %s in GLSL program %s",
		      attributeName.getData(), name.c_str());
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

ShaderPermutation* ShaderPermutation::current = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

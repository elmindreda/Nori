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
#include <wendy/GLTexture.h>
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

ShaderUniform::Type convertUniformType(CGtype type)
{
  switch (type)
  {
    case CG_FLOAT:
      return ShaderUniform::FLOAT;
    case CG_FLOAT2:
      return ShaderUniform::FLOAT_VEC2;
    case CG_FLOAT3:
      return ShaderUniform::FLOAT_VEC3;
    case CG_FLOAT4:
      return ShaderUniform::FLOAT_VEC4;
    case CG_FLOAT2x2:
      return ShaderUniform::FLOAT_MAT2;
    case CG_FLOAT3x3:
      return ShaderUniform::FLOAT_MAT3;
    case CG_FLOAT4x4:
      return ShaderUniform::FLOAT_MAT4;
    case CG_SAMPLER1D:
      return ShaderUniform::SAMPLER_1D;
    case CG_SAMPLER2D:
      return ShaderUniform::SAMPLER_2D;
    case CG_SAMPLER3D:
      return ShaderUniform::SAMPLER_3D;
    case CG_SAMPLERRECT:
      return ShaderUniform::SAMPLER_RECT;
    case CG_SAMPLERCUBE:
      return ShaderUniform::SAMPLER_CUBE;
    default:
      throw Exception("Invalid Cg parameter type");
  }
}

}

///////////////////////////////////////////////////////////////////////

bool ShaderUniform::isScalar(void) const
{
  if (type == FLOAT)
    return true;

  return false;
}

bool ShaderUniform::isVector(void) const
{
  if (type == FLOAT_VEC2 || type == FLOAT_VEC3 || type == FLOAT_VEC4)
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
  if (type == SAMPLER_1D || type == SAMPLER_2D || type == SAMPLER_3D ||
      type == SAMPLER_RECT || type == SAMPLER_CUBE)
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

void ShaderUniform::setValue(float newValue)
{
  if (type != FLOAT)
  {
    Log::writeError("Uniform %s in program %s is not of type float", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter1f(uniformID, newValue);
}

void ShaderUniform::setValue(const Vector2& newValue)
{
  if (type != FLOAT_VEC2)
  {
    Log::writeError("Uniform %s in program %s is not of type float2", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter2fv(uniformID, newValue);
}

void ShaderUniform::setValue(const Vector3& newValue)
{
  if (type != FLOAT_VEC3)
  {
    Log::writeError("Uniform %s in program %s is not of type float3", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter3fv(uniformID, newValue);
}

void ShaderUniform::setValue(const Vector4& newValue)
{
  if (type != FLOAT_VEC4)
  {
    Log::writeError("Uniform %s in program %s is not of type float4", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter4fv(uniformID, newValue);
}

void ShaderUniform::setValue(const Matrix2& newValue)
{
  if (type != FLOAT_MAT2)
  {
    Log::writeError("Uniform %s in program %s is not of type float2x2", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetMatrixParameterfr(uniformID, newValue);
}

void ShaderUniform::setValue(const Matrix3& newValue)
{
  if (type != FLOAT_MAT3)
  {
    Log::writeError("Uniform %s in program %s is not of type float3x3", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetMatrixParameterfr(uniformID, newValue);
}

void ShaderUniform::setValue(const Matrix4& newValue)
{
  if (type != FLOAT_MAT4)
  {
    Log::writeError("Uniform %s in program %s is not of type float2", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetMatrixParameterfr(uniformID, newValue);
}

void ShaderUniform::setTexture(const Texture& newTexture)
{
  if (!isSampler())
  {
    Log::writeError("Uniform %s in program %s is not a sampler", name.c_str(), program.getName().c_str());
    return;
  }

  // TODO: Check for texture type match.

  cgGLSetTextureParameter(uniformID, newTexture.textureID);
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
}

ShaderUniform& ShaderUniform::operator = (const ShaderUniform& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

VertexShader::~VertexShader(void)
{
  if (shaderID)
    cgDestroyProgram(shaderID);
}

const String& VertexShader::getText(void) const
{
  return text;
}

VertexShader* VertexShader::createInstance(Context& context,
                                           const String& text,
					   const String& name)
{
  Ptr<VertexShader> shader = new VertexShader(context, name);
  if (!shader->init(text))
    return NULL;

  return shader.detachObject();
}

VertexShader::VertexShader(Context& initContext, const String& name):
  Resource<VertexShader>(name),
  context(initContext),
  shaderID(NULL)
{
}

VertexShader::VertexShader(const VertexShader& source):
  Resource<VertexShader>(""),
  context(source.context)
{
}

bool VertexShader::init(const String& initText)
{
  text = initText;

  shaderID = cgCreateProgram(context.cgContextID, CG_SOURCE, text.c_str(), context.cgVertexProfile, NULL, NULL);
  if (!shaderID)
  {
    Log::writeError("Failed to compile Cg vertex shader: %s", cgGetErrorString(cgGetError()));
    return false;
  }

  return true;
}

VertexShader& VertexShader::operator = (const VertexShader& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

FragmentShader::~FragmentShader(void)
{
  if (shaderID)
    cgDestroyProgram(shaderID);
}

const String& FragmentShader::getText(void) const
{
  return text;
}

FragmentShader* FragmentShader::createInstance(Context& context,
                                               const String& text,
					       const String& name)
{
  Ptr<FragmentShader> shader = new FragmentShader(context, name);
  if (!shader->init(text))
    return NULL;

  return shader.detachObject();
}

FragmentShader::FragmentShader(Context& initContext, const String& name):
  Resource<FragmentShader>(name),
  context(initContext),
  shaderID(NULL)
{
}

FragmentShader::FragmentShader(const FragmentShader& source):
  Resource<FragmentShader>(""),
  context(source.context)
{
}

bool FragmentShader::init(const String& initText)
{
  text = initText;

  shaderID = cgCreateProgram(context.cgContextID, CG_SOURCE, text.c_str(), context.cgFragmentProfile, NULL, NULL);
  if (!shaderID)
  {
    Log::writeError("Failed to compile Cg fragment shader: %s", cgGetErrorString(cgGetError()));
    return false;
  }

  return true;
}

FragmentShader& FragmentShader::operator = (const FragmentShader& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

void ShaderProgram::apply(void) const
{
  cgGLBindProgram(programID);
}

ShaderUniform* ShaderProgram::findUniform(const String& name)
{
  for (UniformList::const_iterator i = uniforms.begin();  i != uniforms.end();  i++)
  {
    if ((*i)->name == name)
      return *i;
  }

  return NULL;
}

const ShaderUniform* ShaderProgram::findUniform(const String& name) const
{
  for (UniformList::const_iterator i = uniforms.begin();  i != uniforms.end();  i++)
  {
    if ((*i)->name == name)
      return *i;
  }

  return NULL;
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

VertexShader& ShaderProgram::getVertexShader(void) const
{
  return *vertexShader;
}

FragmentShader& ShaderProgram::getFragmentShader(void) const
{
  return *fragmentShader;
}

SignalProxy1<void, ShaderProgram&> ShaderProgram::getAppliedSignal(void)
{
  return appliedSignal;
}

ShaderProgram* ShaderProgram::createInstance(Context& context,
                                             VertexShader& vertexShader,
					     FragmentShader& fragmentShader,
					     const String& name)
{
  Ptr<ShaderProgram> program = new ShaderProgram(context, name);
  if (!program->init(vertexShader, fragmentShader))
    return NULL;

  return program.detachObject();
}

ShaderProgram::ShaderProgram(Context& initContext, const String& name):
  Resource<ShaderProgram>(name),
  context(initContext)
{
}

ShaderProgram::ShaderProgram(const ShaderProgram& source):
  Resource<ShaderProgram>(""),
  context(source.context)
{
}

bool ShaderProgram::init(VertexShader& initVertexShader, FragmentShader& initFragmentShader)
{
  vertexShader = &initVertexShader;
  fragmentShader = &initFragmentShader;

  programID = cgCombinePrograms2(vertexShader->shaderID, fragmentShader->shaderID);
  if (!programID)
  {
    Log::writeError("Unable to combine shaders for program %s", getName().c_str());
    return false;
  }

  CGparameter parameter = cgGetFirstParameter(programID, CG_PROGRAM);

  while (parameter)
  {
    CGtype type = cgGetParameterType(parameter);
    if (type != CG_ARRAY && type != CG_STRUCT)
    {
      ShaderUniform* uniform = new ShaderUniform(*this);
      uniform->name = cgGetParameterName(parameter);
      uniform->type = convertUniformType(type);
      uniform->uniformID = parameter;

      uniforms.push_back(uniform);
    }

    parameter = cgGetNextParameter(parameter);
  }

  cgGLLoadProgram(programID);
  return true;
}

ShaderProgram& ShaderProgram::operator = (const ShaderProgram& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

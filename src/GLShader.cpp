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

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLShader.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include <algorithm>

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

Uniform::Type convertUniformType(CGtype type)
{
  switch (type)
  {
    case CG_FLOAT:
      return Uniform::FLOAT;
    case CG_FLOAT2:
      return Uniform::FLOAT_VEC2;
    case CG_FLOAT3:
      return Uniform::FLOAT_VEC3;
    case CG_FLOAT4:
      return Uniform::FLOAT_VEC4;
    case CG_FLOAT2x2:
      return Uniform::FLOAT_MAT2;
    case CG_FLOAT3x3:
      return Uniform::FLOAT_MAT3;
    case CG_FLOAT4x4:
      return Uniform::FLOAT_MAT4;
    default:
      throw Exception("Invalid Cg parameter type");
  }
}

Sampler::Type convertSamplerType(CGtype type)
{
  switch (type)
  {
    case CG_SAMPLER1D:
      return Sampler::SAMPLER_1D;
    case CG_SAMPLER2D:
      return Sampler::SAMPLER_2D;
    case CG_SAMPLER3D:
      return Sampler::SAMPLER_3D;
    case CG_SAMPLERRECT:
      return Sampler::SAMPLER_RECT;
    case CG_SAMPLERCUBE:
      return Sampler::SAMPLER_CUBE;
    default:
      throw Exception("Invalid Cg parameter type");
  }
}

bool isUniformType(CGtype type)
{
  switch (type)
  {
    case CG_FLOAT:
    case CG_FLOAT2:
    case CG_FLOAT3:
    case CG_FLOAT4:
    case CG_FLOAT2x2:
    case CG_FLOAT3x3:
    case CG_FLOAT4x4:
      return true;
  }

  return false;
}

bool isSamplerType(CGtype type)
{
  switch (type)
  {
    case CG_SAMPLER1D:
    case CG_SAMPLER2D:
    case CG_SAMPLER3D:
    case CG_SAMPLERRECT:
    case CG_SAMPLERCUBE:
      return true;
  }

  return false;
}

template <typename T>
class NameComparator
{
public:
  inline NameComparator(const String& name);
  inline bool operator () (const T& object);
  inline bool operator () (const T* object);
private:
  const String& name;
};

template <typename T>
inline NameComparator<T>::NameComparator(const String& initName):
  name(initName)
{
}

template <typename T>
inline bool NameComparator<T>::operator () (const T& object)
{
  return name == object.getName();
}

template <typename T>
inline bool NameComparator<T>::operator () (const T* object)
{
  return name == object->getName();
}

}

///////////////////////////////////////////////////////////////////////

bool Uniform::isScalar(void) const
{
  if (type == FLOAT)
    return true;

  return false;
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

Uniform::Type Uniform::getType(void) const
{
  return type;
}

const String& Uniform::getName(void) const
{
  return name;
}

void Uniform::setValue(float newValue)
{
  if (type != FLOAT)
  {
    Log::writeError("Uniform %s in program %s is not of type float", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter1f((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Vector2& newValue)
{
  if (type != FLOAT_VEC2)
  {
    Log::writeError("Uniform %s in program %s is not of type float2", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter2fv((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Vector3& newValue)
{
  if (type != FLOAT_VEC3)
  {
    Log::writeError("Uniform %s in program %s is not of type float3", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter3fv((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Vector4& newValue)
{
  if (type != FLOAT_VEC4)
  {
    Log::writeError("Uniform %s in program %s is not of type float4", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetParameter4fv((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Matrix2& newValue)
{
  if (type != FLOAT_MAT2)
  {
    Log::writeError("Uniform %s in program %s is not of type float2x2", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetMatrixParameterfr((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Matrix3& newValue)
{
  if (type != FLOAT_MAT3)
  {
    Log::writeError("Uniform %s in program %s is not of type float3x3", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetMatrixParameterfr((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Matrix4& newValue)
{
  if (type != FLOAT_MAT4)
  {
    Log::writeError("Uniform %s in program %s is not of type float2", name.c_str(), program.getName().c_str());
    return;
  }

  cgGLSetMatrixParameterfr((CGparameter) uniformID, newValue);
}

Program& Uniform::getProgram(void) const
{
  return program;
}

Uniform::Uniform(Program& initProgram):
  program(initProgram)
{
}

Uniform::Uniform(const Uniform& source):
  program(source.program)
{
}

Uniform& Uniform::operator = (const Uniform& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

Sampler::Type Sampler::getType(void) const
{
  return type;
}

const String& Sampler::getName(void) const
{
  return name;
}

void Sampler::setTexture(Texture* newTexture)
{
  // TODO: The code.
}

Program& Sampler::getProgram(void) const
{
  return program;
}

Sampler::Sampler(Program& initProgram):
  program(initProgram)
{
}

Sampler::Sampler(const Sampler& source):
  program(source.program)
{
}

Sampler& Sampler::operator = (const Sampler& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

VertexShader::~VertexShader(void)
{
  if (shaderID)
    cgDestroyProgram((CGprogram) shaderID);
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

  shaderID = cgCreateProgram((CGcontext) context.cgContextID,
                             CG_SOURCE,
			     text.c_str(),
			     (CGprofile) context.cgVertexProfile,
			     NULL,
			     NULL);
  if (!shaderID)
  {
    Log::writeError("Failed to compile Cg vertex shader:\n%s\n%s",
                    cgGetErrorString(cgGetError()),
		    cgGetLastListing((CGcontext) context.cgContextID));
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
    cgDestroyProgram((CGprogram) shaderID);
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

  shaderID = cgCreateProgram((CGcontext) context.cgContextID,
                             CG_SOURCE,
			     text.c_str(),
			     (CGprofile) context.cgFragmentProfile,
			     NULL,
			     NULL);
  if (!shaderID)
  {
    Log::writeError("Failed to compile Cg fragment shader:\n%s\n%s",
                    cgGetErrorString(cgGetError()),
		    cgGetLastListing((CGcontext) context.cgContextID));
    return false;
  }

  return true;
}

FragmentShader& FragmentShader::operator = (const FragmentShader& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

void Program::apply(void)
{
  cgGLBindProgram((CGprogram) programID);
}

Uniform* Program::findUniform(const String& name)
{
  UniformList::const_iterator i = std::find_if(uniforms.begin(), uniforms.end(), NameComparator<Uniform>(name));
  if (i == uniforms.end())
    return NULL;

  return *i;
}

const Uniform* Program::findUniform(const String& name) const
{
  UniformList::const_iterator i = std::find_if(uniforms.begin(), uniforms.end(), NameComparator<Uniform>(name));
  if (i == uniforms.end())
    return NULL;

  return *i;
}

Sampler* Program::findSampler(const String& name)
{
  SamplerList::const_iterator i = std::find_if(samplers.begin(), samplers.end(), NameComparator<Sampler>(name));
  if (i == samplers.end())
    return NULL;

  return *i;
}

const Sampler* Program::findSampler(const String& name) const
{
  SamplerList::const_iterator i = std::find_if(samplers.begin(), samplers.end(), NameComparator<Sampler>(name));
  if (i == samplers.end())
    return NULL;

  return *i;
}

unsigned int Program::getUniformCount(void) const
{
  return uniforms.size();
}

Uniform& Program::getUniform(unsigned int index)
{
  return *uniforms[index];
}

const Uniform& Program::getUniform(unsigned int index) const
{
  return *uniforms[index];
}

unsigned int Program::getSamplerCount(void) const
{
  return samplers.size();
}

Sampler& Program::getSampler(unsigned int index)
{
  return *samplers[index];
}

const Sampler& Program::getSampler(unsigned int index) const
{
  return *samplers[index];
}

VertexShader& Program::getVertexShader(void) const
{
  return *vertexShader;
}

FragmentShader& Program::getFragmentShader(void) const
{
  return *fragmentShader;
}

Program* Program::createInstance(Context& context,
                                             VertexShader& vertexShader,
					     FragmentShader& fragmentShader,
					     const String& name)
{
  Ptr<Program> program = new Program(context, name);
  if (!program->init(vertexShader, fragmentShader))
    return NULL;

  return program.detachObject();
}

Program::Program(Context& initContext, const String& name):
  Resource<Program>(name),
  context(initContext)
{
}

Program::Program(const Program& source):
  Resource<Program>(""),
  context(source.context)
{
}

bool Program::init(VertexShader& initVertexShader, FragmentShader& initFragmentShader)
{
  vertexShader = &initVertexShader;
  fragmentShader = &initFragmentShader;

  programID = cgCombinePrograms2((CGprogram) vertexShader->shaderID,
                                 (CGprogram) fragmentShader->shaderID);
  if (!programID)
  {
    Log::writeError("Unable to combine shaders for program %s", getName().c_str());
    return false;
  }

  CGparameter parameter = cgGetFirstParameter((CGprogram) programID, CG_PROGRAM);

  while (parameter)
  {
    CGtype type = cgGetParameterType(parameter);
    if (type != CG_ARRAY && type != CG_STRUCT)
    {
      if (isSamplerType(type))
      {
	Sampler* sampler = new Sampler(*this);
	sampler->name = cgGetParameterName(parameter);
	sampler->type = convertSamplerType(type);
	sampler->samplerID = parameter;

	samplers.push_back(sampler);
      }
      else if (isUniformType(type))
      {
	Uniform* uniform = new Uniform(*this);
	uniform->name = cgGetParameterName(parameter);
	uniform->type = convertUniformType(type);
	uniform->uniformID = parameter;

	uniforms.push_back(uniform);
      }
      else
	Log::writeWarning("Ignoring shader uniform %s", cgGetParameterName(parameter));
    }

    parameter = cgGetNextParameter(parameter);
  }

  cgGLLoadProgram((CGprogram) programID);
  return true;
}

Program& Program::operator = (const Program& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

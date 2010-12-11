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

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLint getElementCount(Varying::Type type)
{
  switch (type)
  {
    case Varying::FLOAT:
      return 1;
    case Varying::FLOAT_VEC2:
      return 2;
    case Varying::FLOAT_VEC3:
      return 3;
    case Varying::FLOAT_VEC4:
      return 4;
    default:
      throw Exception("Invalid varying parameter type");
  }
}

GLenum getVaryingBaseType(Varying::Type type)
{
  switch (type)
  {
    case Varying::FLOAT:
    case Varying::FLOAT_VEC2:
    case Varying::FLOAT_VEC3:
    case Varying::FLOAT_VEC4:
      return GL_FLOAT;
    default:
      throw Exception("Invalid varying parameter type");
  }
}

Varying::Type convertVaryingType(CGtype type)
{
  switch (type)
  {
    case CG_FLOAT:
      return Varying::FLOAT;
    case CG_FLOAT2:
      return Varying::FLOAT_VEC2;
    case CG_FLOAT3:
      return Varying::FLOAT_VEC3;
    case CG_FLOAT4:
      return Varying::FLOAT_VEC4;
    default:
      throw Exception("Invalid Cg varying parameter type");
  }
}

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
      throw Exception("Invalid Cg uniform parameter type");
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
      throw Exception("Invalid Cg sampler parameter type");
  }
}

const char* getTypeName(Uniform::Type type)
{
  switch (type)
  {
    case Uniform::FLOAT:
      return "float";
    case Uniform::FLOAT_VEC2:
      return "float2";
    case Uniform::FLOAT_VEC3:
      return "float3";
    case Uniform::FLOAT_VEC4:
      return "float4";
    case Uniform::FLOAT_MAT2:
      return "float2x2";
    case Uniform::FLOAT_MAT3:
      return "float3x3";
    case Uniform::FLOAT_MAT4:
      return "float4x4";
    default:
      throw Exception("Invalid uniform type");
  }
}

const char* getTypeName(Sampler::Type type)
{
  switch (type)
  {
    case Sampler::SAMPLER_1D:
      return "sampler1D";
    case Sampler::SAMPLER_2D:
      return "sampler2D";
    case Sampler::SAMPLER_3D:
      return "sampler3D";
    case Sampler::SAMPLER_RECT:
      return "samplerRECT";
    case Sampler::SAMPLER_CUBE:
      return "samplerCUBE";
    default:
      throw Exception("Invalid sampler type");
  }
}

const char* getTypeName(Varying::Type type)
{
  switch (type)
  {
    case Varying::FLOAT:
      return "float";
    case Varying::FLOAT_VEC2:
      return "float2";
    case Varying::FLOAT_VEC3:
      return "float3";
    case Varying::FLOAT_VEC4:
      return "float4";
    default:
      throw Exception("Invalid varying type");
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
    default:
      return false;
  }
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
    default:
      return false;
  }
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

const unsigned int PROGRAM_XML_VERSION = 3;

}

///////////////////////////////////////////////////////////////////////

bool Varying::isScalar(void) const
{
  return type == FLOAT;
}

bool Varying::isVector(void) const
{
  if (type == FLOAT_VEC2 || type == FLOAT_VEC3 || type == FLOAT_VEC4)
    return true;

  return false;
}

Varying::Type Varying::getType(void) const
{
  return type;
}

const String& Varying::getName(void) const
{
  return name;
}

void Varying::enable(size_t stride, size_t offset)
{
  cgGLEnableClientState((CGparameter) varyingID);

#if WENDY_DEBUG
  checkCg("Failed to enable varying \'%s\' of program \'%s\'",
          name.c_str(),
          program.getPath().asString().c_str());
#endif

  cgGLSetParameterPointer((CGparameter) varyingID,
                          getElementCount(type),
			  getVaryingBaseType(type),
			  stride,
			  (const void*) offset);

#if WENDY_DEBUG
  checkCg("Failed to set varying \'%s\' of program \'%s\'",
          name.c_str(),
          program.getPath().asString().c_str());
#endif
}

void Varying::disable(void)
{
  cgGLDisableClientState((CGparameter) varyingID);

#if WENDY_DEBUG
  checkCg("Failed to disable varying \'%s\' of program \'%s\'",
          name.c_str(),
          program.getPath().asString().c_str());
#endif
}

Program& Varying::getProgram(void) const
{
  return program;
}

Varying::Varying(Program& initProgram):
  program(initProgram)
{
}

Varying::Varying(const Varying& source):
  program(source.program)
{
}

Varying& Varying::operator = (const Varying& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

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
    Log::writeError("Uniform \'%s\' in program \'%s\' is not of type float",
                    name.c_str(),
                    program.getPath().asString().c_str());
    return;
  }

  cgGLSetParameter1f((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Vec2& newValue)
{
  if (type != FLOAT_VEC2)
  {
    Log::writeError("Uniform \'%s\' in program \'%s\' is not of type float2",
                    name.c_str(),
                    program.getPath().asString().c_str());
    return;
  }

  cgGLSetParameter2fv((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Vec3& newValue)
{
  if (type != FLOAT_VEC3)
  {
    Log::writeError("Uniform \'%s\' in program \'%s\' is not of type float3",
                    name.c_str(),
                    program.getPath().asString().c_str());
    return;
  }

  cgGLSetParameter3fv((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Vec4& newValue)
{
  if (type != FLOAT_VEC4)
  {
    Log::writeError("Uniform \'%s\' in program \'%s\' is not of type float4",
                    name.c_str(),
                    program.getPath().asString().c_str());
    return;
  }

  cgGLSetParameter4fv((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Mat2& newValue)
{
  if (type != FLOAT_MAT2)
  {
    Log::writeError("Uniform \'%s\' in program \'%s\' is not of type float2x2",
                    name.c_str(),
                    program.getPath().asString().c_str());
    return;
  }

  cgGLSetMatrixParameterfc((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Mat3& newValue)
{
  if (type != FLOAT_MAT3)
  {
    Log::writeError("Uniform \'%s\' in program \'%s\' is not of type float3x3",
                    name.c_str(),
                    program.getPath().asString().c_str());
    return;
  }

  cgGLSetMatrixParameterfc((CGparameter) uniformID, newValue);
}

void Uniform::setValue(const Mat4& newValue)
{
  if (type != FLOAT_MAT4)
  {
    Log::writeError("Uniform \'%s\' in program \'%s\' is not of type float4x4",
                    name.c_str(),
                    program.getPath().asString().c_str());
    return;
  }

  cgGLSetMatrixParameterfc((CGparameter) uniformID, newValue);
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

void Sampler::setTexture(Texture& newTexture)
{
  cgGLSetTextureParameter((CGparameter) samplerID, newTexture.textureID);

#if WENDY_DEBUG
  checkCg("Failed to set sampler \'%s\' of program \'%s\' to texture \'%s\'",
          name.c_str(),
          program.getPath().asString().c_str(),
          newTexture.getPath().asString().c_str());
#endif
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

VertexProgram::~VertexProgram(void)
{
  if (programID)
    cgDestroyProgram((CGprogram) programID);
}

const String& VertexProgram::getText(void) const
{
  return text;
}

Ref<VertexProgram> VertexProgram::create(const ResourceInfo& info,
                                         Context& context,
                                         const String& text)
{
  Ref<VertexProgram> program(new VertexProgram(info, context));
  if (!program->init(text))
    return NULL;

  return program;
}

Ref<VertexProgram> VertexProgram::read(Context& context, const Path& path)
{
  VertexProgramReader reader(context);
  return reader.read(path);
}

VertexProgram::VertexProgram(const ResourceInfo& info, Context& initContext):
  Resource(info),
  context(initContext),
  programID(NULL)
{
}

VertexProgram::VertexProgram(const VertexProgram& source):
  Resource(source),
  context(source.context)
{
}

bool VertexProgram::init(const String& initText)
{
  text = initText;

  programID = cgCreateProgram((CGcontext) context.cgContextID,
                              CG_SOURCE,
			      text.c_str(),
			      (CGprofile) context.cgVertexProfile,
			      NULL,
			      NULL);
  if (!programID)
  {
    Log::writeError("Failed to compile Cg vertex program \'%s\':\n%s\n%s",
                    getPath().asString().c_str(),
                    cgGetErrorString(cgGetError()),
		    cgGetLastListing((CGcontext) context.cgContextID));
    return false;
  }

  return true;
}

VertexProgram& VertexProgram::operator = (const VertexProgram& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

FragmentProgram::~FragmentProgram(void)
{
  if (programID)
    cgDestroyProgram((CGprogram) programID);
}

const String& FragmentProgram::getText(void) const
{
  return text;
}

Ref<FragmentProgram> FragmentProgram::create(const ResourceInfo& info,
                                             Context& context,
                                             const String& text)
{
  Ref<FragmentProgram> program(new FragmentProgram(info, context));
  if (!program->init(text))
    return NULL;

  return program;
}

Ref<FragmentProgram> FragmentProgram::read(Context& context, const Path& path)
{
  FragmentProgramReader reader(context);
  return reader.read(path);
}

FragmentProgram::FragmentProgram(const ResourceInfo& info, Context& initContext):
  Resource(info),
  context(initContext),
  programID(NULL)
{
}

FragmentProgram::FragmentProgram(const FragmentProgram& source):
  Resource(source),
  context(source.context)
{
}

bool FragmentProgram::init(const String& initText)
{
  text = initText;

  programID = cgCreateProgram((CGcontext) context.cgContextID,
                              CG_SOURCE,
			      text.c_str(),
			      (CGprofile) context.cgFragmentProfile,
			      NULL,
			      NULL);
  if (!programID)
  {
    Log::writeError("Failed to compile Cg fragment program \'%s\':\n%s\n%s",
                    getPath().asString().c_str(),
                    cgGetErrorString(cgGetError()),
		    cgGetLastListing((CGcontext) context.cgContextID));
    return false;
  }

  return true;
}

FragmentProgram& FragmentProgram::operator = (const FragmentProgram& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

Program::~Program(void)
{
  if (current == this)
    current = NULL;

  while (!varyings.empty())
  {
    delete varyings.back();
    varyings.pop_back();
  }

  while (!uniforms.empty())
  {
    delete uniforms.back();
    uniforms.pop_back();
  }

  while (!samplers.empty())
  {
    delete samplers.back();
    samplers.pop_back();
  }
}

Varying* Program::findVarying(const String& name)
{
  VaryingList::const_iterator i = std::find_if(varyings.begin(),
                                               varyings.end(),
                                               NameComparator<Varying>(name));
  if (i == varyings.end())
    return NULL;

  return *i;
}

const Varying* Program::findVarying(const String& name) const
{
  VaryingList::const_iterator i = std::find_if(varyings.begin(),
                                               varyings.end(),
                                               NameComparator<Varying>(name));
  if (i == varyings.end())
    return NULL;

  return *i;
}

Uniform* Program::findUniform(const String& name)
{
  UniformList::const_iterator i = std::find_if(uniforms.begin(),
                                               uniforms.end(),
                                               NameComparator<Uniform>(name));
  if (i == uniforms.end())
    return NULL;

  return *i;
}

const Uniform* Program::findUniform(const String& name) const
{
  UniformList::const_iterator i = std::find_if(uniforms.begin(),
                                               uniforms.end(),
                                               NameComparator<Uniform>(name));
  if (i == uniforms.end())
    return NULL;

  return *i;
}

Sampler* Program::findSampler(const String& name)
{
  SamplerList::const_iterator i = std::find_if(samplers.begin(),
                                               samplers.end(),
                                               NameComparator<Sampler>(name));
  if (i == samplers.end())
    return NULL;

  return *i;
}

const Sampler* Program::findSampler(const String& name) const
{
  SamplerList::const_iterator i = std::find_if(samplers.begin(),
                                               samplers.end(),
                                               NameComparator<Sampler>(name));
  if (i == samplers.end())
    return NULL;

  return *i;
}

unsigned int Program::getVaryingCount(void) const
{
  return varyings.size();
}

Varying& Program::getVarying(unsigned int index)
{
  return *varyings[index];
}

const Varying& Program::getVarying(unsigned int index) const
{
  return *varyings[index];
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

VertexProgram& Program::getVertexProgram(void) const
{
  return *vertexProgram;
}

FragmentProgram& Program::getFragmentProgram(void) const
{
  return *fragmentProgram;
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             VertexProgram& vertexProgram,
			     FragmentProgram& fragmentProgram)
{
  Ref<Program> program(new Program(info, context));
  if (!program->init(vertexProgram, fragmentProgram))
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
  context(source.context)
{
}

bool Program::init(VertexProgram& initVertexProgram, FragmentProgram& initFragmentProgram)
{
  vertexProgram = &initVertexProgram;
  fragmentProgram = &initFragmentProgram;

  cgGLLoadProgram((CGprogram) vertexProgram->programID);

  if (!checkCg("Failed to load vertex program \'%s\'", vertexProgram->getPath().asString().c_str()))
    return false;

  cgGLLoadProgram((CGprogram) fragmentProgram->programID);

  if (!checkCg("Failed to load fragment program \'%s\'", fragmentProgram->getPath().asString().c_str()))
    return false;

  CGparameter parameter;

  parameter = cgGetFirstLeafParameter((CGprogram) vertexProgram->programID,
                                      CG_PROGRAM);

  while (parameter)
  {
    if (cgGetParameterResource(parameter) != CG_UNDEFINED)
    {
      CGenum variability = cgGetParameterVariability(parameter);
      if (variability == CG_VARYING)
      {
	if (cgGetParameterDirection(parameter) != CG_IN)
	  continue;

	CGtype type = cgGetParameterType(parameter);

	Varying* varying = new Varying(*this);
	varying->name = cgGetParameterName(parameter);
	varying->type = convertVaryingType(type);
	varying->varyingID = parameter;

	varyings.push_back(varying);
      }
      else if (variability == CG_UNIFORM)
      {
	CGtype type = cgGetParameterType(parameter);

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
	  Log::writeError("Unknown Cg parameter type");
      }
    }

    parameter = cgGetNextLeafParameter(parameter);
  }

  parameter = cgGetFirstLeafParameter((CGprogram) fragmentProgram->programID,
                                      CG_PROGRAM);

  while (parameter)
  {
    if (cgGetParameterResource(parameter) != CG_UNDEFINED)
    {
      CGenum variability = cgGetParameterVariability(parameter);
      if (variability == CG_UNIFORM)
      {
	CGtype type = cgGetParameterType(parameter);

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
	  Log::writeError("Unknown Cg parameter type");
      }
    }

    parameter = cgGetNextLeafParameter(parameter);
  }

  return true;
}

void Program::apply(void) const
{
  if (current == this)
    return;

  cgGLBindProgram((CGprogram) vertexProgram->programID);
  cgGLBindProgram((CGprogram) fragmentProgram->programID);

  current = const_cast<Program*>(this);
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

void ProgramInterface::addSampler(const String& name, Sampler::Type type)
{
  samplers.push_back(SamplerList::value_type(name, type));
}

void ProgramInterface::addVarying(const String& name, Varying::Type type)
{
  varyings.push_back(VaryingList::value_type(name, type));
}

bool ProgramInterface::matches(const Program& program, bool verbose) const
{
  for (unsigned int i = 0;  i < uniforms.size();  i++)
  {
    const UniformList::value_type& entry = uniforms[i];

    const Uniform* uniform = program.findUniform(entry.first);
    if (!uniform)
    {
      if (verbose)
      {
	Log::writeError("Uniform \'%s\' missing in program \'%s\'",
	                entry.first.c_str(),
			program.getPath().asString().c_str());
      }

      return false;
    }

    if (uniform->getType() != entry.second)
    {
      if (verbose)
      {
	Log::writeError("Uniform \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
	                entry.first.c_str(),
			program.getPath().asString().c_str(),
			getTypeName(entry.second));
      }

      return false;
    }
  }

  for (unsigned int i = 0;  i < samplers.size();  i++)
  {
    const SamplerList::value_type& entry = samplers[i];

    const Sampler* sampler = program.findSampler(entry.first);
    if (!sampler)
    {
      if (verbose)
      {
	Log::writeError("Sampler \'%s\' missing in program \'%s\'",
	                entry.first.c_str(),
			program.getPath().asString().c_str());
      }

      return false;
    }

    if (sampler->getType() != entry.second)
    {
      if (verbose)
      {
	Log::writeError("Sampler \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
	                entry.first.c_str(),
			program.getPath().asString().c_str(),
			getTypeName(entry.second));
      }

      return false;
    }
  }

  for (unsigned int i = 0;  i < varyings.size();  i++)
  {
    const VaryingList::value_type& entry = varyings[i];

    const Varying* varying = program.findVarying(entry.first);
    if (!varying)
    {
      if (verbose)
      {
	Log::writeError("Varying \'%s\' missing in program \'%s\'",
	                entry.first.c_str(),
			program.getPath().asString().c_str());
      }

      return false;
    }

    if (varying->getType() != entry.second)
    {
      if (verbose)
      {
	Log::writeError("Varying \'%s\' in program \'%s\' has incorrect type; should be \'%s\'",
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
  if (format.getComponentCount() != varyings.size())
    return false;

  for (unsigned int i = 0;  i < varyings.size();  i++)
  {
    const VaryingList::value_type& entry = varyings[i];

    const VertexComponent* component = format.findComponent(entry.first);
    if (!component)
      return false;

    if (component->getType() != VertexComponent::FLOAT32)
      return false;

    if ((component->getElementCount() == 1 && entry.second != Varying::FLOAT) ||
        (component->getElementCount() == 2 && entry.second != Varying::FLOAT_VEC2) ||
        (component->getElementCount() == 3 && entry.second != Varying::FLOAT_VEC3) ||
        (component->getElementCount() == 4 && entry.second != Varying::FLOAT_VEC4))
      return false;
  }

  return true;
}

Program* Program::current = NULL;

///////////////////////////////////////////////////////////////////////

VertexProgramReader::VertexProgramReader(Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext)
{
}

Ref<VertexProgram> VertexProgramReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<VertexProgram*>(cache);

  ResourceInfo info(getIndex(), path);

  std::ifstream stream;
  if (!open(stream, info.path))
    return NULL;

  stream.seekg(0, std::ios::end);

  String text;
  text.resize(stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());

  return VertexProgram::create(info, context, text);
}

///////////////////////////////////////////////////////////////////////

FragmentProgramReader::FragmentProgramReader(Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext)
{
}

Ref<FragmentProgram> FragmentProgramReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<FragmentProgram*>(cache);

  ResourceInfo info(getIndex(), path);

  std::ifstream stream;
  if (!open(stream, info.path))
    return NULL;

  stream.seekg(0, std::ios::end);

  String text;
  text.resize(stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());

  return FragmentProgram::create(info, context, text);
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
  if (!open(stream, info.path))
    return NULL;

  if (!XML::Reader::read(stream))
  {
    vertexProgram = NULL;
    fragmentProgram = NULL;
    program = NULL;
    return NULL;
  }

  vertexProgram = NULL;
  fragmentProgram = NULL;

  if (!program)
  {
    Log::writeError("No shader program specification found in file");
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
      Log::writeError("Only one shader program per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != PROGRAM_XML_VERSION)
    {
      Log::writeError("Shader program XML format version mismatch");
      return false;
    }

    return true;
  }

  if (name == "vertex")
  {
    if (vertexProgram)
    {
      Log::writeError("Cannot nest vertex programs");
      return false;
    }

    Path vertexProgramPath(readString("path"));
    if (vertexProgramPath.isEmpty())
    {
      Log::writeError("Vertex program name in shader program \'%s\' is empty",
                      info.path.asString().c_str());
      return true;
    }

    VertexProgramReader reader(context);
    vertexProgram = reader.read(vertexProgramPath);
    if (!vertexProgram)
    {
      Log::writeError("Cannot find vertex program \'%s\' for shader program \'%s\'",
                      vertexProgramPath.asString().c_str(),
		      info.path.asString().c_str());
      return false;
    }

    return true;
  }

  if (name == "fragment")
  {
    if (fragmentProgram)
    {
      Log::writeError("Cannot nest fragment programs");
      return false;
    }

    Path fragmentProgramPath(readString("path"));
    if (fragmentProgramPath.isEmpty())
    {
      Log::writeError("Fragment program name in shader program \'%s\' is empty",
                      info.path.asString().c_str());
      return true;
    }

    FragmentProgramReader reader(context);
    fragmentProgram = reader.read(fragmentProgramPath);
    if (!fragmentProgram)
    {
      Log::writeError("Cannot find fragment program \'%s\' for shader program \'%s\'",
                      fragmentProgramPath.asString().c_str(),
		      info.path.asString().c_str());
      return false;
    }

    return true;
  }

  return true;
}

bool ProgramReader::onEndElement(const String& name)
{
  if (name == "program")
  {
    if (!vertexProgram)
    {
      Log::writeError("Vertex program missing for shader program \'%s\'",
		      info.path.asString().c_str());
      return false;
    }

    if (!fragmentProgram)
    {
      Log::writeError("Fragment program missing for shader program \'%s\'",
		      info.path.asString().c_str());
      return false;
    }

    program = Program::create(info, context, *vertexProgram, *fragmentProgram);
    if (!program)
      return false;

    vertexProgram = NULL;
    fragmentProgram = NULL;

    return true;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

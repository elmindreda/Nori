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

const unsigned int PROGRAM_XML_VERSION = 3;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

VertexProgram::VertexProgram(void)
{
}

VertexProgram::VertexProgram(const String& initText, const Path& initPath):
  text(initText),
  path(initPath)
{
}

///////////////////////////////////////////////////////////////////////

FragmentProgram::FragmentProgram(void)
{
}

FragmentProgram::FragmentProgram(const String& initText, const Path& initPath):
  text(initText),
  path(initPath)
{
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
          program->getPath().asString().c_str());
#endif

  cgGLSetParameterPointer((CGparameter) varyingID,
                          getElementCount(type),
			  getVaryingBaseType(type),
			  stride,
			  (const void*) offset);

#if WENDY_DEBUG
  checkCg("Failed to set varying \'%s\' of program \'%s\'",
          name.c_str(),
          program->getPath().asString().c_str());
#endif
}

void Varying::disable(void)
{
  cgGLDisableClientState((CGparameter) varyingID);

#if WENDY_DEBUG
  checkCg("Failed to disable varying \'%s\' of program \'%s\'",
          name.c_str(),
          program->getPath().asString().c_str());
#endif
}

Program& Varying::getProgram(void) const
{
  return *program;
}

Varying::Varying(Program& initProgram):
  program(&initProgram)
{
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
                    program->getPath().asString().c_str());
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
                    program->getPath().asString().c_str());
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
                    program->getPath().asString().c_str());
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
                    program->getPath().asString().c_str());
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
                    program->getPath().asString().c_str());
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
                    program->getPath().asString().c_str());
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
                    program->getPath().asString().c_str());
    return;
  }

  cgGLSetMatrixParameterfc((CGparameter) uniformID, newValue);
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
          program->getPath().asString().c_str(),
          newTexture.getPath().asString().c_str());
#endif
}

Program& Sampler::getProgram(void) const
{
  return *program;
}

Sampler::Sampler(Program& initProgram):
  program(&initProgram)
{
}

///////////////////////////////////////////////////////////////////////

Program::~Program(void)
{
  if (vertexProgramID)
    cgDestroyProgram((CGprogram) vertexProgramID);

  if (fragmentProgramID)
    cgDestroyProgram((CGprogram) fragmentProgramID);
}

Varying* Program::findVarying(const String& name)
{
  VaryingList::iterator i = std::find_if(varyings.begin(),
                                         varyings.end(),
                                         NameComparator<Varying>(name));
  if (i == varyings.end())
    return NULL;

  return &(*i);
}

const Varying* Program::findVarying(const String& name) const
{
  VaryingList::const_iterator i = std::find_if(varyings.begin(),
                                               varyings.end(),
                                               NameComparator<Varying>(name));
  if (i == varyings.end())
    return NULL;

  return &(*i);
}

Uniform* Program::findUniform(const String& name)
{
  UniformList::iterator i = std::find_if(uniforms.begin(),
                                         uniforms.end(),
                                         NameComparator<Uniform>(name));
  if (i == uniforms.end())
    return NULL;

  return &(*i);
}

const Uniform* Program::findUniform(const String& name) const
{
  UniformList::const_iterator i = std::find_if(uniforms.begin(),
                                               uniforms.end(),
                                               NameComparator<Uniform>(name));
  if (i == uniforms.end())
    return NULL;

  return &(*i);
}

Sampler* Program::findSampler(const String& name)
{
  SamplerList::iterator i = std::find_if(samplers.begin(),
                                         samplers.end(),
                                         NameComparator<Sampler>(name));
  if (i == samplers.end())
    return NULL;

  return &(*i);
}

const Sampler* Program::findSampler(const String& name) const
{
  SamplerList::const_iterator i = std::find_if(samplers.begin(),
                                               samplers.end(),
                                               NameComparator<Sampler>(name));
  if (i == samplers.end())
    return NULL;

  return &(*i);
}

unsigned int Program::getVaryingCount(void) const
{
  return varyings.size();
}

Varying& Program::getVarying(unsigned int index)
{
  return varyings[index];
}

const Varying& Program::getVarying(unsigned int index) const
{
  return varyings[index];
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

unsigned int Program::getSamplerCount(void) const
{
  return samplers.size();
}

Sampler& Program::getSampler(unsigned int index)
{
  return samplers[index];
}

const Sampler& Program::getSampler(unsigned int index) const
{
  return samplers[index];
}

Ref<Program> Program::create(const ResourceInfo& info,
                             Context& context,
                             const VertexProgram& vertexProgram,
			     const FragmentProgram& fragmentProgram)
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
  context(source.context),
  vertexProgramID(0),
  fragmentProgramID(0)
{
}

bool Program::init(const VertexProgram& vertexProgram,
                   const FragmentProgram& fragmentProgram)
{
  vertexProgramID = cgCreateProgram((CGcontext) context.cgContextID,
                                    CG_SOURCE,
			            vertexProgram.text.c_str(),
			            (CGprofile) context.cgVertexProfile,
			            NULL,
			            NULL);
  if (!vertexProgramID)
  {
    Log::writeError("Failed to compile Cg vertex program \'%s\':\n%s\n%s",
                    vertexProgram.path.asString().c_str(),
                    cgGetErrorString(cgGetError()),
		    cgGetLastListing((CGcontext) context.cgContextID));
    return false;
  }

  fragmentProgramID = cgCreateProgram((CGcontext) context.cgContextID,
                                      CG_SOURCE,
			              fragmentProgram.text.c_str(),
			              (CGprofile) context.cgFragmentProfile,
			              NULL,
			              NULL);
  if (!fragmentProgramID)
  {
    Log::writeError("Failed to compile Cg fragment program \'%s\':\n%s\n%s",
                    fragmentProgram.path.asString().c_str(),
                    cgGetErrorString(cgGetError()),
		    cgGetLastListing((CGcontext) context.cgContextID));
    return false;
  }

  cgGLLoadProgram((CGprogram) vertexProgramID);

  if (!checkCg("Failed to load vertex program \'%s\'",
               vertexProgram.path.asString().c_str()))
  {
    return false;
  }

  cgGLLoadProgram((CGprogram) fragmentProgramID);

  if (!checkCg("Failed to load fragment program \'%s\'",
               fragmentProgram.path.asString().c_str()))
  {
    return false;
  }

  CGparameter parameter;

  parameter = cgGetFirstLeafParameter((CGprogram) vertexProgramID, CG_PROGRAM);

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

        varyings.push_back(Varying(*this));
	Varying& varying = varyings.back();
	varying.name = cgGetParameterName(parameter);
	varying.type = convertVaryingType(type);
	varying.varyingID = parameter;
      }
      else if (variability == CG_UNIFORM)
      {
	CGtype type = cgGetParameterType(parameter);

	if (isSamplerType(type))
	{
          samplers.push_back(Sampler(*this));
	  Sampler& sampler = samplers.back();
	  sampler.name = cgGetParameterName(parameter);
	  sampler.type = convertSamplerType(type);
	  sampler.samplerID = parameter;
	}
	else if (isUniformType(type))
	{
          uniforms.push_back(Uniform(*this));
	  Uniform& uniform = uniforms.back();
	  uniform.name = cgGetParameterName(parameter);
	  uniform.type = convertUniformType(type);
	  uniform.uniformID = parameter;
	}
	else
	  Log::writeError("Unknown Cg parameter type");
      }
    }

    parameter = cgGetNextLeafParameter(parameter);
  }

  parameter = cgGetFirstLeafParameter((CGprogram) fragmentProgramID, CG_PROGRAM);

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
          samplers.push_back(Sampler(*this));
	  Sampler& sampler = samplers.back();
	  sampler.name = cgGetParameterName(parameter);
	  sampler.type = convertSamplerType(type);
	  sampler.samplerID = parameter;
	}
	else if (isUniformType(type))
	{
          uniforms.push_back(Uniform(*this));
	  Uniform& uniform = uniforms.back();
	  uniform.name = cgGetParameterName(parameter);
	  uniform.type = convertUniformType(type);
	  uniform.uniformID = parameter;
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
  cgGLBindProgram((CGprogram) vertexProgramID);
  cgGLBindProgram((CGprogram) fragmentProgramID);
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
  for (int i = 0;  i < uniforms.size();  i++)
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

  for (int i = 0;  i < samplers.size();  i++)
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

  for (int i = 0;  i < varyings.size();  i++)
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

  for (int i = 0;  i < varyings.size();  i++)
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
      Log::writeError("Program specification \'%s\' contains more than one vertex program",
                      info.path.asString().c_str());
      return false;
    }

    Path path(readString("path"));
    if (path.isEmpty())
    {
      Log::writeError("Vertex program path in shader program \'%s\' is empty",
                      info.path.asString().c_str());
      return true;
    }

    String text;
    if (!readTextFile(text, path))
    {
      Log::writeError("Cannot find vertex program \'%s\' for shader program \'%s\'",
                      path.asString().c_str(),
		      info.path.asString().c_str());
      return false;
    }

    vertexProgram = new VertexProgram(text, path);
    return true;
  }

  if (name == "fragment")
  {
    if (fragmentProgram)
    {
      Log::writeError("Program specification \'%s\' contains more than one fragment program",
                      info.path.asString().c_str());
      return false;
    }

    Path path(readString("path"));
    if (path.isEmpty())
    {
      Log::writeError("Fragment program path in shader program \'%s\' is empty",
                      info.path.asString().c_str());
      return true;
    }

    String text;
    if (!readTextFile(text, path))
    {
      Log::writeError("Cannot find fragment program \'%s\' for shader program \'%s\'",
                      path.asString().c_str(),
		      info.path.asString().c_str());
      return false;
    }

    fragmentProgram = new FragmentProgram(text, path);
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

bool ProgramReader::readTextFile(String& text, const Path& path)
{
  std::ifstream stream;
  if (!open(stream, path))
    return false;

  stream.seekg(0, std::ios::end);

  text.resize(stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

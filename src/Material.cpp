///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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

#include <wendy/Config.hpp>

#include <wendy/Bimap.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>
#include <wendy/Pass.hpp>
#include <wendy/Material.hpp>

#include <algorithm>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

Bimap<String, CullMode> cullModeMap;
Bimap<String, BlendFactor> blendFactorMap;
Bimap<String, FragmentFunction> functionMap;
Bimap<String, StencilOp> operationMap;
Bimap<String, FilterMode> filterModeMap;
Bimap<String, AddressMode> addressModeMap;
Bimap<String, RenderPhase> phaseMap;

Bimap<SamplerType, TextureType> textureTypeMap;

const uint MATERIAL_XML_VERSION = 9;

void initializeMaps()
{
  if (cullModeMap.isEmpty())
  {
    cullModeMap["none"] = CULL_NONE;
    cullModeMap["front"] = CULL_FRONT;
    cullModeMap["back"] = CULL_BACK;
    cullModeMap["both"] = CULL_BOTH;
  }

  if (blendFactorMap.isEmpty())
  {
    blendFactorMap["zero"] = BLEND_ZERO;
    blendFactorMap["one"] = BLEND_ONE;
    blendFactorMap["src color"] = BLEND_SRC_COLOR;
    blendFactorMap["dst color"] = BLEND_DST_COLOR;
    blendFactorMap["src alpha"] = BLEND_SRC_ALPHA;
    blendFactorMap["dst alpha"] = BLEND_DST_ALPHA;
    blendFactorMap["one minus src color"] = BLEND_ONE_MINUS_SRC_COLOR;
    blendFactorMap["one minus dst color"] = BLEND_ONE_MINUS_DST_COLOR;
    blendFactorMap["one minus src alpha"] = BLEND_ONE_MINUS_SRC_ALPHA;
    blendFactorMap["one minus dst alpha"] = BLEND_ONE_MINUS_DST_ALPHA;
  }

  if (functionMap.isEmpty())
  {
    functionMap["never"] = ALLOW_NEVER;
    functionMap["always"] = ALLOW_ALWAYS;
    functionMap["equal"] = ALLOW_EQUAL;
    functionMap["not equal"] = ALLOW_NOT_EQUAL;
    functionMap["lesser"] = ALLOW_LESSER;
    functionMap["lesser or equal"] = ALLOW_LESSER_EQUAL;
    functionMap["greater"] = ALLOW_GREATER;
    functionMap["greater or equal"] = ALLOW_GREATER_EQUAL;
  }

  if (operationMap.isEmpty())
  {
    operationMap["keep"] = STENCIL_KEEP;
    operationMap["zero"] = STENCIL_ZERO;
    operationMap["replace"] = STENCIL_REPLACE;
    operationMap["increase"] = STENCIL_INCREASE;
    operationMap["decrease"] = STENCIL_DECREASE;
    operationMap["invert"] = STENCIL_INVERT;
    operationMap["increase wrap"] = STENCIL_INCREASE_WRAP;
    operationMap["decrease wrap"] = STENCIL_DECREASE_WRAP;
  }

  if (addressModeMap.isEmpty())
  {
    addressModeMap["wrap"] = ADDRESS_WRAP;
    addressModeMap["clamp"] = ADDRESS_CLAMP;
  }

  if (filterModeMap.isEmpty())
  {
    filterModeMap["nearest"] = FILTER_NEAREST;
    filterModeMap["bilinear"] = FILTER_BILINEAR;
    filterModeMap["trilinear"] = FILTER_TRILINEAR;
  }

  if (textureTypeMap.isEmpty())
  {
    textureTypeMap[SAMPLER_1D] = TEXTURE_1D;
    textureTypeMap[SAMPLER_2D] = TEXTURE_2D;
    textureTypeMap[SAMPLER_3D] = TEXTURE_3D;
    textureTypeMap[SAMPLER_RECT] = TEXTURE_RECT;
    textureTypeMap[SAMPLER_CUBE] = TEXTURE_CUBE;
  }

  if (phaseMap.isEmpty())
  {
    phaseMap[""] = RENDER_DEFAULT;
    phaseMap["default"] = RENDER_DEFAULT;
    phaseMap["shadowmap"] = RENDER_SHADOWMAP;
  }
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool parsePass(RenderContext& context, Pass& pass, pugi::xml_node root)
{
  initializeMaps();

  ResourceCache& cache = context.cache();

  if (pugi::xml_node node = root.child("blending"))
  {
    if (pugi::xml_attribute a = node.attribute("src"))
    {
      if (blendFactorMap.hasKey(a.value()))
        pass.setBlendFactors(blendFactorMap[a.value()], pass.dstFactor());
      else
      {
        logError("Invalid source blend factor %s", a.value());
        return false;
      }
    }

    if (pugi::xml_attribute a = node.attribute("dst"))
    {
      if (blendFactorMap.hasKey(a.value()))
        pass.setBlendFactors(pass.srcFactor(), blendFactorMap[a.value()]);
      else
      {
        logError("Invalid destination blend factor %s", a.value());
        return false;
      }
    }
  }

  if (pugi::xml_node node = root.child("color"))
  {
    if (pugi::xml_attribute a = node.attribute("writing"))
      pass.setColorWriting(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("multisampling"))
      pass.setMultisampling(a.as_bool());
  }

  if (pugi::xml_node node = root.child("depth"))
  {
    if (pugi::xml_attribute a = node.attribute("testing"))
      pass.setDepthTesting(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("writing"))
      pass.setDepthWriting(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("function"))
    {
      if (functionMap.hasKey(a.value()))
        pass.setDepthFunction(functionMap[a.value()]);
      else
      {
        logError("Invalid depth function %s", a.value());
        return false;
      }
    }
  }

  if (pugi::xml_node node = root.child("stencil"))
  {
    if (pugi::xml_attribute a = node.attribute("testing"))
      pass.setStencilTesting(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("mask"))
      pass.setStencilWriteMask(a.as_uint());

    if (pugi::xml_attribute a = node.attribute("reference"))
      pass.setStencilReference(a.as_uint());

    if (pugi::xml_attribute a = node.attribute("stencilFail"))
    {
      if (functionMap.hasKey(a.value()))
        pass.setStencilFailOperation(operationMap[a.value()]);
      else
      {
        logError("Invalid stencil fail operation %s", a.value());
        return false;
      }
    }

    if (pugi::xml_attribute a = node.attribute("depthFail"))
    {
      if (functionMap.hasKey(a.value()))
        pass.setDepthFailOperation(operationMap[a.value()]);
      else
      {
        logError("Invalid depth fail operation %s", a.value());
        return false;
      }
    }

    if (pugi::xml_attribute a = node.attribute("depthPass"))
    {
      if (functionMap.hasKey(a.value()))
        pass.setDepthPassOperation(operationMap[a.value()]);
      else
      {
        logError("Invalid depth pass operation %s", a.value());
        return false;
      }
    }

    if (pugi::xml_attribute a = node.attribute("function"))
    {
      if (functionMap.hasKey(a.value()))
        pass.setStencilFunction(functionMap[a.value()]);
      else
      {
        logError("Invalid stencil function %s", a.value());
        return false;
      }
    }
  }

  if (pugi::xml_node node = root.child("polygon"))
  {
    if (pugi::xml_attribute a = node.attribute("wireframe"))
      pass.setWireframe(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("cull"))
    {
      if (cullModeMap.hasKey(a.value()))
        pass.setCullMode(cullModeMap[a.value()]);
      else
      {
        logError("Invalid cull mode %s", a.value());
        return false;
      }
    }
  }

  if (pugi::xml_node node = root.child("line"))
  {
    if (pugi::xml_attribute a = node.attribute("smoothing"))
      pass.setLineSmoothing(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("width"))
      pass.setLineWidth(a.as_float());
  }

  if (pugi::xml_node node = root.child("program"))
  {
    const String vertexShaderName(node.attribute("vs").value());
    if (vertexShaderName.empty())
    {
      logError("No vertex shader specified");
      return false;
    }

    const String fragmentShaderName(node.attribute("fs").value());
    if (fragmentShaderName.empty())
    {
      logError("No fragment shader specified");
      return false;
    }

    Ref<Program> program = Program::read(context,
                                         vertexShaderName,
                                         fragmentShaderName);
    if (!program)
    {
      logError("Failed to load program");
      return false;
    }

    pass.setProgram(program);

    for (auto s : node.children("sampler"))
    {
      const String samplerName(s.attribute("name").value());
      if (samplerName.empty())
      {
        logWarning("Program %s lists unnamed sampler uniform",
                   program->name().c_str());

        continue;
      }

      Sampler* sampler = program->findSampler(samplerName.c_str());
      if (!sampler)
      {
        logWarning("Program %s does not have sampler uniform %s",
                   program->name().c_str(),
                   samplerName.c_str());

        continue;
      }

      Ref<Texture> texture;

      if (pugi::xml_attribute a = s.attribute("image"))
      {
        TextureParams params(textureTypeMap[sampler->type()], TF_NONE);

        if (s.attribute("mipmapped").as_bool())
          params.flags |= TF_MIPMAPPED;

        if (s.attribute("sRGB").as_bool())
          params.flags |= TF_SRGB;

        texture = Texture::read(context, params, a.value());
      }
      else if (pugi::xml_attribute a = s.attribute("texture"))
        texture = cache.find<Texture>(a.value());
      else
      {
        logError("No texture specified for sampler %s of program %s",
                  samplerName.c_str(),
                  program->name().c_str());

        return false;
      }

      if (!texture)
      {
        logError("Failed to find texture for sampler %s of program %s",
                  samplerName.c_str(),
                  program->name().c_str());

        return false;
      }

      if (pugi::xml_attribute a = root.attribute("anisotropy"))
        texture->setMaxAnisotropy(a.as_float());

      if (pugi::xml_attribute a = s.attribute("filter"))
      {
        if (filterModeMap.hasKey(a.value()))
          texture->setFilterMode(filterModeMap[a.value()]);
        else
        {
          logError("Invalid filter mode name %s", a.value());
          return false;
        }
      }

      if (pugi::xml_attribute a = s.attribute("address"))
      {
        if (addressModeMap.hasKey(a.value()))
          texture->setAddressMode(addressModeMap[a.value()]);
        else
        {
          logError("Invalid address mode name %s", a.value());
          return false;
        }
      }

      pass.setSamplerState(samplerName.c_str(), texture);
    }

    for (auto u : node.children("uniform"))
    {
      const String uniformName(u.attribute("name").value());
      if (uniformName.empty())
      {
        logWarning("Program %s lists unnamed uniform",
                   program->name().c_str());

        continue;
      }

      const Uniform* uniform = program->findUniform(uniformName.c_str());
      if (!uniform)
      {
        logWarning("Program %s does not have uniform %s",
                   program->name().c_str(),
                   uniformName.c_str());

        continue;
      }

      pugi::xml_attribute attribute = u.attribute("value");
      if (!attribute)
      {
        logError("Missing value for uniform %s of program %s",
                 uniformName.c_str(),
                 program->name().c_str());

        return false;
      }

      switch (uniform->type())
      {
        case UNIFORM_FLOAT:
          pass.setUniformState(uniformName.c_str(), attribute.as_float());
          break;
        case UNIFORM_VEC2:
          pass.setUniformState(uniformName.c_str(), vec2Cast(attribute.value()));
          break;
        case UNIFORM_VEC3:
          pass.setUniformState(uniformName.c_str(), vec3Cast(attribute.value()));
          break;
        case UNIFORM_VEC4:
          pass.setUniformState(uniformName.c_str(), vec4Cast(attribute.value()));
          break;
        case UNIFORM_MAT2:
          pass.setUniformState(uniformName.c_str(), mat2Cast(attribute.value()));
          break;
        case UNIFORM_MAT3:
          pass.setUniformState(uniformName.c_str(), mat3Cast(attribute.value()));
          break;
        case UNIFORM_MAT4:
          pass.setUniformState(uniformName.c_str(), mat4Cast(attribute.value()));
          break;
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

void Material::setSamplerStates(const char* name, Texture* newTexture)
{
  for (uint i = 0;  i < 2;  i++)
  {
    for (auto& p : m_techniques[i].passes)
    {
      if (p.hasSamplerState(name))
        p.setSamplerState(name, newTexture);
    }
  }
}

Ref<Material> Material::create(const ResourceInfo& info, RenderContext& context)
{
  return new Material(info);
}

Ref<Material> Material::read(RenderContext& context, const String& name)
{
  MaterialReader reader(context);
  return reader.read(name);
}

Material::Material(const ResourceInfo& info):
  Resource(info)
{
}

///////////////////////////////////////////////////////////////////////

MaterialReader::MaterialReader(RenderContext& context):
  ResourceReader<Material>(context.cache()),
  m_context(context)
{
  initializeMaps();
}

Ref<Material> MaterialReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.name().c_str());
  if (stream.fail())
  {
    logError("Failed to open material %s", name.c_str());
    return nullptr;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load material %s: %s",
             name.c_str(),
             result.description());
    return nullptr;
  }

  pugi::xml_node root = document.child("material");
  if (!root || root.attribute("version").as_uint() != MATERIAL_XML_VERSION)
  {
    logError("Material file format mismatch in %s", name.c_str());
    return nullptr;
  }

  std::vector<bool> phases(2, false);

  Ref<Material> material = Material::create(ResourceInfo(cache, name, path), m_context);

  for (auto t : root.children("technique"))
  {
    const String phaseName(t.attribute("phase").value());
    if (!phaseMap.hasKey(phaseName))
    {
      logError("Invalid render phase %s in material %s",
               phaseName.c_str(),
               name.c_str());
      return nullptr;
    }

    const RenderPhase phase = phaseMap[phaseName];
    if (phases[phase])
      continue;

    Technique& technique = material->technique(phase);

    for (auto p : t.children("pass"))
    {
      technique.passes.push_back(Pass());
      Pass& pass = technique.passes.back();

      if (!parsePass(m_context, pass, p))
      {
        logError("Failed to parse pass for material %s", name.c_str());
        return nullptr;
      }

      phases[phase] = true;
    }
  }

  return material;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

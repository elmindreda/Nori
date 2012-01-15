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

#include <wendy/Config.h>

#include <wendy/Bimap.h>

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderSystem.h>
#include <wendy/RenderMaterial.h>

#include <algorithm>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

Bimap<String, GL::CullMode> cullModeMap;
Bimap<String, GL::BlendFactor> blendFactorMap;
Bimap<String, GL::Function> functionMap;
Bimap<String, GL::Operation> operationMap;
Bimap<String, GL::FilterMode> filterModeMap;
Bimap<String, GL::AddressMode> addressModeMap;
Bimap<String, System::Type> systemTypeMap;
Bimap<String, Phase> phaseMap;

Bimap<GL::SamplerType, GL::TextureType> textureTypeMap;

const unsigned int MATERIAL_XML_VERSION = 8;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Technique& Material::getTechnique(Phase phase)
{
  return techniques[phase];
}

const Technique& Material::getTechnique(Phase phase) const
{
  return techniques[phase];
}

Ref<Material> Material::create(const ResourceInfo& info, System& system)
{
  return new Material(info);
}

Ref<Material> Material::read(System& system, const String& name)
{
  MaterialReader reader(system);
  return reader.read(name);
}

Material::Material(const ResourceInfo& info):
  Resource(info)
{
}

///////////////////////////////////////////////////////////////////////

MaterialReader::MaterialReader(System& initSystem):
  ResourceReader<Material>(initSystem.getCache()),
  system(initSystem)
{
  if (cullModeMap.isEmpty())
  {
    cullModeMap["none"] = GL::CULL_NONE;
    cullModeMap["front"] = GL::CULL_FRONT;
    cullModeMap["back"] = GL::CULL_BACK;
    cullModeMap["both"] = GL::CULL_BOTH;
  }

  if (blendFactorMap.isEmpty())
  {
    blendFactorMap["zero"] = GL::BLEND_ZERO;
    blendFactorMap["one"] = GL::BLEND_ONE;
    blendFactorMap["src color"] = GL::BLEND_SRC_COLOR;
    blendFactorMap["dst color"] = GL::BLEND_DST_COLOR;
    blendFactorMap["src alpha"] = GL::BLEND_SRC_ALPHA;
    blendFactorMap["dst alpha"] = GL::BLEND_DST_ALPHA;
    blendFactorMap["one minus src color"] = GL::BLEND_ONE_MINUS_SRC_COLOR;
    blendFactorMap["one minus dst color"] = GL::BLEND_ONE_MINUS_DST_COLOR;
    blendFactorMap["one minus src alpha"] = GL::BLEND_ONE_MINUS_SRC_ALPHA;
    blendFactorMap["one minus dst alpha"] = GL::BLEND_ONE_MINUS_DST_ALPHA;
  }

  if (functionMap.isEmpty())
  {
    functionMap["never"] = GL::ALLOW_NEVER;
    functionMap["always"] = GL::ALLOW_ALWAYS;
    functionMap["equal"] = GL::ALLOW_EQUAL;
    functionMap["not equal"] = GL::ALLOW_NOT_EQUAL;
    functionMap["lesser"] = GL::ALLOW_LESSER;
    functionMap["lesser or equal"] = GL::ALLOW_LESSER_EQUAL;
    functionMap["greater"] = GL::ALLOW_GREATER;
    functionMap["greater or equal"] = GL::ALLOW_GREATER_EQUAL;
  }

  if (operationMap.isEmpty())
  {
    operationMap["keep"] = GL::OP_KEEP;
    operationMap["zero"] = GL::OP_ZERO;
    operationMap["replace"] = GL::OP_REPLACE;
    operationMap["increase"] = GL::OP_INCREASE;
    operationMap["decrease"] = GL::OP_DECREASE;
    operationMap["invert"] = GL::OP_INVERT;
    operationMap["increase wrap"] = GL::OP_INCREASE_WRAP;
    operationMap["decrease wrap"] = GL::OP_DECREASE_WRAP;
  }

  if (addressModeMap.isEmpty())
  {
    addressModeMap["wrap"] = GL::ADDRESS_WRAP;
    addressModeMap["clamp"] = GL::ADDRESS_CLAMP;
  }

  if (filterModeMap.isEmpty())
  {
    filterModeMap["nearest"] = GL::FILTER_NEAREST;
    filterModeMap["bilinear"] = GL::FILTER_BILINEAR;
    filterModeMap["trilinear"] = GL::FILTER_TRILINEAR;
  }

  if (textureTypeMap.isEmpty())
  {
    textureTypeMap[GL::SAMPLER_1D] = GL::TEXTURE_1D;
    textureTypeMap[GL::SAMPLER_2D] = GL::TEXTURE_2D;
    textureTypeMap[GL::SAMPLER_3D] = GL::TEXTURE_3D;
    textureTypeMap[GL::SAMPLER_RECT] = GL::TEXTURE_RECT;
    textureTypeMap[GL::SAMPLER_CUBE] = GL::TEXTURE_CUBE;
  }

  if (systemTypeMap.isEmpty())
  {
    systemTypeMap["forward"] = System::FORWARD;
  }

  if (phaseMap.isEmpty())
  {
    phaseMap[""] = PHASE_DEFAULT;
    phaseMap["default"] = PHASE_DEFAULT;
    phaseMap["shadowmap"] = PHASE_SHADOWMAP;
  }
}

Ref<Material> MaterialReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open material \'%s\'", name.c_str());
    return NULL;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load material \'%s\': %s",
             name.c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("material");
  if (!root || root.attribute("version").as_uint() != MATERIAL_XML_VERSION)
  {
    logError("Material file format mismatch in \'%s\'", name.c_str());
    return NULL;
  }

  std::vector<bool> phases(2, false);

  GL::Context& context = system.getContext();

  Ref<Material> material = Material::create(ResourceInfo(cache, name, path), system);

  for (pugi::xml_node t = root.child("technique");  t;  t = t.next_sibling("technique"))
  {
    const String phaseName(t.attribute("phase").value());
    if (!phaseMap.hasKey(phaseName))
    {
      logError("Invalid render phase \'%s\' in material \'%s\'",
               phaseName.c_str(),
               name.c_str());
      return NULL;
    }

    const Phase phase = phaseMap[phaseName];
    if (phases[phase])
      continue;

    const String typeName(t.attribute("type").value());
    if (!systemTypeMap.hasKey(typeName))
    {
      logError("Invalid render system type \'%s\' in material \'%s\'",
               typeName.c_str(),
               name.c_str());
      return NULL;
    }

    const System::Type type = systemTypeMap[typeName];
    if (system.getType() != type)
      continue;

    Technique& technique = material->getTechnique(phase);

    for (pugi::xml_node p = t.child("pass");  p;  p = p.next_sibling("pass"))
    {
      technique.passes.push_back(Pass());
      Pass& pass = technique.passes.back();

      if (pugi::xml_node node = p.child("blending"))
      {
        if (pugi::xml_attribute a = node.attribute("src"))
        {
          if (blendFactorMap.hasKey(a.value()))
            pass.setBlendFactors(blendFactorMap[a.value()], pass.getDstFactor());
          else
          {
            logError("Invalid source blend factor \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }

        if (pugi::xml_attribute a = node.attribute("dst"))
        {
          if (blendFactorMap.hasKey(a.value()))
            pass.setBlendFactors(pass.getSrcFactor(), blendFactorMap[a.value()]);
          else
          {
            logError("Invalid destination blend factor \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }
      }

      if (pugi::xml_node node = p.child("color"))
      {
        if (pugi::xml_attribute a = node.attribute("writing"))
          pass.setColorWriting(a.as_bool());

        if (pugi::xml_attribute a = node.attribute("multisampling"))
          pass.setMultisampling(a.as_bool());
      }

      if (pugi::xml_node node = p.child("depth"))
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
            logError("Invalid depth function \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }
      }

      if (pugi::xml_node node = p.child("stencil"))
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
            logError("Invalid stencil fail operation \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }

        if (pugi::xml_attribute a = node.attribute("depthFail"))
        {
          if (functionMap.hasKey(a.value()))
            pass.setDepthFailOperation(operationMap[a.value()]);
          else
          {
            logError("Invalid depth fail operation \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }

        if (pugi::xml_attribute a = node.attribute("depthPass"))
        {
          if (functionMap.hasKey(a.value()))
            pass.setDepthPassOperation(operationMap[a.value()]);
          else
          {
            logError("Invalid depth pass operation \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }

        if (pugi::xml_attribute a = node.attribute("function"))
        {
          if (functionMap.hasKey(a.value()))
            pass.setStencilFunction(functionMap[a.value()]);
          else
          {
            logError("Invalid stencil function \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }
      }

      if (pugi::xml_node node = p.child("polygon"))
      {
        if (pugi::xml_attribute a = node.attribute("wireframe"))
          pass.setWireframe(a.as_bool());

        if (pugi::xml_attribute a = node.attribute("cull"))
        {
          if (cullModeMap.hasKey(a.value()))
            pass.setCullMode(cullModeMap[a.value()]);
          else
          {
            logError("Invalid cull mode \'%s\' in material \'%s\'",
                     a.value(),
                     name.c_str());
            return NULL;
          }
        }
      }

      if (pugi::xml_node node = p.child("line"))
      {
        if (pugi::xml_attribute a = node.attribute("smoothing"))
          pass.setLineSmoothing(a.as_bool());

        if (pugi::xml_attribute a = node.attribute("width"))
          pass.setLineWidth(a.as_float());
      }

      if (pugi::xml_node node = p.child("program"))
      {
        const String vertexShaderName(node.attribute("vs").value());
        if (vertexShaderName.empty())
        {
          logError("No vertex shader name in material \'%s\'", name.c_str());
          return NULL;
        }

        const String fragmentShaderName(node.attribute("fs").value());
        if (fragmentShaderName.empty())
        {
          logError("No fragment shader name in material \'%s\'", name.c_str());
          return NULL;
        }

        Ref<GL::Program> program = GL::Program::read(context,
                                                     vertexShaderName,
                                                     fragmentShaderName);
        if (!program)
        {
          logError("Failed to load GLSL program for material \'%s\'",
                   name.c_str());
          return NULL;
        }

        pass.setProgram(program);

        GL::ShaderDefines defines;

        for (pugi::xml_node d = node.child("define");  d;  d = d.next_sibling("define"))
        {
          const String defineName(d.attribute("name").value());
          if (defineName.empty())
          {
            logWarning("GLSL program \'%s\' in material \'%s\' lists unnamed define",
                       program->getName().c_str(),
                       name.c_str());

            continue;
          }

          String defineValue(d.attribute("value").value());
          if (defineValue.empty())
            defineValue = "1";

          defines.push_back(std::make_pair(defineName, defineValue));
        }

        std::sort(defines.begin(), defines.end());

        for (pugi::xml_node s = node.child("sampler");  s;  s = s.next_sibling("sampler"))
        {
          const String samplerName(s.attribute("name").value());
          if (samplerName.empty())
          {
            logWarning("GLSL program \'%s\' in material \'%s\' lists unnamed sampler uniform",
                       program->getName().c_str(),
                       name.c_str());

            continue;
          }

          GL::Sampler* sampler = program->findSampler(samplerName.c_str());
          if (!sampler)
          {
            logWarning("GLSL program \'%s\' in material \'%s\' does not have sampler uniform \'%s\'",
                       program->getName().c_str(),
                       name.c_str(),
                       samplerName.c_str());

            continue;
          }

          GL::TextureParams params(textureTypeMap[sampler->getType()]);

          if (pugi::xml_attribute a = s.attribute("mipmapped"))
            params.mipmapped = a.as_bool();

          if (pugi::xml_attribute a = s.attribute("sRGB"))
            params.sRGB = a.as_bool();

          Ref<GL::Texture> texture;

          if (pugi::xml_attribute a = s.attribute("image"))
            texture = GL::Texture::read(context, params, a.value());
          else if (pugi::xml_attribute a = s.attribute("texture"))
            texture = cache.find<GL::Texture>(a.value());
          else
          {
            logError("No texture specified for sampler \'%s\' of GLSL program \'%s\' in material \'%s\'",
                     samplerName.c_str(),
                     program->getName().c_str(),
                     name.c_str());
            return NULL;
          }

          if (!texture)
          {
            logError("Failed to find texture for sampler \'%s\' of GLSL program \'%s\' in material \'%s\'",
                     samplerName.c_str(),
                     program->getName().c_str(),
                     name.c_str());
            return NULL;
          }

          if (pugi::xml_attribute a = root.attribute("anisotropy"))
            texture->setMaxAnisotropy(a.as_float());

          if (pugi::xml_attribute a = s.attribute("filter"))
          {
            if (filterModeMap.hasKey(a.value()))
              texture->setFilterMode(filterModeMap[a.value()]);
            else
            {
              logError("Invalid filter mode name \'%s\'", a.value());
              return NULL;
            }
          }

          if (pugi::xml_attribute a = s.attribute("address"))
          {
            if (addressModeMap.hasKey(a.value()))
              texture->setAddressMode(addressModeMap[a.value()]);
            else
            {
              logError("Invalid address mode name \'%s\'", a.value());
              return NULL;
            }
          }

          pass.setSamplerState(samplerName.c_str(), texture);
        }

        for (pugi::xml_node u = node.child("uniform");  u;  u = u.next_sibling("uniform"))
        {
          const String uniformName(u.attribute("name").value());
          if (uniformName.empty())
          {
            logWarning("GLSL program \'%s\' in material \'%s\' lists unnamed uniform",
                       program->getName().c_str(),
                       name.c_str());

            continue;
          }

          const GL::Uniform* uniform = program->findUniform(uniformName.c_str());
          if (!uniform)
          {
            logWarning("GLSL program \'%s\' in material \'%s\' does not have uniform \'%s\'",
                       program->getName().c_str(),
                       name.c_str(),
                       uniformName.c_str());

            continue;
          }

          pugi::xml_attribute attribute = u.attribute("value");
          if (!attribute)
          {
            logError("Missing value for uniform \'%s\' of GLSL program \'%s\' in material \'%s\'",
                     uniformName.c_str(),
                     program->getName().c_str(),
                     name.c_str());
            return NULL;
          }

          switch (uniform->getType())
          {
            case GL::UNIFORM_FLOAT:
              pass.setUniformState(uniformName.c_str(), attribute.as_float());
              break;
            case GL::UNIFORM_VEC2:
              pass.setUniformState(uniformName.c_str(), vec2Cast(attribute.value()));
              break;
            case GL::UNIFORM_VEC3:
              pass.setUniformState(uniformName.c_str(), vec3Cast(attribute.value()));
              break;
            case GL::UNIFORM_VEC4:
              pass.setUniformState(uniformName.c_str(), vec4Cast(attribute.value()));
              break;
            case GL::UNIFORM_MAT2:
              pass.setUniformState(uniformName.c_str(), mat2Cast(attribute.value()));
              break;
            case GL::UNIFORM_MAT3:
              pass.setUniformState(uniformName.c_str(), mat3Cast(attribute.value()));
              break;
            case GL::UNIFORM_MAT4:
              pass.setUniformState(uniformName.c_str(), mat4Cast(attribute.value()));
              break;
          }
        }
      }

      phases[phase] = true;
    }
  }

  return material;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

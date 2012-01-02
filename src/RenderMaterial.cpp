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

bool isCompatible(System::Type systemType, Technique::Type techniqueType)
{
  if (systemType == System::SIMPLE)
    return false;

  switch (techniqueType)
  {
    case Technique::FORWARD:
      return systemType == System::FORWARD;
    case Technique::DEFERRED:
      return systemType == System::DEFERRED;
    case Technique::SHADOWMAP:
      return true;
  }

  panic("Invalid technique type %u", techniqueType);
}

Bimap<String, GL::CullMode> cullModeMap;
Bimap<String, GL::BlendFactor> blendFactorMap;
Bimap<String, GL::Function> functionMap;
Bimap<String, Technique::Type> techniqueTypeMap;
Bimap<String, GL::FilterMode> filterModeMap;
Bimap<String, GL::AddressMode> addressModeMap;

Bimap<GL::Sampler::Type, GL::TextureType> textureTypeMap;

const unsigned int MATERIAL_XML_VERSION = 7;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Technique::Technique(Type initType):
  type(initType),
  quality(1.f)
{
}

Pass& Technique::createPass()
{
  passes.push_back(Pass());
  return passes.back();
}

void Technique::destroyPass(Pass& pass)
{
  for (PassList::iterator i = passes.begin();  i != passes.end();  i++)
  {
    if (&(*i) == &pass)
    {
      passes.erase(i);
      break;
    }
  }
}

void Technique::destroyPasses()
{
  passes.clear();
}

const PassList& Technique::getPasses() const
{
  return passes;
}

Technique::Type Technique::getType() const
{
  return type;
}

float Technique::getQuality() const
{
  return quality;
}

void Technique::setQuality(float newQuality)
{
  quality = newQuality;
}

///////////////////////////////////////////////////////////////////////

Technique& Material::createTechnique(Technique::Type type)
{
  techniques.push_back(Technique(type));
  return techniques.back();
}

void Material::destroyTechnique(Technique& technique)
{
  for (TechniqueList::iterator t = techniques.begin();  t != techniques.end();  t++)
  {
    if (&(*t) == &technique)
    {
      techniques.erase(t);
      return;
    }
  }
}

void Material::destroyTechniques()
{
  techniques.clear();
}

Technique* Material::findBestTechnique(Technique::Type type)
{
  Technique* best = NULL;

  for (TechniqueList::iterator t = techniques.begin();  t != techniques.end();  t++)
  {
    if (t->getType() != type)
      continue;

    if (best && best->getQuality() > t->getQuality())
      continue;

    best = &(*t);
  }

  return best;
}

const Technique* Material::findBestTechnique(Technique::Type type) const
{
  const Technique* best = NULL;

  for (TechniqueList::const_iterator t = techniques.begin();  t != techniques.end();  t++)
  {
    if (t->getType() != type)
      continue;

    if (best && best->getQuality() > t->getQuality())
      continue;

    best = &(*t);
  }

  return best;
}

TechniqueList& Material::getTechniques()
{
  return techniques;
}

const TechniqueList& Material::getTechniques() const
{
  return techniques;
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
  ResourceReader(initSystem.getCache()),
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

  if (techniqueTypeMap.isEmpty())
  {
    techniqueTypeMap["forward"] = Technique::FORWARD;
    techniqueTypeMap["deferred"] = Technique::DEFERRED;
    techniqueTypeMap["shadowmap"] = Technique::SHADOWMAP;
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
    textureTypeMap[GL::Sampler::SAMPLER_1D] = GL::TEXTURE_1D;
    textureTypeMap[GL::Sampler::SAMPLER_2D] = GL::TEXTURE_2D;
    textureTypeMap[GL::Sampler::SAMPLER_3D] = GL::TEXTURE_3D;
    textureTypeMap[GL::Sampler::SAMPLER_RECT] = GL::TEXTURE_RECT;
    textureTypeMap[GL::Sampler::SAMPLER_CUBE] = GL::TEXTURE_CUBE;
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

  GL::Context& context = system.getContext();

  Ref<Material> material = Material::create(ResourceInfo(cache, name, path), system);

  for (pugi::xml_node t = root.child("technique");  t;  t = t.next_sibling("technique"))
  {
    const String typeName(t.attribute("type").value());
    if (!techniqueTypeMap.hasKey(typeName))
    {
      logError("Invalid technique type \'%s\' in material \'%s\'",
               typeName.c_str(),
               name.c_str());
      return NULL;
    }

    const Technique::Type type = techniqueTypeMap[typeName];

    if (!isCompatible(system.getType(), type))
      continue;

    Technique& technique = material->createTechnique(type);

    if (pugi::xml_attribute a = t.attribute("quality"))
      technique.setQuality(a.as_float());

    for (pugi::xml_node p = t.child("pass");  p;  p = p.next_sibling("pass"))
    {
      Pass& pass = technique.createPass();

      if (pugi::xml_node node = p.child("blending"))
      {
        const String srcFactorName(node.attribute("src").value());
        if (!srcFactorName.empty())
        {
          if (blendFactorMap.hasKey(srcFactorName))
          {
            pass.setBlendFactors(blendFactorMap[srcFactorName],
                                 pass.getDstFactor());
          }
          else
          {
            logError("Invalid blend factor \'%s\' in material \'%s\'",
                     srcFactorName.c_str(),
                     name.c_str());
            return NULL;
          }
        }

        const String dstFactorName(node.attribute("dst").value());
        if (!dstFactorName.empty())
        {
          if (blendFactorMap.hasKey(dstFactorName))
          {
            pass.setBlendFactors(pass.getSrcFactor(),
                                 blendFactorMap[dstFactorName]);
          }
          else
          {
            logError("Invalid blend factor \'%s\' in material \'%s\'",
                     dstFactorName.c_str(),
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

        const String functionName(node.attribute("function").value());
        if (!functionName.empty())
        {
          if (functionMap.hasKey(functionName))
            pass.setDepthFunction(functionMap[functionName]);
          else
          {
            logError("Invalid depth function \'%s\' in material \'%s\'",
                     functionName.c_str(),
                     name.c_str());
            return NULL;
          }
        }
      }

      if (pugi::xml_node node = p.child("polygon"))
      {
        if (pugi::xml_attribute a = node.attribute("wireframe"))
          pass.setWireframe(a.as_bool());

        const String cullModeName(node.attribute("cull").value());
        if (!cullModeName.empty())
        {
          if (cullModeMap.hasKey(cullModeName))
            pass.setCullMode(cullModeMap[cullModeName]);
          else
          {
            logError("Invalid cull mode \'%s\' in material \'%s\'",
                     cullModeName.c_str(),
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
        const String programName(node.attribute("path").value());
        if (programName.empty())
        {
          logError("No GLSL program in material \'%s\'", name.c_str());
          return NULL;
        }

        Ref<GL::Program> program = GL::Program::read(context, programName);
        if (!program)
        {
          logError("Failed to load GLSL program in material \'%s\'",
                   name.c_str());
          return NULL;
        }

        pass.setProgram(program);

        for (pugi::xml_node s = node.child("sampler");  s;  s = s.next_sibling("sampler"))
        {
          const String samplerName(s.attribute("name").value());
          if (samplerName.empty())
          {
            logWarning("GLSL program \'%s\' in material \'%s\' lists unnamed sampler uniform",
                       programName.c_str(),
                       name.c_str());
            continue;
          }

          GL::Sampler* sampler = program->findSampler(samplerName.c_str());
          if (!sampler)
          {
            logWarning("GLSL program \'%s\' in material \'%s\' does not have sampler uniform \'%s\'",
                       programName.c_str(),
                       name.c_str(),
                       samplerName.c_str());
            continue;
          }

          const String imageName(s.attribute("image").value());
          if (imageName.empty())
          {
            logError("No image name specified for sampler \'%s\' of GLSL program \'%s\' in material \'%s\'",
                     samplerName.c_str(),
                     programName.c_str(),
                     name.c_str());
            return NULL;
          }

          GL::TextureParams params(textureTypeMap[sampler->getType()]);

          if (pugi::xml_attribute a = s.attribute("mipmapped"))
            params.mipmapped = a.as_bool();

          if (pugi::xml_attribute a = s.attribute("sRGB"))
            params.sRGB = a.as_bool();

          String textureName;

          textureName.append("source:");
          textureName.append(imageName);

          textureName.append(" mipmapped:");
          textureName.append(params.mipmapped ? "true" : "false");

          textureName.append(" sRGB:");
          textureName.append(params.sRGB ? "true" : "false");

          Ref<GL::Texture> texture = cache.find<GL::Texture>(textureName);
          if (!texture)
          {
            Ref<Image> data = Image::read(cache, imageName);
            if (!data)
            {
              logError("Failed to load image \'%s\' for sampler \'%s\' of GLSL program \'%s\' in material \'%s\'",
                       imageName.c_str(),
                       samplerName.c_str(),
                       programName.c_str(),
                       name.c_str());
              return NULL;
            }

            const ResourceInfo info(cache, textureName);

            texture = GL::Texture::create(info, context, params, *data);
            if (!texture)
            {
              logError("Failed to create texture of image \'%s\' for sampler \'%s\' of GLSL program \'%s\' in material \'%s\'",
                       imageName.c_str(),
                       samplerName.c_str(),
                       programName.c_str(),
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
          }

          pass.setSamplerState(samplerName.c_str(), texture);
        }

        for (pugi::xml_node u = node.child("uniform");  u;  u = u.next_sibling("uniform"))
        {
          const String uniformName(u.attribute("name").value());
          if (uniformName.empty())
          {
            logWarning("GLSL program \'%s\' in material \'%s\' lists unnamed uniform",
                       programName.c_str(),
                       name.c_str());
            continue;
          }

          const GL::Uniform* uniform = program->findUniform(uniformName.c_str());
          if (!uniform)
          {
            logWarning("GLSL program \'%s\' in material \'%s\' does not have uniform \'%s\'",
                       programName.c_str(),
                       name.c_str(),
                       uniformName.c_str());
            continue;
          }

          pugi::xml_attribute attribute = u.attribute("value");
          if (!attribute)
          {
            logError("Missing value for uniform \'%s\' of GLSL program \'%s\' in material \'%s\'",
                     uniformName.c_str(),
                     programName.c_str(),
                     name.c_str());
            return NULL;
          }

          switch (uniform->getType())
          {
            case GL::Uniform::FLOAT:
              pass.setUniformState(uniformName.c_str(), attribute.as_float());
              break;
            case GL::Uniform::VEC2:
              pass.setUniformState(uniformName.c_str(), vec2Cast(attribute.value()));
              break;
            case GL::Uniform::VEC3:
              pass.setUniformState(uniformName.c_str(), vec3Cast(attribute.value()));
              break;
            case GL::Uniform::VEC4:
              pass.setUniformState(uniformName.c_str(), vec4Cast(attribute.value()));
              break;
            case GL::Uniform::MAT2:
              pass.setUniformState(uniformName.c_str(), mat2Cast(attribute.value()));
              break;
            case GL::Uniform::MAT3:
              pass.setUniformState(uniformName.c_str(), mat3Cast(attribute.value()));
              break;
            case GL::Uniform::MAT4:
              pass.setUniformState(uniformName.c_str(), mat4Cast(attribute.value()));
              break;
          }
        }
      }
    }
  }

  return material;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

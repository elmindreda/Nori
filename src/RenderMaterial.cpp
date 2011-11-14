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

#include <wendy/OpenGL.h>
#include <wendy/GLProgram.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLContext.h>

#include <wendy/RenderMaterial.h>

#include <algorithm>

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
Bimap<String, Technique::Type> techniqueTypeMap;

const unsigned int RENDER_MATERIAL_XML_VERSION = 6;

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

Material::Material(const ResourceInfo& info):
  Resource(info)
{
}

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

Ref<Material> Material::read(GL::Context& context, const Path& path)
{
  MaterialReader reader(context);
  return reader.read(path);
}

///////////////////////////////////////////////////////////////////////

MaterialReader::MaterialReader(GL::Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext),
  info(initContext.getIndex())
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
}

Ref<Material> MaterialReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Material*>(cache);

  info.path = path;
  currentTechnique = NULL;
  currentPass = NULL;

  std::ifstream stream;
  if (!getIndex().openFile(stream, info.path))
    return NULL;

  if (!XML::Reader::read(stream))
  {
    material = NULL;
    return NULL;
  }

  if (!material)
  {
    logError("No valid techniques found in material \'%s\'",
             info.path.asString().c_str());

    material = NULL;
    return NULL;
  }

  return material.detachObject();
}

bool MaterialReader::onBeginElement(const String& name)
{
  if (name == "material")
  {
    if (material)
    {
      logError("Only one material per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != RENDER_MATERIAL_XML_VERSION)
    {
      logError("Material XML format version mismatch in \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    material = new Material(info);
    return true;
  }

  if (material)
  {
    if (name == "technique")
    {
      String typeName = readString("type");
      if (!techniqueTypeMap.hasKey(typeName))
      {
        logError("Invalid technique type \'%s\'", typeName.c_str());
        return false;
      }

      Technique& technique = material->createTechnique(techniqueTypeMap[typeName]);
      technique.setQuality(readFloat("quality"));
      currentTechnique = &technique;
      return true;
    }

    if (currentTechnique)
    {
      if (name == "pass")
      {
        Pass& pass = currentTechnique->createPass();
        currentPass = &pass;
        return true;
      }

      if (currentPass)
      {
        if (name == "blending")
        {
          String srcFactorName = readString("src");
          if (!srcFactorName.empty())
          {
            if (blendFactorMap.hasKey(srcFactorName))
              currentPass->setBlendFactors(blendFactorMap[srcFactorName],
                                           currentPass->getDstFactor());
            else
            {
              logError("Invalid blend factor name \'%s\'", srcFactorName.c_str());
              return false;
            }
          }

          String dstFactorName = readString("dst");
          if (!dstFactorName.empty())
          {
            if (blendFactorMap.hasKey(dstFactorName))
              currentPass->setBlendFactors(currentPass->getSrcFactor(),
                                           blendFactorMap[dstFactorName]);
            else
            {
              logError("Invalid blend factor name \'%s\'",
                       dstFactorName.c_str());
              return false;
            }
          }

          return true;
        }

        if (name == "color")
        {
          currentPass->setColorWriting(readBoolean("writing", currentPass->isColorWriting()));
          return true;
        }

        if (name == "depth")
        {
          currentPass->setDepthTesting(readBoolean("testing", currentPass->isDepthTesting()));
          currentPass->setDepthWriting(readBoolean("writing", currentPass->isDepthWriting()));

          String functionName = readString("function");
          if (!functionName.empty())
          {
            if (functionMap.hasKey(functionName))
              currentPass->setDepthFunction(functionMap[functionName]);
            else
            {
              logError("Invalid depth test function name \'%s\'",
                       functionName.c_str());
              return false;
            }
          }

          return true;
        }

        if (name == "polygon")
        {
          currentPass->setWireframe(readBoolean("wireframe"));

          String cullModeName = readString("cull");
          if (!cullModeName.empty())
          {
            if (cullModeMap.hasKey(cullModeName))
              currentPass->setCullMode(cullModeMap[cullModeName]);
            else
            {
              logError("Invalid cull mode \'%s\'", cullModeName.c_str());
              return false;
            }
          }

          return true;
        }

        if (name == "line")
        {
          currentPass->setLineSmoothing(readBoolean("smoothing"));
          currentPass->setLineWidth(readFloat("width"));
          return true;
        }

        if (name == "program")
        {
          Path programPath(readString("path"));
          if (programPath.isEmpty())
          {
            logError("Shader program path missing in material \'%s\'",
                     material->getPath().asString().c_str());
            return false;
          }

          Ref<GL::Program> program = GL::Program::read(context, programPath);
          if (!program)
          {
            logWarning("Failed to load shader program \'%s\'; skipping technique %u in material \'%s\'",
                       programPath.asString().c_str(),
                       material->getTechniques().size(),
                       material->getPath().asString().c_str());

            material->destroyTechnique(*currentTechnique);
            currentTechnique = NULL;
            return true;
          }

          currentPass->setProgram(program);
          return true;
        }

        if (GL::Program* program = currentPass->getProgram())
        {
          if (name == "sampler")
          {
            String samplerName = readString("name");
            if (samplerName.empty())
            {
              logWarning("Shader program \'%s\' lists unnamed sampler uniform",
                         program->getPath().asString().c_str());
              return true;
            }

            if (!program->findSampler(samplerName.c_str()))
            {
              logWarning("Shader program \'%s\' does not have sampler uniform \'%s\'",
                         program->getPath().asString().c_str(),
                         samplerName.c_str());
              return true;
            }

            Path texturePath(readString("texture"));
            if (texturePath.isEmpty())
            {
              logError("Texture path missing for sampler \'%s\'",
                       samplerName.c_str());
              return true;
            }

            Ref<GL::Texture> texture = GL::Texture::read(context, texturePath);
            if (!texture)
            {
              logError("Failed to find texture \'%s\' for sampler \'%s\' of material \'%s\'",
                       texturePath.asString().c_str(),
                       samplerName.c_str(),
                       material->getPath().asString().c_str());
              return false;
            }

            currentPass->setSamplerState(samplerName.c_str(), texture);
            return true;
          }

          if (name == "uniform")
          {
            String uniformName = readString("name");
            if (uniformName.empty())
            {
              logWarning("Shader program \'%s\' lists unnamed uniform",
                         program->getPath().asString().c_str());
              return true;
            }

            GL::Uniform* uniform = program->findUniform(uniformName.c_str());
            if (!uniform)
            {
              logWarning("Shader program \'%s\' does not have uniform \'%s\'",
                         program->getPath().asString().c_str(),
                         uniformName.c_str());
              return true;
            }

            switch (uniform->getType())
            {
              case GL::Uniform::FLOAT:
                currentPass->setUniformState(uniformName.c_str(), readFloat("value"));
                break;
              case GL::Uniform::VEC2:
                currentPass->setUniformState(uniformName.c_str(), vec2Cast(readString("value")));
                break;
              case GL::Uniform::VEC3:
                currentPass->setUniformState(uniformName.c_str(), vec3Cast(readString("value")));
                break;
              case GL::Uniform::VEC4:
                currentPass->setUniformState(uniformName.c_str(), vec4Cast(readString("value")));
                break;
              case GL::Uniform::MAT2:
                currentPass->setUniformState(uniformName.c_str(), mat2Cast(readString("value")));
                break;
              case GL::Uniform::MAT3:
                currentPass->setUniformState(uniformName.c_str(), mat3Cast(readString("value")));
                break;
              case GL::Uniform::MAT4:
                currentPass->setUniformState(uniformName.c_str(), mat4Cast(readString("value")));
                break;
            }

            return true;
          }
        }
      }
    }
  }

  return true;
}

bool MaterialReader::onEndElement(const String& name)
{
  if (material)
  {
    if (currentTechnique)
    {
      if (name == "technique")
      {
        currentTechnique = NULL;
        return true;
      }

      if (currentPass)
      {
        if (name == "pass")
        {
          currentPass = NULL;
          return true;
        }
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

bool MaterialWriter::write(const Path& path, const Material& material)
{
  std::ofstream stream(path.asString().c_str());
  if (!stream)
    return false;

  GL::RenderState defaults;

  try
  {
    setStream(&stream);

    beginElement("material");
    addAttribute("version", (int) RENDER_MATERIAL_XML_VERSION);

    const TechniqueList& techniques = material.getTechniques();

    for (TechniqueList::const_iterator t = techniques.begin();  t != techniques.end();  t++)
    {
      beginElement("technique");
      addAttribute("type", techniqueTypeMap[t->getType()]);
      addAttribute("quality", t->getQuality());

      const PassList& passes = t->getPasses();

      for (PassList::const_iterator p = passes.begin();  p != passes.end();  p++)
      {
        beginElement("pass");

        if (p->getSrcFactor() != defaults.getSrcFactor() ||
            p->getDstFactor() != defaults.getDstFactor())
        {
          beginElement("blending");
          addAttribute("src", blendFactorMap[p->getSrcFactor()]);
          addAttribute("dst", blendFactorMap[p->getDstFactor()]);
          endElement();
        }

        if (p->isColorWriting() != defaults.isColorWriting())
        {
          beginElement("color");
          addAttribute("writing", p->isColorWriting());
          endElement();
        }

        if (p->isDepthTesting() != defaults.isDepthTesting() ||
            p->isDepthWriting() != defaults.isDepthWriting())
        {
          beginElement("depth");
          addAttribute("testing", p->isDepthTesting());
          addAttribute("writing", p->isDepthWriting());
          addAttribute("function", functionMap[p->getDepthFunction()]);
          endElement();
        }

        if (p->isWireframe() != defaults.isWireframe() ||
            p->getCullMode() != defaults.getCullMode())
        {
          beginElement("polygon");
          addAttribute("wireframe", p->isWireframe());
          addAttribute("cull", cullModeMap[p->getCullMode()]);
          endElement();
        }

        if (p->isLineSmoothing() != defaults.isLineSmoothing() ||
            p->getLineWidth() != defaults.getLineWidth())
        {
          beginElement("line");
          addAttribute("smoothing", p->isLineSmoothing());
          addAttribute("width", p->getLineWidth());
          endElement();
        }

        if (GL::Program* program = p->getProgram())
        {
          beginElement("program");
          addAttribute("path", program->getPath().asString());

          for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
          {
            const GL::Sampler& sampler = program->getSampler(i);

            Ref<GL::Texture> texture = p->getSamplerState(sampler.getName().c_str());
            if (!texture)
              continue;

            beginElement("sampler");
            addAttribute("name", sampler.getName());
            addAttribute("texture", texture->getPath().asString());
            endElement();
          }

          for (unsigned int i = 0;  i < program->getUniformCount();  i++)
          {
            const GL::Uniform& uniform = program->getUniform(i);

            beginElement("uniform");
            addAttribute("name", uniform.getName());

            switch (uniform.getType())
            {
              case GL::Uniform::FLOAT:
              {
                float value;
                p->getUniformState(uniform.getName().c_str(), value);
                addAttribute("value", value);
                break;
              }

              case GL::Uniform::VEC2:
              {
                vec2 value;
                p->getUniformState(uniform.getName().c_str(), value);
                addAttribute("value", stringCast(value));
                break;
              }

              case GL::Uniform::VEC3:
              {
                vec3 value;
                p->getUniformState(uniform.getName().c_str(), value);
                addAttribute("value", stringCast(value));
                break;
              }

              case GL::Uniform::VEC4:
              {
                vec4 value;
                p->getUniformState(uniform.getName().c_str(), value);
                addAttribute("value", stringCast(value));
                break;
              }

              case GL::Uniform::MAT2:
              {
                mat2 value;
                p->getUniformState(uniform.getName().c_str(), value);
                addAttribute("value", stringCast(value));
                break;
              }

              case GL::Uniform::MAT3:
              {
                mat3 value;
                p->getUniformState(uniform.getName().c_str(), value);
                addAttribute("value", stringCast(value));
                break;
              }

              case GL::Uniform::MAT4:
              {
                mat4 value;
                p->getUniformState(uniform.getName().c_str(), value);
                addAttribute("value", stringCast(value));
                break;
              }
            }

            endElement();
          }

          endElement();
        }

        endElement();
      }

      endElement();
    }

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    logError("Failed to write material \'%s\': %s",
             material.getPath().asString().c_str(),
             exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

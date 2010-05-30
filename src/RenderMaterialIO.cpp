///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/GLImage.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>

#include <internal/RenderMaterialIO.h>

#include <algorithm>
#include <cstdlib>

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

const unsigned int RENDER_MATERIAL_XML_VERSION = 4;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

MaterialCodec::MaterialCodec(void):
  ResourceCodec<Material>("XML material codec")
{
  addSuffix("material");
}

Material* MaterialCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<Material>::read(path, name);
}

Material* MaterialCodec::read(Stream& stream, const String& name)
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

  currentTechnique = NULL;
  currentPass = NULL;

  materialName = name;

  if (!XML::Codec::read(stream))
  {
    material = NULL;
    return NULL;
  }

  if (!material || !material->getTechniqueCount())
  {
    Log::writeError("No valid techniques found in material \'%s\'",
                    materialName.c_str());

    material = NULL;
    return NULL;
  }

  return material.detachObject();
}

bool MaterialCodec::write(const Path& path, const Material& material)
{
  return ResourceCodec<Material>::write(path, material);
}

bool MaterialCodec::write(Stream& stream, const Material& material)
{
  GL::RenderState defaults;

  try
  {
    setStream(&stream);

    beginElement("material");
    addAttribute("version", (int) RENDER_MATERIAL_XML_VERSION);

    for (unsigned int i = 0;  i < material.getTechniqueCount();  i++)
    {
      const Technique& technique = material.getTechnique(i);

      beginElement("technique");
      addAttribute("name", technique.getName());
      addAttribute("quality", technique.getQuality());

      for (unsigned int i = 0;  i < technique.getPassCount();  i++)
      {
        const Pass& pass = technique.getPass(i);

        beginElement("pass");

        if (!pass.getName().empty())
          addAttribute("name", pass.getName());

        if (pass.getSrcFactor() != defaults.getSrcFactor() ||
            pass.getDstFactor() != defaults.getDstFactor())
        {
          beginElement("blending");
          addAttribute("src", blendFactorMap[pass.getSrcFactor()]);
          addAttribute("dst", blendFactorMap[pass.getDstFactor()]);
          endElement();
        }

        if (pass.isColorWriting() != defaults.isColorWriting())
        {
          beginElement("color");
          addAttribute("writing", pass.isColorWriting());
          endElement();
        }

        if (pass.isDepthTesting() != defaults.isDepthTesting() ||
            pass.isDepthWriting() != defaults.isDepthWriting())
        {
          beginElement("depth");
          addAttribute("testing", pass.isDepthTesting());
          addAttribute("writing", pass.isDepthWriting());
          addAttribute("function", functionMap[pass.getDepthFunction()]);
          endElement();
        }

        if (pass.isWireframe() != defaults.isWireframe() ||
            pass.getCullMode() != defaults.getCullMode())
        {
          beginElement("polygon");
          addAttribute("wireframe", pass.isWireframe());
          addAttribute("cull", cullModeMap[pass.getCullMode()]);
          endElement();
        }

        if (GL::Program* program = pass.getProgram())
        {
          beginElement("program");
          addAttribute("name", program->getName());

          for (unsigned int i = 0;  i < pass.getSamplerCount();  i++)
          {
            const GL::SamplerState& state = pass.getSamplerState(i);

            Ref<GL::Texture> texture;
            state.getTexture(texture);
            if (!texture)
              continue;

            beginElement("sampler");
            addAttribute("name", state.getSampler().getName());
            addAttribute("texture", texture->getName());
            endElement();
          }

          for (unsigned int i = 0;  i < pass.getUniformCount();  i++)
          {
            const GL::UniformState& state = pass.getUniformState(i);

            beginElement("uniform");
            addAttribute("name", state.getUniform().getName());

            switch (state.getUniform().getType())
            {
              case GL::Uniform::FLOAT:
              {
                float value;
                state.getValue(value);
                addAttribute("value", value);
                break;
              }

              case GL::Uniform::FLOAT_VEC2:
              {
                Vec2 value;
                state.getValue(value);
                addAttribute("value", value.asString());
                break;
              }

              case GL::Uniform::FLOAT_VEC3:
              {
                Vec3 value;
                state.getValue(value);
                addAttribute("value", value.asString());
                break;
              }

              case GL::Uniform::FLOAT_VEC4:
              {
                Vec4 value;
                state.getValue(value);
                addAttribute("value", value.asString());
                break;
              }

              case GL::Uniform::FLOAT_MAT2:
              {
                Mat2 value;
                state.getValue(value);
                addAttribute("value", value.asString());
                break;
              }

              case GL::Uniform::FLOAT_MAT3:
              {
                Mat3 value;
                state.getValue(value);
                addAttribute("value", value.asString());
                break;
              }

              case GL::Uniform::FLOAT_MAT4:
              {
                Mat4 value;
                state.getValue(value);
                addAttribute("value", value.asString());
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
    Log::writeError("Failed to write material \'%s\': %s",
                    material.getName().c_str(),
                    exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool MaterialCodec::onBeginElement(const String& name)
{
  if (name == "material")
  {
    if (material)
    {
      Log::writeError("Only one material per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != RENDER_MATERIAL_XML_VERSION)
    {
      Log::writeError("Material XML format version mismatch");
      return false;
    }

    material = new Material(materialName);
    return true;
  }

  if (material)
  {
    if (name == "technique")
    {
      Technique& technique = material->createTechnique(readString("name"));
      technique.setQuality(readFloat("quality"));
      currentTechnique = &technique;
      return true;
    }

    if (currentTechnique)
    {
      if (name == "pass")
      {
        Pass& pass = currentTechnique->createPass(readString("name"));
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
              Log::writeError("Invalid blend factor name \'%s\'", srcFactorName.c_str());
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
              Log::writeError("Invalid blend factor name \'%s\'",
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
              Log::writeError("Invalid depth test function name \'%s\'",
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
              Log::writeError("Invalid cull mode \'%s\'", cullModeName.c_str());
              return false;
            }
          }

          return true;
        }

        if (name == "program")
        {
          String programName = readString("name");
          if (programName.empty())
          {
            Log::writeError("Shader program name missing");
            return false;
          }

          GL::Program* program = GL::Program::readInstance(programName);
          if (!program)
          {
            Log::writeWarning("Failed to load shader program \'%s\'; skipping technique %u in material \'%s\'",
                              programName.c_str(),
                              material->getTechniqueCount(),
                              material->getName().c_str());

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
              Log::writeWarning("Shader program \'%s\' lists unnamed sampler uniform",
                                program->getName().c_str());
              return true;
            }

            if (!program->findSampler(samplerName))
            {
              Log::writeWarning("Shader program \'%s\' does not have sampler uniform \'%s\'",
                                program->getName().c_str(),
                                samplerName.c_str());
              return true;
            }

            String textureName = readString("texture");
            if (textureName.empty())
              return true;

            Ref<GL::Texture> texture = GL::Texture::readInstance(textureName);
            if (!texture)
              return false;

            currentPass->getSamplerState(samplerName).setTexture(texture);
            return true;
          }

          if (name == "uniform")
          {
            String uniformName = readString("name");
            if (uniformName.empty())
            {
              Log::writeWarning("Shader program \'%s\' lists unnamed uniform",
                                program->getName().c_str());
              return true;
            }

            GL::Uniform* uniform = program->findUniform(uniformName);
            if (!uniform)
            {
              Log::writeWarning("Shader program \'%s\' does not have uniform \'%s\'",
                                program->getName().c_str(),
                                uniformName.c_str());
              return true;
            }

            switch (uniform->getType())
            {
              case GL::Uniform::FLOAT:
                currentPass->getUniformState(uniformName).setValue(readFloat("value"));
                break;
              case GL::Uniform::FLOAT_VEC2:
                currentPass->getUniformState(uniformName).setValue(Vec2(readString("value")));
                break;
              case GL::Uniform::FLOAT_VEC3:
                currentPass->getUniformState(uniformName).setValue(Vec3(readString("value")));
                break;
              case GL::Uniform::FLOAT_VEC4:
                currentPass->getUniformState(uniformName).setValue(Vec4(readString("value")));
                break;
              case GL::Uniform::FLOAT_MAT2:
                currentPass->getUniformState(uniformName).setValue(Mat2(readString("value")));
                break;
              case GL::Uniform::FLOAT_MAT3:
                currentPass->getUniformState(uniformName).setValue(Mat3(readString("value")));
                break;
              case GL::Uniform::FLOAT_MAT4:
                currentPass->getUniformState(uniformName).setValue(Mat4(readString("value")));
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

bool MaterialCodec::onEndElement(const String& name)
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

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

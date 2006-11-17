///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>
#include <wendy/GLRenderIO.h>

#include <algorithm>
#include <cstdlib>

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

Mapper<String, CullMode> cullModeMap;
Mapper<String, GLenum> combineModeMap;
Mapper<String, GLenum> polygonModeMap;
Mapper<String, GLenum> blendFactorMap;
Mapper<String, GLenum> functionMap;
Mapper<String, GLenum> operationMap;
Mapper<String, GLint> filterMap;
Mapper<String, GLint> addressModeMap;

const unsigned int RENDER_STYLE_XML_VERSION = 2;

}

///////////////////////////////////////////////////////////////////////

RenderStyleCodec::RenderStyleCodec(void):
  ResourceCodec<RenderStyle>("XML render style codec")
{
  addSuffix("style");
}

RenderStyle* RenderStyleCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<RenderStyle>::read(path, name);
}

RenderStyle* RenderStyleCodec::read(Stream& stream, const String& name)
{
  if (cullModeMap.isEmpty())
  {
    cullModeMap["none"] = CULL_NONE;
    cullModeMap["front"] = CULL_FRONT;
    cullModeMap["back"] = CULL_BACK;
    cullModeMap["both"] = CULL_BOTH;
  }

  if (combineModeMap.isEmpty())
  {
    combineModeMap["replace"] = GL_REPLACE;
    combineModeMap["modulate"] = GL_MODULATE;
    combineModeMap["decal"] = GL_DECAL;
    combineModeMap["blend"] = GL_BLEND;
  }

  if (polygonModeMap.isEmpty())
  {
    polygonModeMap["points"] = GL_POINT;
    polygonModeMap["lines"] = GL_LINE;
    polygonModeMap["faces"] = GL_FILL;
  }

  if (blendFactorMap.isEmpty())
  {
    blendFactorMap["zero"] = GL_ZERO;
    blendFactorMap["one"] = GL_ONE;
    blendFactorMap["src color"] = GL_SRC_COLOR;
    blendFactorMap["dst color"] = GL_DST_COLOR;
    blendFactorMap["src alpha"] = GL_SRC_ALPHA;
    blendFactorMap["dst alpha"] = GL_DST_ALPHA;
    blendFactorMap["one minus src color"] = GL_ONE_MINUS_SRC_COLOR;
    blendFactorMap["one minus dst color"] = GL_ONE_MINUS_DST_COLOR;
    blendFactorMap["one minus src alpha"] = GL_ONE_MINUS_SRC_ALPHA;
    blendFactorMap["one minus dst alpha"] = GL_ONE_MINUS_DST_ALPHA;
  }

  if (functionMap.isEmpty())
  {
    functionMap["never"] = GL_NEVER;
    functionMap["always"] = GL_ALWAYS;
    functionMap["equal"] = GL_EQUAL;
    functionMap["not equal"] = GL_NOTEQUAL;
    functionMap["lesser"] = GL_LESS;
    functionMap["lesser or equal"] = GL_LEQUAL;
    functionMap["greater"] = GL_GREATER;
    functionMap["greater or equal"] = GL_GEQUAL;
  }

  if (operationMap.isEmpty())
  {
    operationMap["keep"] = GL_KEEP;
    operationMap["reset"] = GL_ZERO;
    operationMap["replace"] = GL_REPLACE;
    operationMap["increment"] = GL_INCR;
    operationMap["decrement"] = GL_DECR;
    operationMap["invert"] = GL_INVERT;
  }

  if (filterMap.isEmpty())
  {
    filterMap["nearest"] = GL_NEAREST;
    filterMap["linear"] = GL_LINEAR;
    filterMap["nearest mipmap nearest"] = GL_NEAREST_MIPMAP_NEAREST;
    filterMap["nearest mipmap linear"] = GL_NEAREST_MIPMAP_LINEAR;
    filterMap["linear mipmap nearest"] = GL_LINEAR_MIPMAP_NEAREST;
    filterMap["linear mipmap linear"] = GL_LINEAR_MIPMAP_LINEAR;
  }

  if (addressModeMap.isEmpty())
  {
    addressModeMap["wrap"] = GL_REPEAT;
    addressModeMap["clamp"] = GL_CLAMP;
  }

  currentPass = NULL;
  currentLayer = NULL;

  styleName = name;

  if (!XML::Codec::read(stream))
    return NULL;

  return style.detachObject();
}

bool RenderStyleCodec::write(const Path& path, const RenderStyle& style)
{
  return ResourceCodec<RenderStyle>::write(path, style);
}

bool RenderStyleCodec::write(Stream& stream, const RenderStyle& style)
{
  RenderPass defaults;

  try
  {
    setStream(&stream);

    beginElement("style");
    addAttribute("version", (int) RENDER_STYLE_XML_VERSION);

    for (unsigned int i = 0;  i < style.getPassCount();  i++)
    {
      const RenderPass& pass = style.getPass(i);

      beginElement("pass");

      if (pass.isLit() != defaults.isLit())
      {
	beginElement("lighting");
	addAttribute("enabled", pass.isLit());
	endElement();
      }

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

      if (pass.getAlphaFunction() != defaults.getAlphaFunction())
      {
	beginElement("alpha");
	addAttribute("function", functionMap[pass.getAlphaFunction()]);
	endElement();
      }

      /*
      if (pass.isStencilTesting() != defaults.isStencilTesting() ||
	  pass.getStencilFunction() != defaults.getStencilFunction() ||
	  pass.getStencilReference() != defaults.getStencilReference() ||
	  pass.getStencilMask() != defaults.getStencilMask() ||
	  pass.getStencilFailOperation() != defaults.getStencilFailOperation() ||
	  pass.getDepthPassOperation() != defaults.getDepthPassOperation() ||
	  pass.getDepthFailOperation() != defaults.getDepthFailOperation())
      {
	beginElement("stencil");
	addAttribute("testing", pass.isStencilTesting());
	addAttribute("function", functionMap[pass.getStencilFunction()]);
	addAttribute("reference", pass.getStencilReference());
	addAttribute("mask", pass.getStencilMask());
	addAttribute("stencil-failed", operationMap[pass.getStencilFailOperation()]);
	addAttribute("depth-failed", operationMap[pass.getDepthFailOperation()]);
	addAttribute("depth-passed", operationMap[pass.getDepthPassOperation()]);
	endElement();
      }
      */

      if (pass.getLineWidth() != defaults.getLineWidth())
      {
	beginElement("line");
	addAttribute("width", pass.getLineWidth());
	endElement();
      }

      if (pass.getPolygonMode() != defaults.getPolygonMode() ||
	  pass.getCullMode() != defaults.getCullMode())
      {
	beginElement("polygon");
	addAttribute("mode", polygonModeMap[pass.getPolygonMode()]);
	addAttribute("cull", cullModeMap[pass.getCullMode()]);
	endElement();
      }

      if (pass.isLit())
      {
	if (pass.getAmbientColor() != defaults.getAmbientColor())
	{
	  beginElement("ambient");
	  addAttributes(pass.getAmbientColor());
	  endElement();
	}

	if (pass.getDiffuseColor() != defaults.getDiffuseColor())
	{
	  beginElement("diffuse");
	  addAttributes(pass.getDiffuseColor());
	  endElement();
	}

	if (pass.getSpecularColor() != defaults.getSpecularColor() ||
	    pass.getShininess() != defaults.getShininess())
	{
	  beginElement("specular");
	  addAttributes(pass.getSpecularColor());
	  addAttribute("shininess", pass.getShininess());
	  endElement();
	}
      }
      else
      {
	if (pass.getDefaultColor() != defaults.getDefaultColor())
	{
	  beginElement("default");
	  addAttributes(pass.getDefaultColor());
	  endElement();
	}
      }

      for (unsigned int i = 0;  i < pass.getTextureLayerCount();  i++)
      {
	const GL::TextureLayer& layer = pass.getTextureLayer(i);

	if (layer.getTextureName().empty())
	  break;

	beginElement("texture");
	addAttribute("name", layer.getTextureName());

	if (!layer.getSamplerName().empty())
	  addAttribute("sampler", layer.getSamplerName());

	beginElement("combine");
	addAttribute("mode", combineModeMap[layer.getCombineMode()]);
	addAttributes(layer.getCombineColor());
	endElement();

	String mappingMode = (layer.isSphereMapped() ? "sphere" : "none");

	beginElement("mapping");
	addAttribute("mode", mappingMode);
	endElement();

	beginElement("filter");
	addAttribute("min", filterMap[layer.getMinFilter()]);
	addAttribute("mag", filterMap[layer.getMagFilter()]);
	endElement();

	beginElement("address");
	addAttribute("mode", addressModeMap[layer.getAddressMode()]);
	endElement();

	endElement();
      }

      if (!pass.getShaderProgramName().empty())
      {
	beginElement("shader-program");
	addAttribute("name", pass.getShaderProgramName());
	endElement();
      }

      endElement();
    }

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write render style %s: %s", style.getName().c_str(), exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool RenderStyleCodec::onBeginElement(const String& name)
{
  if (name == "style")
  {
    if (style)
    {
      Log::writeError("Only one render style per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != RENDER_STYLE_XML_VERSION)
    {
      Log::writeError("Render style XML format version mismatch");
      return false;
    }

    style = new RenderStyle(styleName);
    return true;
  }

  if (style)
  {
    if (name == "pass")
    {
      RenderPass& pass = style->createPass();
      currentPass = &pass;
      return true;
    }

    if (currentPass)
    {
      if (name == "lighting")
      {
        currentPass->setLit(readBoolean("enabled", currentPass->isLit()));
        return true;
      }

      if (name == "blending")
      {
        String srcFactorName = readString("src");
        if (srcFactorName.length())
        {
          if (blendFactorMap.hasKey(srcFactorName))
            currentPass->setBlendFactors(blendFactorMap[srcFactorName], currentPass->getDstFactor());
          else
	  {
	    Log::writeError("Invalid blend factor name %s", srcFactorName.c_str());
            return false;
	  }
        }

        String dstFactorName = readString("dst");
        if (dstFactorName.length())
        {
          if (blendFactorMap.hasKey(dstFactorName))
            currentPass->setBlendFactors(currentPass->getSrcFactor(), blendFactorMap[dstFactorName]);
          else
	  {
	    Log::writeError("Invalid blend factor name %s", dstFactorName.c_str());
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
        if (functionName.length())
        {
          if (functionMap.hasKey(functionName))
            currentPass->setDepthFunction(functionMap[functionName]);
          else
	  {
	    Log::writeError("Invalid depth test function name %s", functionName.c_str());
            return false;
	  }
        }

	return true;
      }
      
      if (name == "alpha")
      {
        String functionName = readString("function");
        if (functionName.length())
        {
          if (functionMap.hasKey(functionName))
            currentPass->setAlphaFunction(functionMap[functionName]);
          else
	  {
	    Log::writeError("Invalid alpha test function name %s", functionName.c_str());
            return false;
	  }
	}

	return true;
      }

      /*
      if (name == "stencil")
      {
        currentPass->setStencilTesting(readBoolean("testing", currentPass->isStencilTesting()));

	String functionName = readString("function");
	if (!functionMap.hasKey(functionName))
	  return false;

        currentPass->setStencilFunction(functionMap[functionName]);

        String operationName;

        operationName = readString("stencil-failed");
        if (operationMap.hasKey(operationName))
          currentPass->setStencilOperations(operationMap[operationName],
                                            currentPass->getDepthFailOperation(),
                                            currentPass->getDepthPassOperation());
        else
          return false;

        operationName = readString("depth-failed");
        if (operationMap.hasKey(operationName))
          currentPass->setStencilOperations(currentPass->getStencilFailOperation(),
                                            operationMap[operationName],
                                            currentPass->getDepthPassOperation());
        else
          return false;

        operationName = readString("depth-passed");
        if (operationMap.hasKey(operationName))
          currentPass->setStencilOperations(currentPass->getStencilFailOperation(),
                                            currentPass->getDepthFailOperation(),
                                            operationMap[operationName]);
        else
          return false;

	const unsigned int reference = readInteger("reference", currentPass->getStencilReference());
        const unsigned int mask = readInteger("mask", currentPass->getStencilMask());

        currentPass->setStencilValues(reference, mask);

        return true;
      }
      */

      if (name == "line")
      {
        currentPass->setLineWidth(readFloat("width"));
        return true;
      }

      if (name == "polygon")
      {
        String polygonModeName = readString("mode");
        if (polygonModeName.length())
        {
          if (polygonModeMap.hasKey(polygonModeName))
            currentPass->setPolygonMode(polygonModeMap[polygonModeName]);
          else
          {
            Log::writeError("Invalid polygon mode %s", polygonModeName.c_str());
            return false;
          }
        }

        String cullModeName = readString("cull");
        if (cullModeName.length())
        {
          if (cullModeMap.hasKey(cullModeName))
            currentPass->setCullMode(cullModeMap[cullModeName]);
          else
          {
            Log::writeError("Invalid cull mode %s", cullModeName.c_str());
            return false;
          }
        }

        return true;
      }

      if (name == "default")
      {
	ColorRGBA color;
	readAttributes(color, currentPass->getDefaultColor());
        currentPass->setDefaultColor(color);
        return true;
      }

      if (name == "ambient")
      {
	ColorRGBA color;
	readAttributes(color, currentPass->getAmbientColor());
        currentPass->setAmbientColor(color);
        return true;
      }

      if (name == "diffuse")
      {
	ColorRGBA color;
	readAttributes(color, currentPass->getDiffuseColor());
        currentPass->setDiffuseColor(color);
        return true;
      }

      if (name == "specular")
      {
	ColorRGBA color;
	readAttributes(color, currentPass->getSpecularColor());
        currentPass->setSpecularColor(color);

	currentPass->setShininess(readFloat("shininess", currentPass->getShininess()));
        return true;
      }

      if (name == "texture")
      {
	String textureName = readString("name");
	if (!textureName.length())
	  return true;

	if (!Texture::readInstance(textureName))
	  return false;

	TextureLayer& layer = currentPass->createTextureLayer();
	layer.setTextureName(textureName);
	layer.setSamplerName(readString("sampler"));

	currentLayer = &layer;
	return true;
      }

      if (name == "shader-program")
      {
	String shaderProgramName = readString("name");
	if (!shaderProgramName.length())
	  return true;

	if (!ShaderProgram::readInstance(shaderProgramName))
	  return false;
	
	currentPass->setShaderProgramName(shaderProgramName);
	return true;
      }

      if (currentLayer)
      {
	if (name == "combine")
	{
	  String combineModeName = readString("mode");
	  if (combineModeName.length())
	  {
	    if (combineModeMap.hasKey(combineModeName))
	      currentLayer->setCombineMode(combineModeMap[combineModeName]);
	    else
	    {
	      Log::writeError("Invalid texture combine %s", combineModeName.c_str());
	      return false;
	    }
	  }

	  ColorRGBA color;
	  readAttributes(color, currentLayer->getCombineColor());
	  currentLayer->setCombineColor(color);
	  return true;
	}
      }

      if (name == "mapping")
      {
	String modeName = readString("mode");
	if (modeName == "sphere")
	  currentLayer->setSphereMapped(true);
	else if (modeName == "none")
	  currentLayer->setSphereMapped(false);
	else
	{
	  Log::writeError("Invalid texture layer mapping mode name %s", modeName.c_str());
	  return false;
	}

	return true;
      }

      if (name == "filter")
      {
	String filterName;
	
	filterName = readString("min");
	if (!filterName.empty())
	{
	  if (filterMap.hasKey(filterName))
	    currentLayer->setFilters(filterMap[filterName], currentLayer->getMagFilter());
	  else
	    Log::writeError("Invalid texture layer min filter type %s", filterName.c_str());
	}
	
	filterName = readString("mag");
	if (!filterName.empty())
	{
	  if (filterMap.hasKey(filterName))
	    currentLayer->setFilters(currentLayer->getMinFilter(), filterMap[filterName]);
	  else
	    Log::writeError("Invalid texture layer mag filter type %s", filterName.c_str());
	}
      }

      if (name == "address")
      {
	String addressModeName = readString("mode");
	if (!addressModeName.empty())
	{
	  if (addressModeMap.hasKey(addressModeName))
	    currentLayer->setAddressMode(addressModeMap[addressModeName]);
	  else
	    Log::writeError("Invalid texture layer address mode %s", addressModeName.c_str());
	}
      }
    }
  }

  return true;
}

bool RenderStyleCodec::onEndElement(const String& name)
{
  if (style)
  {
    if (currentPass)
    {
      if (name == "pass")
      {
	currentPass = NULL;
	return true;
      }

      if (currentLayer)
      {
	if (name == "texture")
	{
	  currentLayer = NULL;
	  return true;
	}
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

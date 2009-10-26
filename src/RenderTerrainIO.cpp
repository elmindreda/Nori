///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderMesh.h>
#include <wendy/RenderTerrain.h>

#include <internal/RenderTerrainIO.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int RENDER_TERRAIN_XML_VERSION = 1;

}

///////////////////////////////////////////////////////////////////////

TerrainCodecXML::TerrainCodecXML(void):
  ResourceCodec<Terrain>("XML terrain codec")
{
  addSuffix("terrain");
}

Terrain* TerrainCodecXML::read(const Path& path, const String& name)
{
  return ResourceCodec<Terrain>::read(path, name);
}

Terrain* TerrainCodecXML::read(Stream& stream, const String& name)
{
  terrainName = name;

  if (!XML::Codec::read(stream))
  {
    terrain = NULL;
    return NULL;
  }

  return terrain.detachObject();
}

bool TerrainCodecXML::write(const Path& path, const Terrain& terrain)
{
  return ResourceCodec<Terrain>::write(path, terrain);
}

bool TerrainCodecXML::write(Stream& stream, const Terrain& terrain)
{
  try
  {
    setStream(&stream);

    beginElement("terrain");
    addAttribute("version", (int) RENDER_TERRAIN_XML_VERSION);

    // TODO: No idea.

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write terrain %s: %s",
                    terrain.getName().c_str(),
		    exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool TerrainCodecXML::onBeginElement(const String& name)
{
  if (name == "terrain")
  {
    if (terrain)
    {
      Log::writeError("Only one terrain per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != RENDER_TERRAIN_XML_VERSION)
    {
      Log::writeError("Terrain XML format version mismatch");
      return false;
    }

    Ref<Image> heightmap = Image::readInstance(readString("heightmap"));
    if (!heightmap)
      return false;

    Ref<Material> material = Material::readInstance(readString("material"));
    if (!material)
      return false;

    Vec3 size;
    readAttributes(size);

    terrain = Terrain::createInstance(*heightmap, size, *material, terrainName);
    if (!terrain)
      return false;

    return true;
  }

  return true;
}

bool TerrainCodecXML::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

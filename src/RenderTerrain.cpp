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

#include <moira/Moira.h>

#include <wendy/Core.h>
#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderStyle.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderMesh.h>
#include <wendy/RenderTerrain.h>

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

void Terrain::enqueue(Queue& queue, const Transform3& transform) const
{
  mesh->enqueue(queue, transform);
}

void Terrain::calculateShadows(const Vector3& sun)
{
}

float Terrain::getHeight(const Vector2& position) const
{
  const Vector3 grid = worldToGrid(Vector3(position.x, 0.f, position.y));

  if (grid.x < 0.f || grid.z < 0.f || grid.x >= width || grid.z >= depth)
    return 0.f;

  const unsigned int cornerX = (unsigned int) grid.x;
  const unsigned int cornerZ = (unsigned int) grid.z;

  const unsigned int index = cornerZ * (width - 1) + cornerX;

  const Tile& tile = tiles[index];
  
  Vector3 normal;

  if (grid.x - floorf(grid.x) > grid.z - floorf(grid.z))
    normal = tile.normals[0];
  else
    normal = tile.normals[1];

  Vector3 corner = gridToWorld(Vector3(floorf(grid.x), 0.f, floorf(grid.z)));
  corner.y = tile.height;
  
  const float D = -corner.dotProduct(normal);

  const float height = -((normal.x * position.x + normal.z * position.y + D) / normal.y);

  return height;
}
 
const Sphere& Terrain::getBounds(void) const
{
  return bounds;
}

Terrain* Terrain::createInstance(const Image& heightmap,
			         const Image& colormap,
				 const Vector3& size,
			         const String& name)
{
  Ptr<Terrain> terrain = new Terrain(name);
  if (!terrain->init(heightmap, colormap, size))
    return NULL;

  return terrain.detachObject();
}

Terrain::Terrain(const String& name):
  Managed<Terrain>(name)
{
}

bool Terrain::init(const Image& heightmap,
                   const Image& colormap,
                   const Vector3& initSize)
{
  if (heightmap.getFormat() != ImageFormat::GREY8)
  {
    Log::writeError("Heightmap must be in GREY8 format");
    return false;
  }

  width = heightmap.getWidth();
  depth = heightmap.getHeight();

  if (width < 2 || depth < 2)
  {
    Log::writeError("Dimensions of heightmap must be at least 2");
    return false;
  }

  texture = GL::Texture::createInstance(colormap, 0);
  if (!texture)
    return false;

  // Create render style
  {
    Technique& technique = style.createTechnique();

    GL::Pass& pass = technique.createPass();

    GL::TextureLayer& layer = pass.createTextureLayer();
    layer.setCombineMode(GL_REPLACE);
    layer.setTexture(texture);
  }

  offset.set(width / 2.f, depth / 2.f);

  size = initSize;

  moira::Mesh meshData;
  meshData.vertices.resize(width * depth);
  meshData.geometries.resize(1);
  meshData.geometries[0].shaderName = style.getName();
  meshData.geometries[0].triangles.resize((width - 1) * (depth - 1) * 2);

  const unsigned char* values = (const unsigned char*) heightmap.getPixels();

  unsigned int index = 0;

  for (unsigned int z = 0;  z < depth;  z++)
  {
    for (unsigned int x = 0;  x < width;  x++)
    {
      const float height = values[x + (depth - z - 1) * width] / 255.f;

      meshData.vertices[index].position.x = size.x * (x - offset.x) / width;
      meshData.vertices[index].position.y = size.y * height;
      meshData.vertices[index].position.z = size.z * (z - offset.y) / depth;
      meshData.vertices[index].texcoord.set(x / (width - 1.f), z / (depth - 1.f));
      
      index++;
    }
  }

  index = 0;

  for (unsigned int z = 0;  z < depth - 1;  z++)
  {
    for (unsigned int x = 0;  x < width - 1;  x++)
    {
      unsigned int* indices;

      indices = meshData.geometries[0].triangles[index++].indices;

      indices[0] = z * width + x;
      indices[1] = (z + 1) * width + x;
      indices[2] = z * width + x + 1;

      indices = meshData.geometries[0].triangles[index++].indices;

      indices[0] = z * width + x + 1;
      indices[1] = (z + 1) * width + x;
      indices[2] = (z + 1) * width + x + 1;
    }
  }

  meshData.calculateNormals(moira::Mesh::SMOOTH_FACES);

  tiles.resize((width - 1) * (depth - 1));

  for (unsigned int z = 0;  z < depth - 1;  z++)
  {
    for (unsigned int x = 0;  x < width - 1;  x++)
    {
      const unsigned int index = x + z * (width - 1);

      Tile& tile = tiles[index];

      tile.normals[0] = meshData.geometries[0].triangles[index * 2].normal;
      tile.normals[1] = meshData.geometries[0].triangles[index * 2 + 1].normal;
      tile.height = meshData.vertices[x + z * width].position.y;
    }
  }

  mesh = Mesh::createInstance(meshData);
  if (!mesh)
    return false;

  bounds.center.set(0.f, 0.f, 0.f);
  bounds.radius = (size / 2.f).length();

  return true;
}

Vector3 Terrain::gridToWorld(const Vector3& grid) const
{
  const Vector3 scale(size.x / width, size.y / 255.f, size.z / depth);

  return (grid - Vector3(offset.x, 0.f, offset.y)) * scale;
}

Vector3 Terrain::worldToGrid(const Vector3& world) const
{
  const Vector3 scale(width / size.x, 255.f / size.y, depth / size.z);

  return world * scale + Vector3(offset.x, 0.f, offset.y);
}

///////////////////////////////////////////////////////////////////////

TerrainCodec::TerrainCodec(void):
  ResourceCodec<Terrain>("XML terrain codec")
{
  addSuffix("terrain");
}

Terrain* TerrainCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<Terrain>::read(path, name);
}

Terrain* TerrainCodec::read(Stream& stream, const String& name)
{
  terrainName = name;

  if (!XML::Codec::read(stream))
  {
    style = NULL;
    return NULL;
  }

  return terrain.detachObject();
}

bool TerrainCodec::write(const Path& path, const Terrain& Terrain)
{
  return ResourceCodec<Style>::write(path, style);
}

bool TerrainCodec::write(Stream& stream, const Terrain& Terrain)
{
  try
  {
    setStream(&stream);

    beginElement("terrain");
    addAttribute("version", (int) RENDER_TERRAIN_XML_VERSION);

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

bool TerrainCodec::onBeginElement(const String& name)
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

    terrain = new Terrain(terrainName);
    return true;
  }

  return true;
}

bool TerrainCodec::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

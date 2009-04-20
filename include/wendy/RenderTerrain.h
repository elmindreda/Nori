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
#ifndef WENDY_RENDERTERRAIN_H
#define WENDY_RENDERTERRAIN_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief %Terrain renderer.
 *  @ingroup renderer
 *
 *  Basic heightmap terrain renderer.
 */
class Terrain : public Renderable,
                public Resource<Terrain>,
		public RefObject<Terrain>
{
public:
  void enqueue(Queue& queue, const Transform3& transform) const;
  /*! @param[in] position The position to query.
   *  @return The height of the terrain at the specified position.
   *  @remarks The y-coordinate of the position is mapped to the z-axis.
   */
  float getHeight(const Vec2& position) const;
  const Sphere& getBounds(void) const;
  static Terrain* createInstance(const Image& heightmap,
				 const Vec3& size,
				 Material& material,
                                 const String& name = "");
private:
  struct Tile;
  Terrain(const String& name);
  bool init(const Image& heightmap, const Vec3& size, Material& material);
  Vec3 gridToWorld(const Vec3& grid) const;
  Vec3 worldToGrid(const Vec3& world) const;
  typedef std::vector<Tile> TileList;
  unsigned int width;
  unsigned int depth;
  Vec3 size;
  Vec2 offset;
  TileList tiles;
  Ref<Mesh> mesh;
  Sphere bounds;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
struct Terrain::Tile
{
  Vec3 normals[2];
  float height;
};

///////////////////////////////////////////////////////////////////////

class TerrainCodec : public ResourceCodec<Terrain>, public XML::Codec
{
public:
  TerrainCodec(void);
  Terrain* read(const Path& path, const String& name = "");
  Terrain* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Terrain& terrain);
  bool write(Stream& stream, const Terrain& terrain);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<Terrain> terrain;
  String terrainName;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERTERRAIN_H*/
///////////////////////////////////////////////////////////////////////

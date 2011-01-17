///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Vector.h>
#include <wendy/AABB.h>
#include <wendy/Sphere.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>
#include <wendy/Mesh.h>

#include <limits>
#include <cstdlib>
#include <fstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

struct Triplet
{
  unsigned int vertex;
  unsigned int normal;
  unsigned int texcoord;
};

struct Face
{
  Triplet p[3];
};

typedef std::vector<Face> FaceList;

struct FaceGroup
{
  FaceList faces;
  String name;
};

typedef std::vector<FaceGroup> FaceGroupList;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

void MeshEdge::setIndices(unsigned int a, unsigned int b)
{
  indices[0] = a;
  indices[1] = b;
}

///////////////////////////////////////////////////////////////////////

void MeshTriangle::setIndices(unsigned int a, unsigned int b, unsigned int c)
{
  indices[0] = a;
  indices[1] = b;
  indices[2] = c;
}

void MeshTriangle::setEdges(unsigned int a, unsigned int b, unsigned int c)
{
  edges[0] = a;
  edges[1] = b;
  edges[2] = c;
}

///////////////////////////////////////////////////////////////////////

Mesh::Mesh(const ResourceInfo& info):
  Resource(info)
{
}

void Mesh::merge(const Mesh& other)
{
  vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());

  for (GeometryList::const_iterator i = other.geometries.begin();  i != other.geometries.end();  i++)
  {
    MeshGeometry* geometry = findGeometry(i->shaderName);
    if (!geometry)
    {
      geometries.push_back(MeshGeometry());
      geometry = &(geometries.back());
      geometry->shaderName = i->shaderName;
    }

    geometry->triangles.insert(geometry->triangles.end(),
                               i->triangles.begin(),
                               i->triangles.end());
  }
}

void Mesh::collapseGeometries(const String& shaderName)
{
  geometries[0].shaderName = shaderName;

  for (unsigned int i = 1;  i < geometries.size();  i++)
  {
    geometries[0].triangles.insert(geometries[0].triangles.end(),
                                   geometries[i].triangles.begin(),
                                   geometries[i].triangles.end());
  }

  geometries.resize(1);
}

MeshGeometry* Mesh::findGeometry(const String& shaderName)
{
  for (unsigned int i = 0;  i < geometries.size();  i++)
  {
    if (geometries[i].shaderName == shaderName)
      return &(geometries[i]);
  }

  return NULL;
}

void Mesh::generateNormals(NormalType type)
{
  generateTriangleNormals();

  VertexMerger merger(vertices);

  if (type == SMOOTH_FACES)
    merger.setNormalMode(VertexMerger::MERGE_NORMALS);

  for (unsigned int i = 0;  i < geometries.size();  i++)
  {
    for (unsigned int j = 0;  j < geometries[i].triangles.size();  j++)
    {
      MeshTriangle& triangle = geometries[i].triangles[j];

      for (unsigned int k = 0;  k < 3;  k++)
      {
        triangle.indices[k] = merger.addAttributeLayer(triangle.indices[k],
                                                       triangle.normal,
                                                       vertices[triangle.indices[k]].texcoord);
      }
    }
  }

  merger.realizeVertices(vertices);
}

void Mesh::generateTriangleNormals(void)
{
  for (unsigned int i = 0;  i < geometries.size();  i++)
  {
    for (unsigned int j = 0;  j < geometries[i].triangles.size();  j++)
    {
      MeshTriangle& triangle = geometries[i].triangles[j];

      Vec3 one = vertices[triangle.indices[1]].position - vertices[triangle.indices[0]].position;
      Vec3 two = vertices[triangle.indices[2]].position - vertices[triangle.indices[0]].position;
      triangle.normal = one.cross(two);
      triangle.normal.normalize();
    }
  }
}

void Mesh::generateEdges(void)
{
  for (unsigned int i = 0;  i < geometries.size();  i++)
  {
    for (unsigned int j = 0;  j < geometries[i].triangles.size();  j++)
    {
      for (unsigned int k = 0;  k < 3;  k++)
      {
        MeshEdge edge;
        edge.setIndices(geometries[i].triangles[j].indices[k],
                        geometries[i].triangles[j].indices[(k + 1) % 3]);

        unsigned int l;

        for (l = 0;  l < edges.size();  l++)
        {
          if (edges[l].indices[0] == edge.indices[0] &&
              edges[l].indices[1] == edge.indices[1])
            break;
          if (edges[l].indices[0] == edge.indices[1] &&
              edges[l].indices[1] == edge.indices[0])
            break;
        }

        if (l == edges.size())
        {
          geometries[i].triangles[j].edges[k] = edges.size();
          edges.push_back(edge);
        }
        else
          geometries[i].triangles[j].edges[k] = l;
      }
    }
  }
}

void Mesh::generateBounds(AABB& bounds) const
{
  if (vertices.empty())
  {
    bounds.center = bounds.size = Vec3::ZERO;
    return;
  }

  typedef std::numeric_limits<float> limits;

  float minX = limits::max(), minY = limits::max(), minZ = limits::max();
  float maxX = limits::min(), maxY = limits::min(), maxZ = limits::min();

  for (unsigned int i = 0;  i < vertices.size();  i++)
  {
    const Vec3& position = vertices[i].position;

    if (position.x < minX)
      minX = position.x;
    if (position.y < minY)
      minY = position.y;
    if (position.z < minZ)
      minZ = position.z;
    if (position.x > maxX)
      maxX = position.x;
    if (position.y > maxY)
      maxY = position.y;
    if (position.z > maxZ)
      maxZ = position.z;
  }

  bounds.setBounds(minX, minY, minZ, maxX, maxY, maxZ);
}

void Mesh::generateBounds(Sphere& bounds) const
{
  bounds.radius = 0.f;

  if (vertices.empty())
  {
    bounds.center = Vec3::ZERO;
    return;
  }

  bounds.center = vertices[0].position;

  for (unsigned int i = 1;  i < vertices.size();  i++)
    bounds.envelop(vertices[i].position);
}

bool Mesh::isValid(void) const
{
  if (vertices.size() == 0)
    return false;

  unsigned int index;

  for (index = 0;  index < geometries.size();  index++)
  {
    if (geometries[index].triangles.size() > 0)
      break;
  }

  if (index == geometries.size())
    return false;

  // TODO: Triangle, edge and vertex validation.

  return true;
}

unsigned int Mesh::getTriangleCount(void) const
{
  unsigned int count = 0;

  for (unsigned int i = 0;  i < geometries.size();  i++)
    count += (unsigned int) geometries[i].triangles.size();

  return count;
}

Ref<Mesh> Mesh::read(ResourceIndex& index, const Path& path)
{
  MeshReader reader(index);
  return reader.read(path);
}

///////////////////////////////////////////////////////////////////////

VertexMerger::VertexMerger(void):
  targetCount(0),
  mode(PRESERVE_NORMALS)
{
}

VertexMerger::VertexMerger(const Mesh::VertexList& initVertices):
  targetCount(0),
  mode(PRESERVE_NORMALS)
{
  importPositions(initVertices);
}

void VertexMerger::importPositions(const Mesh::VertexList& initVertices)
{
  vertices.resize(initVertices.size());
  for (unsigned int i = 0;  i < vertices.size();  i++)
    vertices[i].position = initVertices[i].position;
}

unsigned int VertexMerger::addAttributeLayer(unsigned int vertexIndex,
                                             const Vec3& normal,
                                             const Vec2& texcoord)
{
  Vertex& vertex = vertices[vertexIndex];

  if (mode == PRESERVE_NORMALS)
  {
    for (Vertex::LayerList::iterator i = vertex.layers.begin();  i != vertex.layers.end();  i++)
    {
      if (i->normal.dot(normal) > 0.95f && i->texcoord == texcoord)
        return i->index;
    }

    vertex.layers.push_back(VertexLayer());
    VertexLayer& layer = vertex.layers.back();

    layer.normal = normal;
    layer.texcoord = texcoord;
    layer.index = targetCount++;

    return layer.index;
  }
  else
  {
    unsigned int index = 0;

    for (Vertex::LayerList::iterator i = vertex.layers.begin();  i != vertex.layers.end();  i++)
    {
      if (i->texcoord == texcoord)
        break;

      index++;
    }

    if (index == vertex.layers.size())
    {
      vertex.layers.push_back(VertexLayer());
      VertexLayer& layer = vertex.layers.back();

      layer.normal = Vec3::ZERO;
      layer.normal += vertex.layers.front().normal;
      layer.texcoord = texcoord;
      layer.index = targetCount++;

      index = vertex.layers.size() - 1;
    }

    for (Vertex::LayerList::iterator i = vertex.layers.begin();  i != vertex.layers.end();  i++)
      i->normal += normal;

    return vertex.layers[index].index;
  }
}

void VertexMerger::realizeVertices(Mesh::VertexList& result) const
{
  result.resize(targetCount);

  for (unsigned int i = 0;  i < vertices.size();  i++)
  {
    const Vertex& vertex = vertices[i];

    for (unsigned int j = 0;  j < vertex.layers.size();  j++)
    {
      const VertexLayer& layer = vertex.layers[j];

      result[layer.index].position = vertex.position;
      result[layer.index].normal = layer.normal;
      result[layer.index].texcoord = layer.texcoord;
    }
  }
}

void VertexMerger::setNormalMode(NormalMode newMode)
{
  mode = newMode;
}

///////////////////////////////////////////////////////////////////////

MeshReader::MeshReader(ResourceIndex& index):
  ResourceReader(index)
{
}

Ref<Mesh> MeshReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Mesh*>(cache);

  ResourceInfo info(getIndex(), path);

  std::ifstream stream;
  if (!getIndex().openFile(stream, info.path))
    return NULL;

  String line;

  std::vector<Vec3> positions;
  std::vector<Vec3> normals;
  std::vector<Vec2> texcoords;

  FaceGroupList groups;
  FaceGroup* group = NULL;

  while (std::getline(stream, line))
  {
    const char* text = line.c_str();

    if (!interesting(&text))
      continue;

    String command = parseName(&text);

    if (command == "g")
    {
      // Nothing to do here
    }
    else if (command == "o")
    {
      // Nothing to do here
    }
    else if (command == "s")
    {
      // Nothing to do here
    }
    else if (command == "v")
    {
      Vec3 vertex;

      vertex.x = parseFloat(&text);
      vertex.y = parseFloat(&text);
      vertex.z = parseFloat(&text);
      positions.push_back(vertex);
    }
    else if (command == "vt")
    {
      Vec2 texcoord;

      texcoord.x = parseFloat(&text);
      texcoord.y = parseFloat(&text);
      texcoords.push_back(texcoord);
    }
    else if (command == "vn")
    {
      Vec3 normal;

      normal.x = parseFloat(&text);
      normal.y = parseFloat(&text);
      normal.z = parseFloat(&text);
      normals.push_back(normal);
    }
    else if (command == "usemtl")
    {
      String shaderName = parseName(&text);

      group = NULL;

      for (FaceGroupList::iterator g = groups.begin();  g != groups.end();  g++)
      {
        if (g->name == shaderName)
          group = &(*g);
      }

      if (!group)
      {
        groups.push_back(FaceGroup());
        groups.back().name = shaderName;
        group = &(groups.back());
      }
    }
    else if (command == "mtllib")
    {
      // Silently ignore mtllib
    }
    else if (command == "f")
    {
      if (!group)
      {
        logError("Expected \'usemtl\' but found \'f\' in OBJ file");
        return NULL;
      }

      std::vector<Triplet> triplets;

      while (*text != '\0')
      {
        triplets.push_back(Triplet());
        Triplet& triplet = triplets.back();

        triplet.vertex = parseInteger(&text);

        if (*text++ != '/')
        {
          logError("Expected but missing \'/\' in OBJ file");
          return NULL;
        }

        triplet.texcoord = 0;
        if (std::isdigit(*text))
          triplet.texcoord = parseInteger(&text);

        if (*text++ != '/')
        {
          logError("Expected but missing \'/\' in OBJ file");
          return NULL;
        }

        triplet.normal = 0;
        if (std::isdigit(*text))
          triplet.normal = parseInteger(&text);

        while (std::isspace(*text))
          text++;
      }

      for (unsigned int i = 2;  i < triplets.size();  i++)
      {
        group->faces.push_back(Face());
        Face& face = group->faces.back();

        face.p[0] = triplets[0];
        face.p[1] = triplets[i - 1];
        face.p[2] = triplets[i];
      }
    }
    else
      logWarning("Unknown command \'%s\' in OBJ file", command.c_str());
  }

  Ref<Mesh> mesh = new Mesh(info);

  mesh->vertices.resize(positions.size());

  for (size_t i = 0;  i < positions.size();  i++)
    mesh->vertices[i].position = positions[i];

  VertexMerger merger(mesh->vertices);

  for (FaceGroupList::const_iterator g = groups.begin();  g != groups.end();  g++)
  {
    mesh->geometries.push_back(MeshGeometry());
    MeshGeometry& geometry = mesh->geometries.back();

    const FaceList& faces = g->faces;

    geometry.shaderName = g->name;
    geometry.triangles.resize(faces.size());

    for (size_t i = 0;  i < faces.size();  i++)
    {
      const Face& face = faces[i];
      MeshTriangle& triangle = geometry.triangles[i];

      for (int j = 0;  j < 3;  j++)
      {
        const Triplet& point = face.p[j];

        Vec3 normal = Vec3::ZERO;
        if (point.normal)
          normal = normals[point.normal - 1];

        Vec2 texcoord = Vec2::ZERO;
        if (point.texcoord)
          texcoord = texcoords[point.texcoord - 1];

        triangle.indices[j] = merger.addAttributeLayer(point.vertex - 1, normal, texcoord);
      }
    }
  }

  merger.realizeVertices(mesh->vertices);
  return mesh;
}

String MeshReader::parseName(const char** text)
{
  while (std::isspace(**text))
    (*text)++;

  String result;

  while (std::isalnum(**text) || **text == '_')
  {
    result.append(1, **text);
    (*text)++;
  }

  if (!result.size())
    throw Exception("Expected but missing name in OBJ file");

  return result;
}

int MeshReader::parseInteger(const char** text)
{
  char* end;

  const int result = std::strtol(*text, &end, 0);
  if (end == *text)
    throw Exception("Expected but missing integer value in OBJ file");

  *text = end;
  return result;
}

float MeshReader::parseFloat(const char** text)
{
  char* end;

  const float result = strtof(*text, &end);
  if (end == *text)
    throw Exception("Expected but missing float value in OBJ file");

  *text = end;
  return result;
}

bool MeshReader::interesting(const char** text)
{
  if (std::isspace(**text) || **text == '#' || **text == '\0' || **text == '\r')
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

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
#include <wendy/Portability.h>
#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Vector.h>
#include <wendy/AABB.h>
#include <wendy/Sphere.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Mesh.h>

#include <limits>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

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

Mesh::Mesh(const String& name):
  Resource<Mesh>(name)
{
}

void Mesh::weld(float tolerance)
{
  std::vector<unsigned int> indices;

  tolerance *= tolerance;

  for (unsigned int i = 0;  i < vertices.size();  i++)
  {
    for (unsigned int j = i + 1;  j < vertices.size();  j++)
    {
	    /*
      if ((vertices[i].position - vertices[j].position).lengthSquared() <= tolerance)
		    ;
      */
    }
  }
}

void Mesh::merge(const Mesh& other)
{
  vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());

  for (GeometryList::const_iterator i = other.geometries.begin();  i != other.geometries.end();  i++)
  {
    MeshGeometry* geometry = findGeometry((*i).shaderName);
    if (!geometry)
    {
      geometries.push_back(MeshGeometry());
      geometry = &(geometries.back());
      geometry->shaderName = (*i).shaderName;
    }

    geometry->triangles.insert(geometry->triangles.end(),
                               (*i).triangles.begin(),
                               (*i).triangles.end());
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

void Mesh::calculateNormals(NormalType type)
{
  calculateTriangleNormals();

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

void Mesh::calculateTriangleNormals(void)
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

void Mesh::calculateEdges(void)
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

void Mesh::getBounds(AABB& bounds) const
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

void Mesh::getBounds(Sphere& bounds) const
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
    for (unsigned int i = 0;  i < vertex.layers.size();  i++)
    {
      if (vertex.layers[i].normal.dot(normal) > 0.95f &&
          vertex.layers[i].texcoord == texcoord)
        return vertex.layers[i].index;
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

    while (index < vertex.layers.size())
    {
      if (vertex.layers[index].texcoord == texcoord)
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

    for (unsigned int i = 0;  i < vertex.layers.size();  i++)
      vertex.layers[i].normal += normal;

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

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

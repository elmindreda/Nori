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
#include <wendy/AABB.h>
#include <wendy/Sphere.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>
#include <wendy/Mesh.h>

#include <limits>
#include <cstdlib>
#include <fstream>
#include <cctype>

#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/epsilon.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

class VertexTool
{
public:
  enum NormalMode
  {
    PRESERVE_NORMALS,
    MERGE_NORMALS
  };
  VertexTool();
  VertexTool(const Mesh::VertexList& vertices);
  void importPositions(const Mesh::VertexList& vertices);
  uint32 addAttributeLayer(uint32 vertexIndex,
                           const vec3& normal,
                           const vec2& texcoord = vec2(0.f));
  void realizeVertices(Mesh::VertexList& result) const;
  void setNormalMode(NormalMode newMode);
private:
  struct VertexLayer
  {
    vec3 normal;
    vec2 texcoord;
    uint32 index;
  };
  struct Vertex
  {
    typedef std::vector<VertexLayer> LayerList;
    vec3 position;
    LayerList layers;
  };
  typedef std::vector<Vertex> VertexList;
  VertexList vertices;
  uint32 targetCount;
  NormalMode mode;
};

VertexTool::VertexTool():
  targetCount(0),
  mode(PRESERVE_NORMALS)
{
}

VertexTool::VertexTool(const Mesh::VertexList& initVertices):
  targetCount(0),
  mode(PRESERVE_NORMALS)
{
  importPositions(initVertices);
}

void VertexTool::importPositions(const Mesh::VertexList& initVertices)
{
  vertices.resize(initVertices.size());
  for (size_t i = 0;  i < vertices.size();  i++)
    vertices[i].position = initVertices[i].position;
}

uint32 VertexTool::addAttributeLayer(uint32 vertexIndex,
                                     const vec3& normal,
                                     const vec2& texcoord)
{
  Vertex& vertex = vertices[vertexIndex];

  if (mode == PRESERVE_NORMALS)
  {
    for (auto l = vertex.layers.begin();  l != vertex.layers.end();  l++)
    {
      if (all(equalEpsilon(l->normal, normal, 0.001f)) &&
          all(equalEpsilon(l->texcoord, texcoord, 0.001f)))
      {
        return l->index;
      }
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
    size_t index;
    bool discontinuous = true;

    for (auto l = vertex.layers.begin();  l != vertex.layers.end();  l++)
    {
      if (all(equalEpsilon(l->texcoord, texcoord, 0.001f)))
      {
        if (all(equalEpsilon(l->normal, normal, 0.001f)))
          return l->index;

        discontinuous = false;
        index = l->index;
      }
    }

    if (discontinuous)
      index = targetCount++;

    vertex.layers.push_back(VertexLayer());
    VertexLayer& layer = vertex.layers.back();

    layer.normal = normal;
    layer.texcoord = texcoord;
    layer.index = index;

    return index;
  }
}

void VertexTool::realizeVertices(Mesh::VertexList& result) const
{
  result.resize(targetCount);

  for (auto v = vertices.begin();  v != vertices.end();  v++)
  {
    vec3 normal;

    if (mode == MERGE_NORMALS)
    {
      for (auto l = v->layers.begin();  l != v->layers.end();  l++)
        normal += l->normal;

      normal = normalize(normal);
    }

    for (auto l = v->layers.begin();  l != v->layers.end();  l++)
    {
      result[l->index].position = v->position;
      result[l->index].texcoord = l->texcoord;

      if (mode == MERGE_NORMALS)
        result[l->index].normal = normal;
      else
        result[l->index].normal = l->normal;
    }
  }
}

void VertexTool::setNormalMode(NormalMode newMode)
{
  mode = newMode;
}

struct Triplet
{
  uint32 vertex;
  uint32 normal;
  uint32 texcoord;
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

void MeshTriangle::setIndices(uint32 a, uint32 b, uint32 c)
{
  indices[0] = a;
  indices[1] = b;
  indices[2] = c;
}

///////////////////////////////////////////////////////////////////////

Mesh::Mesh(const ResourceInfo& info):
  Resource(info)
{
}

void Mesh::mergeSections(const char* materialName)
{
  sections[0].materialName = materialName;

  for (size_t i = 1;  i < sections.size();  i++)
  {
    sections[0].triangles.insert(sections[0].triangles.end(),
                                 sections[i].triangles.begin(),
                                 sections[i].triangles.end());
  }

  sections.resize(1);
}

MeshSection* Mesh::findSection(const char* materialName)
{
  for (auto s = sections.begin();  s != sections.end();  s++)
  {
    if (s->materialName == materialName)
      return &(*s);
  }

  return NULL;
}

void Mesh::generateNormals(NormalType type)
{
  generateTriangleNormals();

  VertexTool tool(vertices);

  if (type == SMOOTH_FACES)
    tool.setNormalMode(VertexTool::MERGE_NORMALS);

  for (auto s = sections.begin();  s != sections.end();  s++)
  {
    for (auto t = s->triangles.begin();  t != s->triangles.end();  t++)
    {
      for (size_t k = 0;  k < 3;  k++)
      {
        t->indices[k] = tool.addAttributeLayer(t->indices[k],
                                               t->normal,
                                               vertices[t->indices[k]].texcoord);
      }
    }
  }

  tool.realizeVertices(vertices);
}

void Mesh::generateTriangleNormals()
{
  for (auto s = sections.begin();  s != sections.end();  s++)
  {
    for (auto t = s->triangles.begin();  t != s->triangles.end();  t++)
    {
      const vec3 one = vertices[t->indices[1]].position -
                       vertices[t->indices[0]].position;
      const vec3 two = vertices[t->indices[2]].position -
                       vertices[t->indices[0]].position;

      t->normal = normalize(cross(one, two));
    }
  }
}

AABB Mesh::generateBoundingAABB() const
{
  AABB bounds;

  if (vertices.empty())
    return bounds;

  vec3 minimum(std::numeric_limits<float>::max());
  vec3 maximum(std::numeric_limits<float>::min());

  for (auto v = vertices.begin();  v != vertices.end();  v++)
  {
    minimum = min(minimum, v->position);
    maximum = max(maximum, v->position);
  }

  bounds.setBounds(minimum.x, minimum.y, minimum.z,
                   maximum.x, maximum.y, maximum.z);

  return bounds;
}

Sphere Mesh::generateBoundingSphere() const
{
  Sphere bounds;

  if (vertices.empty())
    return bounds;

  bounds.center = vertices[0].position;

  for (size_t i = 1;  i < vertices.size();  i++)
    bounds.envelop(vertices[i].position);

  return bounds;
}

bool Mesh::isValid() const
{
  if (vertices.empty())
    return false;

  for (auto v = vertices.begin();  v != vertices.end();  v++)
  {
    if (!all(isfinite(v->position)) ||
        !all(isfinite(v->normal)) ||
        !all(isfinite(v->texcoord)))
    {
      return false;
    }
  }

  for (auto s = sections.begin();  s != sections.end();  s++)
  {
    if (s->triangles.empty())
      return false;

    for (auto t = s->triangles.begin();  t != s->triangles.end();  t++)
    {
      if (!all(isfinite(t->normal)))
        return false;

      if (t->indices[0] >= vertices.size() ||
          t->indices[1] >= vertices.size() ||
          t->indices[2] >= vertices.size())
      {
        return false;
      }
    }
  }

  return true;
}

size_t Mesh::getTriangleCount() const
{
  size_t count = 0;

  for (auto s = sections.begin();  s != sections.end();  s++)
    count += s->triangles.size();

  return count;
}

Ref<Mesh> Mesh::read(ResourceCache& cache, const String& name)
{
  MeshReader reader(cache);
  return reader.read(name);
}

///////////////////////////////////////////////////////////////////////

MeshReader::MeshReader(ResourceCache& index):
  ResourceReader<Mesh>(index)
{
}

Ref<Mesh> MeshReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.asString().c_str(), std::ios::in | std::ios::binary);
  if (stream.fail())
  {
    logError("Failed to open mesh \'%s\'", name.c_str());
    return NULL;
  }

  String line;
  unsigned int lineNumber = 0;

  std::vector<vec3> positions;
  std::vector<vec3> normals;
  std::vector<vec2> texcoords;
  std::vector<Triplet> triplets;

  FaceGroupList groups;
  FaceGroup* group = NULL;

  while (std::getline(stream, line))
  {
    const char* text = line.c_str();
    ++lineNumber;

    if (!interesting(&text))
      continue;

    try
    {
      String command = parseName(&text);

      if (command == "g")
      {
        // Silently ignore group names
      }
      else if (command == "o")
      {
        // Silently ignore object names
      }
      else if (command == "s")
      {
        // Silently ignore smoothing
      }
      else if (command == "v")
      {
        vec3 vertex;

        vertex.x = parseFloat(&text);
        vertex.y = parseFloat(&text);
        vertex.z = parseFloat(&text);
        positions.push_back(vertex);
      }
      else if (command == "vt")
      {
        vec2 texcoord;

        texcoord.x = parseFloat(&text);
        texcoord.y = parseFloat(&text);
        texcoords.push_back(texcoord);
      }
      else if (command == "vn")
      {
        vec3 normal;

        normal.x = parseFloat(&text);
        normal.y = parseFloat(&text);
        normal.z = parseFloat(&text);
        normals.push_back(normalize(normal));
      }
      else if (command == "usemtl")
      {
        String materialName = parseName(&text);

        group = NULL;

        for (auto g = groups.begin();  g != groups.end();  g++)
        {
          if (g->name == materialName)
            group = &(*g);
        }

        if (!group)
        {
          groups.push_back(FaceGroup());
          groups.back().name = materialName;
          group = &(groups.back());
        }
      }
      else if (command == "mtllib")
      {
        // Silently ignore .mtl material files
      }
      else if (command == "f")
      {
        if (!group)
          throw Exception("Expected \'usemtl\' before \'f\'");

        triplets.clear();

        while (*text != '\0')
        {
          triplets.push_back(Triplet());
          Triplet& triplet = triplets.back();

          triplet.vertex = parseInteger(&text);
          triplet.texcoord = 0;
          triplet.normal = 0;

          if (*text == '/')
          {
            if (std::isdigit(*(++text)))
              triplet.texcoord = parseInteger(&text);

            if (*text == '/')
            {
              if (std::isdigit(*(++text)))
                triplet.normal = parseInteger(&text);
            }
          }

          while (std::isspace(*text))
            text++;
        }

        for (size_t i = 2;  i < triplets.size();  i++)
        {
          group->faces.push_back(Face());
          Face& face = group->faces.back();

          face.p[0] = triplets[0];
          face.p[1] = triplets[i - 1];
          face.p[2] = triplets[i];
        }
      }
      else
      {
        logWarning("Unknown command \'%s\' in mesh \'%s\' line %d",
                   command.c_str(),
                   name.c_str(),
                   lineNumber);
      }
    }
    catch (Exception& e)
    {
      logError("%s in mesh \'%s\' line %d",
               e.what(),
               name.c_str(),
               lineNumber);

      return NULL;
    }
  }

  Ref<Mesh> mesh = new Mesh(ResourceInfo(cache, name, path));

  mesh->vertices.resize(positions.size());

  for (size_t i = 0;  i < positions.size();  i++)
    mesh->vertices[i].position = positions[i];

  VertexTool tool(mesh->vertices);

  for (auto g = groups.begin();  g != groups.end();  g++)
  {
    mesh->sections.push_back(MeshSection());
    MeshSection& geometry = mesh->sections.back();

    const FaceList& faces = g->faces;

    geometry.materialName = g->name;
    geometry.triangles.resize(faces.size());

    for (size_t i = 0;  i < faces.size();  i++)
    {
      const Face& face = faces[i];
      MeshTriangle& triangle = geometry.triangles[i];

      for (size_t j = 0;  j < 3;  j++)
      {
        const Triplet& point = face.p[j];

        vec3 normal;
        if (point.normal)
          normal = normals[point.normal - 1];

        vec2 texcoord;
        if (point.texcoord)
          texcoord = texcoords[point.texcoord - 1];

        triangle.indices[j] = tool.addAttributeLayer(point.vertex - 1, normal, texcoord);
      }
    }
  }

  tool.realizeVertices(mesh->vertices);
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
    throw Exception("Expected but missing name");

  return result;
}

int MeshReader::parseInteger(const char** text)
{
  char* end;

  const int result = std::strtol(*text, &end, 0);
  if (end == *text)
    throw Exception("Expected but missing integer value");

  *text = end;
  return result;
}

float MeshReader::parseFloat(const char** text)
{
  char* end;

  const float result = float(std::strtod(*text, &end));
  if (end == *text)
    throw Exception("Expected but missing float value");

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

bool MeshWriter::write(const Path& path, const Mesh& mesh)
{
  std::ofstream stream(path.asString().c_str());
  if (!stream.is_open())
  {
    logError("Failed to open \'%s\' for writing",
             path.asString().c_str());
    return false;
  }

  for (auto v = mesh.vertices.begin();  v != mesh.vertices.end();  v++)
    stream << "v " << v->position.x << ' ' << v->position.y << ' ' << v->position.z << '\n';

  for (auto v = mesh.vertices.begin();  v != mesh.vertices.end();  v++)
    stream << "vn " << v->normal.x << ' ' << v->normal.y << ' ' << v->normal.z << '\n';

  for (auto v = mesh.vertices.begin();  v != mesh.vertices.end();  v++)
    stream << "vt " << v->texcoord.x << ' ' << v->texcoord.y << '\n';

  for (auto s = mesh.sections.begin();  s != mesh.sections.end();  s++)
  {
    stream << "usemtl " << s->materialName << '\n';

    for (auto t = s->triangles.begin();  t != s->triangles.end();  t++)
    {
      stream << "f";

      for (size_t i = 0;  i < 3;  i++)
      {
        uint32 index = t->indices[i] + 1;
        stream << ' ' << index << '/' << index << '/' << index;
      }

      stream << '\n';
    }
  }

  stream.close();
  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

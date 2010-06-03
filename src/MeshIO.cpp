///////////////////////////////////////////////////////////////////////
// Wendy core library
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

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Vector.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/XML.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Mesh.h>

#include <internal/MeshIO.h>

#include <cctype>

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

const unsigned int MESH_XML_VERSION = 1;

}

///////////////////////////////////////////////////////////////////////

MeshCodecOBJ::MeshCodecOBJ(void):
  MeshCodec("OBJ mesh codec")
{
  addSuffix("obj");
}

Mesh* MeshCodecOBJ::read(const Path& path, const String& name)
{
  return MeshCodec::read(path, name);
}

Mesh* MeshCodecOBJ::read(Stream& stream, const String& name)
{
  Ptr<TextStream> source(TextStream::createInstance(&stream, false));
  if (!source)
    return NULL;

  String line;

  std::vector<Vec3> positions;
  std::vector<Vec3> normals;
  std::vector<Vec2> texcoords;

  FaceGroupList groups;
  FaceGroup* group = NULL;

  String meshName;

  while (source->readLine(line))
  {
    const char* text = line.c_str();

    if (!interesting(&text))
      continue;

    String command = readName(&text);

    if (command == "g")
    {
      meshName = readName(&text);
    }
    else if (command == "o")
    {
      meshName = readName(&text);
    }
    else if (command == "s")
    {
      // Oh, shut up
    }
    else if (command == "v")
    {
      Vec3 vertex;

      vertex.x = readFloat(&text);
      vertex.y = readFloat(&text);
      vertex.z = readFloat(&text);
      positions.push_back(vertex);
    }
    else if (command == "vt")
    {
      Vec2 texcoord;

      texcoord.x = readFloat(&text);
      texcoord.y = readFloat(&text);
      texcoords.push_back(texcoord);
    }
    else if (command == "vn")
    {
      Vec3 normal;

      normal.x = readFloat(&text);
      normal.y = readFloat(&text);
      normal.z = readFloat(&text);
      normals.push_back(normal);
    }
    else if (command == "usemtl")
    {
      String shaderName = readName(&text);

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
        Log::writeError("Expected \'usemtl\' but found \'f\' in OBJ file");
        return NULL;
      }

      std::vector<Triplet> triplets;

      while (*text != '\0')
      {
	triplets.push_back(Triplet());
	Triplet& triplet = triplets.back();

	triplet.vertex = readInteger(&text);

	if (*text++ != '/')
	{
	  Log::writeError("Expected but missing \'/\' in OBJ file");
	  return NULL;
	}

	triplet.texcoord = 0;
	if (std::isdigit(*text))
	  triplet.texcoord = readInteger(&text);

	if (*text++ != '/')
	{
	  Log::writeError("Expected but missing \'/\' in OBJ file");
	  return NULL;
	}

	triplet.normal = 0;
	if (std::isdigit(*text))
	  triplet.normal = readInteger(&text);

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
      Log::writeWarning("Unknown command \'%s\' in OBJ file", command.c_str());
  }

  Ptr<Mesh> mesh(new Mesh(meshName));

  mesh->vertices.resize(positions.size());

  for (unsigned int i = 0;  i < positions.size();  i++)
    mesh->vertices[i].position = positions[i];

  VertexMerger merger(mesh->vertices);

  for (FaceGroupList::const_iterator g = groups.begin();  g != groups.end();  g++)
  {
    mesh->geometries.push_back(MeshGeometry());
    MeshGeometry& geometry = mesh->geometries.back();

    const FaceList& faces = g->faces;

    geometry.shaderName = g->name;
    geometry.triangles.resize(faces.size());

    for (unsigned int i = 0;  i < faces.size();  i++)
    {
      const Face& face = faces[i];
      MeshTriangle& triangle = geometry.triangles[i];

      for (unsigned int j = 0;  j < 3;  j++)
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

  return mesh.detachObject();
}

bool MeshCodecOBJ::write(const Path& path, const Mesh& mesh)
{
  return MeshCodec::write(path, mesh);
}

bool MeshCodecOBJ::write(Stream& stream, const Mesh& mesh)
{
  // TODO: Implement.

  return false;
}

String MeshCodecOBJ::readName(const char** text)
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

int MeshCodecOBJ::readInteger(const char** text)
{
  char* end;

  const int result = std::strtol(*text, &end, 0);
  if (end == *text)
    throw Exception("Expected but missing integer value in OBJ file");

  *text = end;
  return result;
}

float MeshCodecOBJ::readFloat(const char** text)
{
  char* end;

  const float result = strtof(*text, &end);
  if (end == *text)
    throw Exception("Expected but missing float value in OBJ file");

  *text = end;
  return result;
}

bool MeshCodecOBJ::interesting(const char** text)
{
  if (std::isspace(**text) || **text == '#' || **text == '\0' || **text == '\r')
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

MeshCodecXML::MeshCodecXML(void):
  MeshCodec("XML mesh codec")
{
  addSuffix("mesh");
}

Mesh* MeshCodecXML::read(const Path& path, const String& name)
{
  return MeshCodec::read(path, name);
}

Mesh* MeshCodecXML::read(Stream& stream, const String& name)
{
  currentGeometry = NULL;
  currentTriangle = NULL;
  currentVertex = NULL;

  meshName = name;

  if (!XML::Codec::read(stream))
    return NULL;

  return mesh.detachObject();
}

bool MeshCodecXML::write(const Path& path, const Mesh& mesh)
{
  return MeshCodec::write(path, mesh);
}

bool MeshCodecXML::write(Stream& stream, const Mesh& mesh)
{
  try
  {
    setStream(&stream);

    beginElement("mesh");
    addAttribute("version", MESH_XML_VERSION);

    for (unsigned int i = 0;  i < mesh.geometries.size();  i++)
    {
      const MeshGeometry& geometry = mesh.geometries[i];

      beginElement("geometry");
      addAttribute("shader", geometry.shaderName);

      for (unsigned int j = 0;  j < geometry.triangles.size();  j++)
      {
	const MeshTriangle& triangle = geometry.triangles[j];

        beginElement("triangle");
        addAttribute("a", (int) triangle.indices[0]);
        addAttribute("b", (int) triangle.indices[1]);
        addAttribute("c", (int) triangle.indices[2]);

        beginElement("normal");
        addAttributes(triangle.normal);
        endElement();

        endElement();
      }

      endElement();
    }

    for (unsigned int i = 0;  i < mesh.vertices.size();  i++)
    {
      const MeshVertex& vertex = mesh.vertices[i];

      beginElement("vertex");
      addAttributes(vertex.position);

      beginElement("normal");
      addAttributes(vertex.normal);
      endElement();

      beginElement("texcoord");
      addAttributes(vertex.texcoord);
      endElement();

      endElement();
    }

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write mesh \'%s\': %s", mesh.getName().c_str(), exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool MeshCodecXML::onBeginElement(const String& name)
{
  if (name == "mesh")
  {
    mesh = new Mesh(meshName);

    const unsigned int version = readInteger("version");
    if (version != MESH_XML_VERSION)
    {
      Log::writeError("Mesh XML format version mismatch");
      return false;
    }

    return true;
  }

  if (mesh)
  {
    if (name == "vertex")
    {
      mesh->vertices.push_back(MeshVertex());
      currentVertex = &(mesh->vertices.back());
      readAttributes(currentVertex->position);
      return true;
    }

    if (name == "geometry")
    {
      mesh->geometries.push_back(MeshGeometry());
      currentGeometry = &(mesh->geometries.back());
      currentGeometry->shaderName = readString("shader");
      return true;
    }

    if (currentVertex)
    {
      if (name == "normal")
      {
        readAttributes(currentVertex->normal);
        return true;
      }

      if (name == "texcoord")
      {
	readAttributes(currentVertex->texcoord);
	return true;
      }
    }

    if (currentGeometry)
    {
      if (name == "triangle")
      {
        currentGeometry->triangles.push_back(MeshTriangle());
        currentTriangle = &(currentGeometry->triangles.back());
        currentTriangle->indices[0] = readInteger("a");
        currentTriangle->indices[1] = readInteger("b");
        currentTriangle->indices[2] = readInteger("c");
        return true;
      }

      if (currentTriangle)
      {
        if (name == "normal")
        {
          readAttributes(currentTriangle->normal);
          return true;
        }
      }
    }
  }

  return true;
}

bool MeshCodecXML::onEndElement(const String& name)
{
  if (name == "vertex")
    currentVertex = NULL;

  if (name == "geometry")
    currentGeometry = NULL;

  if (name == "triangle")
    currentTriangle = NULL;

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

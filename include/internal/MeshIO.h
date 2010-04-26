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
#ifndef WENDY_MESHIO_H
#define WENDY_MESHIO_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

typedef ResourceCodec<Mesh> MeshCodec;

///////////////////////////////////////////////////////////////////////

class MeshCodecOBJ : public MeshCodec
{
public:
  MeshCodecOBJ(void);
  Mesh* read(const Path& path, const String& name = "");
  Mesh* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Mesh& mesh);
  bool write(Stream& stream, const Mesh& mesh);
private:
  String readName(const char** text);
  int readInteger(const char** text);
  float readFloat(const char** text);
  bool interesting(const char** text);
};

///////////////////////////////////////////////////////////////////////

class MeshCodecXML : public MeshCodec, public XML::Codec
{
public:
  MeshCodecXML(void);
  Mesh* read(const Path& path, const String& name = "");
  Mesh* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Mesh& mesh);
  bool write(Stream& stream, const Mesh& mesh);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<Mesh> mesh;
  String meshName;
  MeshGeometry* currentGeometry;
  MeshTriangle* currentTriangle;
  MeshVertex* currentVertex;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_MESHIO_H*/
///////////////////////////////////////////////////////////////////////

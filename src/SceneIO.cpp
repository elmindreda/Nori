///////////////////////////////////////////////////////////////////////
// Wendy scene graph
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

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderMesh.h>
#include <wendy/RenderParticle.h>

#include <wendy/SceneGraph.h>

#include <internal/SceneIO.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace scene
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int GRAPH_XML_VERSION = 1;

}

///////////////////////////////////////////////////////////////////////

GraphCodecXML::GraphCodecXML(void):
  GraphCodec("Scene graph XML codec")
{
  addSuffix("graph");
}

Graph* GraphCodecXML::read(const Path& path, const String& name)
{
  return GraphCodec::read(path, name);
}

Graph* GraphCodecXML::read(Stream& stream, const String& name)
{
  graphName = name;

  currentNode = NULL;

  if (!XML::Codec::read(stream))
  {
    graph = NULL;
    return NULL;
  }

  if (!graph)
  {
    Log::writeError("No scene graph specification found in file");
    return NULL;
  }

  return graph.detachObject();
}

bool GraphCodecXML::write(const Path& path, const Graph& graph)
{
  return GraphCodec::write(path, graph);
}

bool GraphCodecXML::write(Stream& stream, const Graph& graph)
{
  try
  {
    setStream(&stream);

    beginElement("graph");
    addAttribute("version", GRAPH_XML_VERSION);

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write scene graph \'%s\': %s",
                    graph.getName().c_str(),
		    exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool GraphCodecXML::onBeginElement(const String& name)
{
  if (name == "graph")
  {
    if (graph)
    {
      Log::writeError("Only one scene graph per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != GRAPH_XML_VERSION)
    {
      Log::writeError("Scene graph XML format version mismatch");
      return false;
    }

    graph = new Graph(graphName);
    return true;
  }

  if (graph)
  {
    if (name == "node")
    {
      String typeName = readString("type");

      NodeType* type = NodeType::findInstance(typeName);
      if (!type)
      {
	Log::writeError("Scene graph node type \'%s\' does not exist", typeName.c_str());
	return false;
      }

      Node* node = type->createNode();
      if (!node)
      {
	Log::writeError("Failed to create node of type \'%s\'", typeName.c_str());
	return false;
      }

      if (currentNode)
	currentNode->addChild(*node);

      currentNode = node;
      return true;
    }
  }

  return true;
}

bool GraphCodecXML::onEndElement(const String& name)
{
  if (graph)
  {
    if (name == "node")
      currentNode = currentNode->getParent();
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace scene*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

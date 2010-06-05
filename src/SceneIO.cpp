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

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
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

namespace
{

const unsigned int GRAPH_XML_VERSION = 1;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

NodeInfo::NodeInfo(NodeType& initType,
                   const String& initName,
                   const Transform3& initTransform):
  type(initType),
  name(initName),
  transform(initTransform)
{
}

void NodeInfo::addParameter(const String& name, const String& value)
{
  parameters[name] = value;
}

bool NodeInfo::hasParameter(const String& name) const
{
  return parameters.find(name) != parameters.end();
}

const String& NodeInfo::getParameterValue(const String& name) const
{
  ParameterMap::const_iterator i = parameters.find(name);
  if (i == parameters.end())
    throw Exception("Invalid scene graph node parameter name");

  return i->second;
}

NodeType& NodeInfo::getType(void) const
{
  return type;
}

const String& NodeInfo::getName(void) const
{
  return name;
}

const Transform3& NodeInfo::getTransform(void) const
{
  return transform;
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

  for (NodeInfo::List::const_iterator i = roots.begin();  i != roots.end();  i++)
  {
    if (!createNode(NULL, **i))
    {
      graph = NULL;
      break;
    }
  }

  while (!roots.empty())
  {
    delete roots.back();
    roots.pop_back();
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

bool GraphCodecXML::createNode(Node* parent, const NodeInfo& info)
{
  Node* node = info.getType().createNode();
  if (!node)
  {
    Log::writeError("Failed to create node of type \'%s\'",
                    info.getType().getName().c_str());
    return false;
  }

  node->setName(info.getName());
  node->getLocalTransform() = info.getTransform();

  if (parent)
    parent->addChild(*node);
  else
    graph->addRootNode(*node);

  const NodeInfo::List& children = info.getChildren();

  for (NodeInfo::List::const_iterator c = children.begin();  c != children.end();  c++)
  {
    if (!createNode(node, **c))
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
      NodeType* type = NodeType::findInstance(readString("type"));
      if (!type)
      {
	Log::writeError("Scene graph node type \'%s\' does not exist",
                        type->getName().c_str());
	return false;
      }

      Transform3 transform;
      transform.position = Vec3(readString("position"));
      transform.rotation = Quat(readString("rotation"));

      NodeInfo* node = new NodeInfo(*type, readString("name"), transform);

      if (currentNode)
        currentNode->addChild(*node);
      else
        roots.push_back(node);

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

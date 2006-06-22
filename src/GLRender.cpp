///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Log.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Point.h>
#include <moira/Matrix.h>
#include <moira/Stream.h>
#include <moira/Image.h>
#include <moira/XML.h>
#include <moira/Mesh.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLShader.h>
#include <wendy/GLLight.h>
#include <wendy/GLVertex.h>
#include <wendy/GLIndexBuffer.h>
#include <wendy/GLVertexBuffer.h>
#include <wendy/GLSprite.h>
#include <wendy/GLRender.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

RenderOperation::RenderOperation(void):
  vertexBuffer(NULL),
  indexBuffer(NULL),
  shader(NULL)
{
}

bool RenderOperation::operator < (const RenderOperation& other) const
{
  return (*shader) < (*other.shader);
}

///////////////////////////////////////////////////////////////////////

RenderQueue::RenderQueue(void):
  sorted(true)
{
}

void RenderQueue::addLight(Light& light)
{
  lights.push_back(&light);
}

void RenderQueue::addOperation(RenderOperation& operation)
{
  operations.push_back(operation);
  sorted = false;
}

void RenderQueue::removeOperations(void)
{
  operations.clear();
}

void RenderQueue::renderOperations(void)
{
  sortOperations();

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(true);

  for (OperationList::const_iterator i = operations.begin();  i != operations.end();  i++)
  {
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf((*i).transform);
    glPopAttrib();

    (*i).vertexBuffer->apply();
    if ((*i).indexBuffer)
      (*i).indexBuffer->apply();

    for (unsigned int pass = 0;  pass < (*i).shader->getPassCount();  pass++)
    {
      (*i).shader->applyPass(pass);

      if ((*i).indexBuffer)
        (*i).indexBuffer->render((*i).renderMode);
      else
        (*i).vertexBuffer->render((*i).renderMode);
    }

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
  }

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(false);
}

const RenderQueue::LightList& RenderQueue::getLights(void) const
{
  return lights;
}

const RenderQueue::OperationList& RenderQueue::getOperations(void) const
{
  return operations;
}

void RenderQueue::sortOperations(void)
{
  if (!sorted)
  {
    std::sort(operations.begin(), operations.end());
    sorted = true;
  }
}

///////////////////////////////////////////////////////////////////////

RenderMesh::~RenderMesh(void)
{
  while (indexBuffers.size())
  {
    delete indexBuffers.back();
    indexBuffers.pop_back();
  }
}

void RenderMesh::enqueue(RenderQueue& queue, const Matrix4& transform) const
{
  for (GeometryList::const_iterator i = geometries.begin();  i != geometries.end();  i++)
  {
    Shader* shader = Shader::findInstance((*i).shaderName);
    if (!shader)
    {
      Log::writeWarning("Shader %s not found", (*i).shaderName.c_str());
      return;
    }

    RenderOperation operation;
    operation.vertexBuffer = vertexBuffer;
    operation.indexBuffer = (*i).indexBuffer;
    operation.renderMode = (*i).renderMode;
    operation.transform = transform;
    operation.shader = shader;
    queue.addOperation(operation);
  }
}

void RenderMesh::render(void) const
{
  vertexBuffer->apply();

  for (GeometryList::const_iterator i = geometries.begin();  i != geometries.end();  i++)
  {
    const Geometry& geometry = *i;

    Shader* shader = Shader::findInstance(geometry.shaderName);
    if (!shader)
    {
      Log::writeWarning("Shader %s not found", (*i).shaderName.c_str());
      return;
    }

    for (unsigned int pass = 0;  pass < shader->getPassCount();  pass++)
    {
      shader->applyPass(pass);

      geometry.indexBuffer->apply();
      geometry.indexBuffer->render(geometry.renderMode);
    }
  }
}

RenderMesh::GeometryList& RenderMesh::getGeometries(void)
{
  return geometries;
}

VertexBuffer* RenderMesh::getVertexBuffer(void)
{
  return vertexBuffer;
}

RenderMesh* RenderMesh::createInstance(const Path& path, const std::string& name)
{
  MeshReader reader;
  Ptr<Mesh> mesh = reader.read(path);
  if (!mesh)
    return NULL;

  return createInstance(*mesh, name);
}

RenderMesh* RenderMesh::createInstance(const Mesh& mesh, const std::string& name)
{
  Ptr<RenderMesh> renderMesh = new RenderMesh(name.empty() ? mesh.getName() : name);
  if (!renderMesh->init(mesh))
    return NULL;

  return renderMesh.detachObject();
}

RenderMesh::RenderMesh(const std::string& name):
  Managed<RenderMesh>(name)
{
}

bool RenderMesh::init(const Mesh& mesh)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create render mesh without OpenGL context");
    return false;
  }

  VertexFormat format;

  if (!format.addComponents("3fv3fn"))
    return false;

  std::string vertexBufferName;
  vertexBufferName.append("mesh:");
  vertexBufferName.append(getName());

  vertexBuffer = VertexBuffer::createInstance(vertexBufferName, (unsigned int) mesh.vertices.size(), format);
  if (!vertexBuffer)
    return false;

  MeshVertex* vertices = reinterpret_cast<MeshVertex*>(vertexBuffer->lock());
  if (!vertices)
    return false;

  for (Mesh::VertexList::const_iterator i = mesh.vertices.begin();  i != mesh.vertices.end();  i++)
    *vertices++ = *i;

  vertexBuffer->unlock();

  for (Mesh::GeometryList::const_iterator i = mesh.geometries.begin();  i != mesh.geometries.end();  i++)
  {
    geometries.push_back(Geometry());
    Geometry& geometry = geometries.back();

    geometry.shaderName = (*i).shaderName;
    geometry.renderMode = GL_TRIANGLES;

    std::string indexBufferName;
    indexBufferName.append("mesh:");
    indexBufferName.append(getName());
    indexBufferName.append("/");
    indexBufferName.append(geometry.shaderName);

    geometry.indexBuffer = IndexBuffer::createInstance(indexBufferName, (unsigned int) (*i).triangles.size() * 3, IndexBuffer::UINT);
    if (!geometry.indexBuffer)
      return false;

    indexBuffers.push_back(geometry.indexBuffer);

    unsigned int* indices = reinterpret_cast<unsigned int*>(geometry.indexBuffer->lock());
    if (!indices)
      return false;

    for (MeshGeometry::TriangleList::const_iterator j = (*i).triangles.begin();  j != (*i).triangles.end();  j++)
    {
      *indices++ = (*j).indices[0];
      *indices++ = (*j).indices[1];
      *indices++ = (*j).indices[2];
    }

    geometry.indexBuffer->unlock();
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

RenderSprite::~RenderSprite(void)
{
}

void RenderSprite::enqueue(RenderQueue& queue, const Matrix4& transform) const
{
  Shader* shader = Shader::findInstance(shaderName);
  if (!shader)
  {
    Log::writeWarning("Shader %s not found", shaderName.c_str());
    return;
  }

  RenderOperation operation;
  operation.vertexBuffer = vertexBuffer;
  operation.renderMode = GL_QUADS;
  operation.transform = transform;
  operation.shader = shader;
  queue.addOperation(operation);
}

void RenderSprite::render(void) const
{
  Shader* shader = Shader::findInstance(shaderName);
  if (!shader)
  {
    Log::writeWarning("Shader %s not found", shaderName.c_str());
    return;
  }

  vertexBuffer->apply();

  for (unsigned int pass = 0;  pass < shader->getPassCount();  pass++)
  {
    shader->applyPass(pass);
    vertexBuffer->render(GL_QUADS);
  }
}

VertexBuffer* RenderSprite::getVertexBuffer(void)
{
  return vertexBuffer;
}

const std::string& RenderSprite::getShaderName(void) const
{
  return shaderName;
}

void RenderSprite::setShaderName(const std::string& newShaderName)
{
  shaderName = newShaderName;
}

const Vector2& RenderSprite::getSpriteSize(void) const
{
  return spriteSize;
}

void RenderSprite::setSpriteSize(const Vector2& newSize)
{
  Vertex2ft3fv* vertices = (Vertex2ft3fv*) vertexBuffer->lock();
  if (!vertices)
    return;

  Sprite3 sprite;
  sprite.size = newSize;
  sprite.realizeVertices(vertices);

  vertexBuffer->unlock();

  spriteSize = newSize;
}

RenderSprite* RenderSprite::createInstance(const std::string& name)
{
  Ptr<RenderSprite> sprite = new RenderSprite(name);
  if (!sprite->init())
    return NULL;

  return sprite.detachObject();
}

RenderSprite::RenderSprite(const std::string& name):
  Managed<RenderSprite>(name)
{
}

bool RenderSprite::init(void)
{
  // TODO: Make a vertex buffer pool (credits to ryg).

  vertexBuffer = VertexBuffer::createInstance("", 4, Vertex2ft3fv::format, VertexBuffer::DYNAMIC);
  if (!vertexBuffer)
    return false;

  setSpriteSize(Vector2(1.f, 1.f));
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

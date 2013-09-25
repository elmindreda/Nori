///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Timer.hpp>
#include <wendy/Profile.hpp>

#include <wendy/GLTexture.hpp>
#include <wendy/GLBuffer.hpp>
#include <wendy/GLProgram.hpp>
#include <wendy/GLContext.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.hpp>

#define GLFW_NO_GLU
#include <GLFW/glfw3.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

const char* getMessageSourceName(GLenum source)
{
  switch (source)
  {
    case GL_DEBUG_SOURCE_API_ARB:
      return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
      return "window system";
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
      return "shader compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
      return "third party";
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
      return "application";
    case GL_DEBUG_SOURCE_OTHER_ARB:
      return "other";
  }

  return "UNKNOWN";
}

const char* getMessageTypeName(GLenum type)
{
  switch (type)
  {
    case GL_DEBUG_TYPE_ERROR_ARB:
      return "error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
      return "deprecated behavior";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
      return "undefined behavior";
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
      return "portability issue";
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
      return "performance issue";
    case GL_DEBUG_TYPE_OTHER_ARB:
      return "issue";
  }

  return "UNKNOWN";
}

const char* getMessageSeverityName(GLenum severity)
{
  switch (severity)
  {
    case GL_DEBUG_SEVERITY_HIGH_ARB:
      return "high";
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
      return "medium";
    case GL_DEBUG_SEVERITY_LOW_ARB:
      return "low";
  }

  return "UNKNOWN";
}

void errorCallback(int error, const char* message)
{
  logError("GLFW reported error: %s", message);
}

void APIENTRY debugCallback(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar* message,
                            GLvoid* userParam)
{
  if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
  {
    logError("OpenGL reported %s severity %s %s %u: %s",
             getMessageSeverityName(severity),
             getMessageSourceName(source),
             getMessageTypeName(type),
             id,
             message);
  }
  else
  {
    logWarning("OpenGL reported %s severity %s %s %u: %s",
               getMessageSeverityName(severity),
               getMessageSourceName(source),
               getMessageTypeName(type),
               id,
               message);
  }
}

const char* getFramebufferStatusMessage(GLenum status)
{
  switch (status)
  {
    case GL_FRAMEBUFFER_COMPLETE:
      return "Framebuffer is complete";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      return "Incomplete framebuffer attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      return "Incomplete or missing framebuffer attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      return "Incomplete framebuffer draw buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      return "Incomplete framebuffer read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED:
      return "Framebuffer configuration is unsupported";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      return "Framebuffer layer targets incomplete";
  }

  logError("Unknown OpenGL framebuffer status %u", status);
  return "Unknown framebuffer status";
}

GLenum convertToGL(PrimitiveType type)
{
  switch (type)
  {
    case POINT_LIST:
      return GL_POINTS;
    case LINE_LIST:
      return GL_LINES;
    case LINE_STRIP:
      return GL_LINE_STRIP;
    case LINE_LOOP:
      return GL_LINE_LOOP;
    case TRIANGLE_LIST:
      return GL_TRIANGLES;
    case TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
  }

  panic("Invalid primitive type %u", type);
}

GLenum convertToGL(CullMode mode)
{
  switch (mode)
  {
    case CULL_NONE:
      break;
    case CULL_FRONT:
      return GL_FRONT;
    case CULL_BACK:
      return GL_BACK;
    case CULL_BOTH:
      return GL_FRONT_AND_BACK;
  }

  panic("Invalid cull mode %u", mode);
}

CullMode invertCullMode(CullMode mode)
{
  switch (mode)
  {
    case CULL_NONE:
      return CULL_BOTH;
    case CULL_FRONT:
      return CULL_BACK;
    case CULL_BACK:
      return CULL_FRONT;
    case CULL_BOTH:
      return CULL_NONE;
  }

  panic("Invalid cull mode %u", mode);
}

GLenum convertToGL(BlendFactor factor)
{
  switch (factor)
  {
    case BLEND_ZERO:
      return GL_ZERO;
    case BLEND_ONE:
      return GL_ONE;
    case BLEND_SRC_COLOR:
      return GL_SRC_COLOR;
    case BLEND_DST_COLOR:
      return GL_DST_COLOR;
    case BLEND_SRC_ALPHA:
      return GL_SRC_ALPHA;
    case BLEND_DST_ALPHA:
      return GL_DST_ALPHA;
    case BLEND_ONE_MINUS_SRC_COLOR:
      return GL_ONE_MINUS_SRC_COLOR;
    case BLEND_ONE_MINUS_DST_COLOR:
      return GL_ONE_MINUS_DST_COLOR;
    case BLEND_ONE_MINUS_SRC_ALPHA:
      return GL_ONE_MINUS_SRC_ALPHA;
    case BLEND_ONE_MINUS_DST_ALPHA:
      return GL_ONE_MINUS_DST_ALPHA;
  }

  panic("Invalid blend factor %u", factor);
}

GLenum convertToGL(Function function)
{
  switch (function)
  {
    case ALLOW_NEVER:
      return GL_NEVER;
    case ALLOW_ALWAYS:
      return GL_ALWAYS;
    case ALLOW_EQUAL:
      return GL_EQUAL;
    case ALLOW_NOT_EQUAL:
      return GL_NOTEQUAL;
    case ALLOW_LESSER:
      return GL_LESS;
    case ALLOW_LESSER_EQUAL:
      return GL_LEQUAL;
    case ALLOW_GREATER:
      return GL_GREATER;
    case ALLOW_GREATER_EQUAL:
      return GL_GEQUAL;
  }

  panic("Invalid comparison function %u", function);
}

GLenum convertToGL(Operation operation)
{
  switch (operation)
  {
    case OP_KEEP:
      return GL_KEEP;
    case OP_ZERO:
      return GL_ZERO;
    case OP_REPLACE:
      return GL_REPLACE;
    case OP_INCREASE:
      return GL_INCR;
    case OP_DECREASE:
      return GL_DECR;
    case OP_INVERT:
      return GL_INVERT;
    case OP_INCREASE_WRAP:
      return GL_INCR_WRAP;
    case OP_DECREASE_WRAP:
      return GL_DECR_WRAP;
  }

  panic("Invalid stencil operation %u", operation);
}

bool isCompatible(const Attribute& attribute, const VertexComponent& component)
{
  switch (attribute.type())
  {
    case ATTRIBUTE_FLOAT:
      return component.elementCount() == 1;
    case ATTRIBUTE_VEC2:
      return component.elementCount() == 2;
    case ATTRIBUTE_VEC3:
      return component.elementCount() == 3;
    case ATTRIBUTE_VEC4:
      return component.elementCount() == 4;
  }

  return false;
}

void setBooleanState(uint state, bool value)
{
  if (value)
    glEnable(state);
  else
    glDisable(state);
}

} /*namespace (and Gandalf)*/

///////////////////////////////////////////////////////////////////////

ContextConfig::ContextConfig(uint initColorBits,
                             uint initDepthBits,
                             uint initStencilBits,
                             uint initSamples,
                             bool initDebug):
  colorBits(initColorBits),
  depthBits(initDepthBits),
  stencilBits(initStencilBits),
  samples(initSamples),
  debug(initDebug)
{
}

///////////////////////////////////////////////////////////////////////

RenderState::RenderState():
  depthTesting(true),
  depthWriting(true),
  colorWriting(true),
  stencilTesting(false),
  wireframe(false),
  lineSmoothing(false),
  multisampling(true),
  lineWidth(1.f),
  cullMode(CULL_BACK),
  srcFactor(BLEND_ONE),
  dstFactor(BLEND_ZERO),
  depthFunction(ALLOW_LESSER),
  stencilFunction(ALLOW_ALWAYS),
  stencilRef(0),
  stencilMask(~0u),
  stencilFailOp(OP_KEEP),
  depthFailOp(OP_KEEP),
  depthPassOp(OP_KEEP)
{
}

///////////////////////////////////////////////////////////////////////

Limits::Limits(Context& context)
{
  maxColorAttachments = getInteger(GL_MAX_COLOR_ATTACHMENTS);
  maxDrawBuffers = getInteger(GL_MAX_DRAW_BUFFERS);
  maxVertexTextureImageUnits = getInteger(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
  maxFragmentTextureImageUnits = getInteger(GL_MAX_TEXTURE_IMAGE_UNITS);
  maxCombinedTextureImageUnits = getInteger(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
  maxTextureSize = getInteger(GL_MAX_TEXTURE_SIZE);
  maxTexture3DSize = getInteger(GL_MAX_3D_TEXTURE_SIZE);
  maxTextureCubeSize = getInteger(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
  maxTextureRectangleSize = getInteger(GL_MAX_RECTANGLE_TEXTURE_SIZE);
  maxTextureCoords = getInteger(GL_MAX_TEXTURE_COORDS);
  maxVertexAttributes = getInteger(GL_MAX_VERTEX_ATTRIBS);

  if (GLEW_EXT_texture_filter_anisotropic)
    maxTextureAnisotropy = getFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
  else
    maxTextureAnisotropy = 1.f;
}

///////////////////////////////////////////////////////////////////////

Stats::Stats():
  m_frameCount(0),
  m_frameRate(0.f),
  m_textureCount(0),
  m_vertexBufferCount(0),
  m_indexBufferCount(0),
  m_programCount(0),
  m_textureSize(0),
  m_vertexBufferSize(0),
  m_indexBufferSize(0)
{
  m_frames.push_back(Frame());
  m_timer.start();
}

void Stats::addFrame()
{
  m_frameCount++;
  m_frameRate = 0.f;

  if (!m_frames.empty())
  {
    // Add the previous frame duration
    m_frames.front().duration = m_timer.deltaTime();

    // Calculate frame rate
    for (auto& f : m_frames)
      m_frameRate += float(f.duration);

    m_frameRate = float(m_frames.size()) / m_frameRate;
  }

  // Add new empty frame for recording the stats
  m_frames.push_front(Frame());
  if (m_frames.size() > 60)
    m_frames.pop_back();
}

void Stats::addStateChange()
{
  Frame& frame = m_frames.front();
  frame.stateChangeCount++;
}

void Stats::addPrimitives(PrimitiveType type, uint vertexCount)
{
  Frame& frame = m_frames.front();
  frame.vertexCount += vertexCount;
  frame.operationCount++;

  switch (type)
  {
    case POINT_LIST:
      frame.pointCount += vertexCount;
      break;
    case LINE_LIST:
      frame.lineCount += vertexCount / 2;
      break;
    case LINE_STRIP:
      frame.lineCount += vertexCount - 1;
      break;
    case TRIANGLE_LIST:
      frame.triangleCount += vertexCount / 3;
      break;
    case TRIANGLE_STRIP:
      frame.triangleCount += vertexCount - 2;
      break;
    case TRIANGLE_FAN:
      frame.triangleCount += vertexCount - 2;
      break;
    default:
      panic("Invalid primitive type %u", type);
  }
}

void Stats::addTexture(size_t size)
{
  m_textureCount++;
  m_textureSize += size;
}

void Stats::removeTexture(size_t size)
{
  m_textureCount--;
  m_textureSize -= size;
}

void Stats::addVertexBuffer(size_t size)
{
  m_vertexBufferCount++;
  m_vertexBufferSize += size;
}

void Stats::removeVertexBuffer(size_t size)
{
  m_vertexBufferCount--;
  m_vertexBufferSize -= size;
}

void Stats::addIndexBuffer(size_t size)
{
  m_indexBufferCount++;
  m_indexBufferSize += size;
}

void Stats::removeIndexBuffer(size_t size)
{
  m_indexBufferCount--;
  m_indexBufferSize -= size;
}

void Stats::addProgram()
{
  m_programCount++;
}

void Stats::removeProgram()
{
  m_programCount--;
}

///////////////////////////////////////////////////////////////////////

Stats::Frame::Frame():
  stateChangeCount(0),
  operationCount(0),
  vertexCount(0),
  pointCount(0),
  lineCount(0),
  triangleCount(0),
  duration(0.0)
{
}

///////////////////////////////////////////////////////////////////////

class Context::SharedSampler
{
public:
  SharedSampler(const char* name, SamplerType type, int ID):
    name(name),
    type(type),
    ID(ID)
  {
  }
  String name;
  SamplerType type;
  int ID;
};

///////////////////////////////////////////////////////////////////////

class Context::SharedUniform
{
public:
  SharedUniform(const char* name, UniformType type, int ID):
    name(name),
    type(type),
    ID(ID)
  {
  }
  String name;
  UniformType type;
  int ID;
};

///////////////////////////////////////////////////////////////////////

Context::~Context()
{
  if (m_defaultFramebuffer)
    setDefaultFramebufferCurrent();

  setCurrentVertexBuffer(nullptr);
  setCurrentIndexBuffer(nullptr);
  setCurrentProgram(nullptr);

  for (size_t i = 0;  i < m_textureUnits.size();  i++)
  {
    setActiveTextureUnit(i);
    setCurrentTexture(nullptr);
  }

  if (m_handle)
  {
    glfwDestroyWindow(m_handle);
    m_handle = nullptr;
  }
}

void Context::clearColorBuffer(const vec4& color)
{
  const RenderState previousState = m_currentState;

  RenderState clearState = m_currentState;
  clearState.colorWriting = true;
  applyState(clearState);

  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  applyState(previousState);
}

void Context::clearDepthBuffer(float depth)
{
  const RenderState previousState = m_currentState;

  RenderState clearState = m_currentState;
  clearState.depthWriting = true;
  applyState(clearState);

  glClearDepth(depth);
  glClear(GL_DEPTH_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  applyState(previousState);
}

void Context::clearStencilBuffer(uint value)
{
  const RenderState previousState = m_currentState;

  RenderState clearState = m_currentState;
  clearState.stencilMask = ~0u;
  applyState(clearState);

  glClearStencil(value);
  glClear(GL_STENCIL_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  applyState(previousState);
}

void Context::clearBuffers(const vec4& color, float depth, uint value)
{
  const RenderState previousState = m_currentState;

  RenderState clearState = m_currentState;
  clearState.colorWriting = true;
  clearState.depthWriting = true;
  clearState.stencilMask = ~0u;
  applyState(clearState);

  glClearColor(color.r, color.g, color.b, color.a);
  glClearDepth(depth);
  glClearStencil(value);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  applyState(previousState);
}

void Context::render(const PrimitiveRange& range)
{
  if (range.isEmpty())
  {
    logWarning("Rendering empty primitive range with shader program %s",
               m_currentProgram->name().c_str());
    return;
  }

  setCurrentVertexBuffer(range.vertexBuffer());
  setCurrentIndexBuffer(range.indexBuffer());

  render(range.type(), range.start(), range.count(), range.base());
}

void Context::render(PrimitiveType type, uint start, uint count, uint base)
{
  ProfileNodeCall call("GL::Context::render");

  if (!m_currentProgram)
  {
    logError("Cannot render without a current shader program");
    return;
  }

  if (!m_currentVertexBuffer)
  {
    logError("Cannot render without a current vertex buffer");
    return;
  }

  if (m_dirtyBinding)
  {
    const VertexFormat& format = m_currentVertexBuffer->format();

    if (m_currentProgram->attributeCount() > format.components().size())
    {
      logError("Shader program %s has more attributes than vertex format has components",
               m_currentProgram->name().c_str());
      return;
    }

    for (size_t i = 0;  i < m_currentProgram->attributeCount();  i++)
    {
      Attribute& attribute = m_currentProgram->attribute(i);

      const VertexComponent* component = format.findComponent(attribute.name().c_str());
      if (!component)
      {
        logError("Attribute %s of program %s has no corresponding vertex format component",
                 attribute.name().c_str(),
                 m_currentProgram->name().c_str());
        return;
      }

      if (!isCompatible(attribute, *component))
      {
        logError("Attribute %s of shader program %s has incompatible type",
                 attribute.name().c_str(),
                 m_currentProgram->name().c_str());
        return;
      }

      attribute.bind(format.size(), component->offset());
    }

    m_dirtyBinding = false;
  }

#if WENDY_DEBUG
  if (!m_currentProgram->isValid())
    return;
#endif

  if (m_currentIndexBuffer)
  {
    const size_t size = IndexBuffer::typeSize(m_currentIndexBuffer->type());

    glDrawElementsBaseVertex(convertToGL(type),
                             count,
                             convertToGL(m_currentIndexBuffer->type()),
                             (GLvoid*) (size * start),
                             base);
  }
  else
    glDrawArrays(convertToGL(type), start, count);

  if (m_stats)
    m_stats->addPrimitives(type, count);
}

void Context::createSharedSampler(const char* name, SamplerType type, int ID)
{
  assert(ID != INVALID_SHARED_STATE_ID);

  if (sharedSamplerID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  m_declaration += format("uniform %s %s;\n", Sampler::typeName(type), name);

  m_samplers.push_back(SharedSampler(name, type, ID));
}

void Context::createSharedUniform(const char* name, UniformType type, int ID)
{
  assert(ID != INVALID_SHARED_STATE_ID);

  if (sharedUniformID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  m_declaration += format("uniform %s %s;\n", Uniform::typeName(type), name);

  m_uniforms.push_back(SharedUniform(name, type, ID));
}

int Context::sharedSamplerID(const char* name, SamplerType type) const
{
  for (auto& s : m_samplers)
  {
    if (s.name == name && s.type == type)
      return s.ID;
  }

  return INVALID_SHARED_STATE_ID;
}

int Context::sharedUniformID(const char* name, UniformType type) const
{
  for (auto& u : m_uniforms)
  {
    if (u.name == name && u.type == type)
      return u.ID;
  }

  return INVALID_SHARED_STATE_ID;
}

SharedProgramState* Context::currentSharedProgramState() const
{
  return m_currentSharedState;
}

void Context::setCurrentSharedProgramState(SharedProgramState* newState)
{
  m_currentSharedState = newState;
}

const char* Context::sharedProgramStateDeclaration() const
{
  return m_declaration.c_str();
}

int Context::swapInterval() const
{
  return m_swapInterval;
}

void Context::setSwapInterval(int newInterval)
{
  glfwSwapInterval(newInterval);
  m_swapInterval = newInterval;
}

const Recti& Context::scissorArea() const
{
  return m_scissorArea;
}

const Recti& Context::viewportArea() const
{
  return m_viewportArea;
}

void Context::setScissorArea(const Recti& newArea)
{
  m_scissorArea = newArea;

  const uint width = m_currentFramebuffer->width();
  const uint height = m_currentFramebuffer->height();

  if (m_scissorArea == Recti(0, 0, width, height))
    glDisable(GL_SCISSOR_TEST);
  else
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor(m_scissorArea.position.x,
              m_scissorArea.position.y,
              m_scissorArea.size.x,
              m_scissorArea.size.y);
  }
}

void Context::setViewportArea(const Recti& newArea)
{
  m_viewportArea = newArea;

  glViewport(m_viewportArea.position.x,
             m_viewportArea.position.y,
             m_viewportArea.size.x,
             m_viewportArea.size.y);
}

Framebuffer& Context::currentFramebuffer() const
{
  return *m_currentFramebuffer;
}

DefaultFramebuffer& Context::defaultFramebuffer() const
{
  return *m_defaultFramebuffer;
}

void Context::setDefaultFramebufferCurrent()
{
  setCurrentFramebuffer(*m_defaultFramebuffer);
}

bool Context::setCurrentFramebuffer(Framebuffer& newFramebuffer)
{
  m_currentFramebuffer = &newFramebuffer;
  m_currentFramebuffer->apply();

#if WENDY_DEBUG
  if (m_currentFramebuffer != m_defaultFramebuffer)
  {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == 0)
      checkGL("Framebuffer status check failed");
    else if (status != GL_FRAMEBUFFER_COMPLETE)
      logError("Image framebuffer is incomplete: %s", getFramebufferStatusMessage(status));
  }
#endif

  return true;
}

Program* Context::currentProgram() const
{
  return m_currentProgram;
}

void Context::setCurrentProgram(Program* newProgram)
{
  if (newProgram != m_currentProgram)
  {
    if (m_currentProgram)
      m_currentProgram->unbind();

    m_currentProgram = newProgram;
    m_dirtyBinding = true;

    if (m_currentProgram)
      m_currentProgram->bind();
    else
      glUseProgram(0);
  }
}

VertexBuffer* Context::currentVertexBuffer() const
{
  return m_currentVertexBuffer;
}

void Context::setCurrentVertexBuffer(VertexBuffer* newVertexBuffer)
{
  if (newVertexBuffer != m_currentVertexBuffer)
  {
    m_currentVertexBuffer = newVertexBuffer;
    m_dirtyBinding = true;

    if (m_currentVertexBuffer)
      glBindBuffer(GL_ARRAY_BUFFER, m_currentVertexBuffer->m_bufferID);
    else
      glBindBuffer(GL_ARRAY_BUFFER, 0);

#if WENDY_DEBUG
    if (!checkGL("Failed to make index buffer current"))
      return;
#endif
  }
}

IndexBuffer* Context::currentIndexBuffer() const
{
  return m_currentIndexBuffer;
}

void Context::setCurrentIndexBuffer(IndexBuffer* newIndexBuffer)
{
  if (newIndexBuffer != m_currentIndexBuffer)
  {
    m_currentIndexBuffer = newIndexBuffer;
    m_dirtyBinding = true;

    if (m_currentIndexBuffer)
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_currentIndexBuffer->m_bufferID);
    else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#if WENDY_DEBUG
    if (!checkGL("Failed to apply index buffer"))
      return;
#endif
  }
}

Texture* Context::currentTexture() const
{
  return m_textureUnits[m_activeTextureUnit];
}

void Context::setCurrentTexture(Texture* newTexture)
{
  if (m_textureUnits[m_activeTextureUnit] != newTexture)
  {
    Texture* oldTexture = m_textureUnits[m_activeTextureUnit];

    if (oldTexture)
    {
      if (!newTexture || oldTexture->type() != newTexture->type())
      {
        glBindTexture(convertToGL(oldTexture->type()), 0);

#if WENDY_DEBUG
        if (!checkGL("Failed to unbind texture %s",
                     oldTexture->name().c_str()))
        {
          return;
        }
#endif
      }
    }

    if (newTexture)
    {
      glBindTexture(convertToGL(newTexture->type()), newTexture->m_textureID);

#if WENDY_DEBUG
      if (!checkGL("Failed to bind texture %s",
                   newTexture->name().c_str()))
      {
        return;
      }
#endif
    }

    m_textureUnits[m_activeTextureUnit] = newTexture;
  }
}

uint Context::textureUnitCount() const
{
  return (uint) m_textureUnits.size();
}

uint Context::activeTextureUnit() const
{
  return m_activeTextureUnit;
}

void Context::setActiveTextureUnit(uint unit)
{
  if (m_activeTextureUnit != unit)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    m_activeTextureUnit = unit;

#if WENDY_DEBUG
    if (!checkGL("Failed to activate texture unit %u", unit))
      return;
#endif
  }
}

bool Context::isCullingInverted()
{
  return m_cullingInverted;
}

void Context::setCullingInversion(bool newState)
{
  m_cullingInverted = newState;
}

const RenderState& Context::currentRenderState() const
{
  return m_currentState;
}

void Context::setCurrentRenderState(const RenderState& newState)
{
  applyState(newState);
}

Stats* Context::stats() const
{
  return m_stats;
}

void Context::setStats(Stats* newStats)
{
  m_stats = newStats;
}

ResourceCache& Context::cache() const
{
  return m_cache;
}

Window& Context::window()
{
  return m_window;
}

const Limits& Context::limits() const
{
  return *m_limits;
}

Context* Context::create(ResourceCache& cache,
                         const WindowConfig& wc,
                         const ContextConfig& cc)
{
  Ptr<Context> context(new Context(cache));
  if (!context->init(wc, cc))
    return nullptr;

  return context.detachObject();
}

Context::Context(ResourceCache& cache):
  m_cache(cache),
  m_handle(nullptr),
  m_dirtyBinding(true),
  m_dirtyState(true),
  m_cullingInverted(false),
  m_activeTextureUnit(0),
  m_stats(nullptr)
{
}

Context::Context(const Context& source):
  m_cache(source.m_cache)
{
  panic("OpenGL contexts may not be copied");
}

Context& Context::operator = (const Context& source)
{
  panic("OpenGL contexts may not be assigned");
}

bool Context::init(const WindowConfig& wc, const ContextConfig& cc)
{
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit())
  {
    logError("Failed to initialize GLFW");
    return false;
  }

  log("GLFW version %s initialized", glfwGetVersionString());

  // Create context and window
  {
    const uint colorBits = min(cc.colorBits, 24u);

    glfwWindowHint(GLFW_RED_BITS, colorBits / 3);
    glfwWindowHint(GLFW_GREEN_BITS, colorBits / 3);
    glfwWindowHint(GLFW_BLUE_BITS, colorBits / 3);
    glfwWindowHint(GLFW_DEPTH_BITS, cc.depthBits);
    glfwWindowHint(GLFW_STENCIL_BITS, cc.stencilBits);
    glfwWindowHint(GLFW_SAMPLES, cc.samples);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, cc.debug);

    glfwWindowHint(GLFW_RESIZABLE, wc.resizable);

    GLFWmonitor* monitor = nullptr;

    if (wc.mode == FULLSCREEN)
      monitor = glfwGetPrimaryMonitor();

    m_handle = glfwCreateWindow(wc.width, wc.height, wc.title.c_str(), monitor, nullptr);
    if (!m_handle)
    {
      logError("Failed to create GLFW window");
      return false;
    }

    glfwSetWindowUserPointer(m_handle, this);
    glfwMakeContextCurrent(m_handle);

    log("OpenGL context version %i.%i created",
        glfwGetWindowAttrib(m_handle, GLFW_CONTEXT_VERSION_MAJOR),
        glfwGetWindowAttrib(m_handle, GLFW_CONTEXT_VERSION_MINOR));

    log("OpenGL context GLSL version is %s",
        (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

    log("OpenGL context renderer is %s by %s",
        (const char*) glGetString(GL_RENDERER),
        (const char*) glGetString(GL_VENDOR));

    m_window.init(m_handle);
    m_window.frameSignal().connect(*this, &Context::onFrame);
  }

  // Initialize GLEW and check extensions
  {
    if (glewInit() != GLEW_OK)
    {
      logError("Failed to initialize GLEW");
      return false;
    }

    if (cc.debug && GLEW_ARB_debug_output)
    {
      glDebugMessageCallbackARB(debugCallback, nullptr);
      glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
  }

  // Retrieve context limits and set up dependent caches
  {
    m_limits = new Limits(*this);

    const uint unitCount = max(m_limits->maxCombinedTextureImageUnits,
                               m_limits->maxTextureCoords);

    m_textureUnits.resize(unitCount);
  }

  // Create and apply default framebuffer
  {
    m_defaultFramebuffer = new DefaultFramebuffer(*this);

    // Read back actual (as opposed to desired) properties
    m_defaultFramebuffer->m_colorBits = getInteger(GL_RED_BITS) +
                                        getInteger(GL_GREEN_BITS) +
                                        getInteger(GL_BLUE_BITS);
    m_defaultFramebuffer->m_depthBits = getInteger(GL_DEPTH_BITS);
    m_defaultFramebuffer->m_stencilBits = getInteger(GL_STENCIL_BITS);
    m_defaultFramebuffer->m_samples = getInteger(GL_SAMPLES);

    setDefaultFramebufferCurrent();
  }

  // Force a known GL state
  {
    int width, height;
    glfwGetWindowSize(m_handle, &width, &height);

    setViewportArea(Recti(0, 0, width, height));
    setScissorArea(Recti(0, 0, width, height));

    setSwapInterval(1);
    forceState(m_currentState);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  return true;
}

void Context::applyState(const RenderState& newState)
{
  if (m_stats)
    m_stats->addStateChange();

  if (m_dirtyState)
  {
    forceState(newState);
    return;
  }

  CullMode cullMode = newState.cullMode;
  if (m_cullingInverted)
    cullMode = invertCullMode(cullMode);

  if (cullMode != m_currentState.cullMode)
  {
    if ((cullMode == CULL_NONE) != (m_currentState.cullMode == CULL_NONE))
      setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);

    if (cullMode != CULL_NONE)
      glCullFace(convertToGL(cullMode));

    m_currentState.cullMode = cullMode;
  }

  if (newState.srcFactor != m_currentState.srcFactor ||
      newState.dstFactor != m_currentState.dstFactor)
  {
    setBooleanState(GL_BLEND, newState.srcFactor != BLEND_ONE ||
                              newState.dstFactor != BLEND_ZERO);

    if (newState.srcFactor != BLEND_ONE || newState.dstFactor != BLEND_ZERO)
    {
      glBlendFunc(convertToGL(newState.srcFactor),
                  convertToGL(newState.dstFactor));
    }

    m_currentState.srcFactor = newState.srcFactor;
    m_currentState.dstFactor = newState.dstFactor;
  }

  if (newState.depthTesting || newState.depthWriting)
  {
    // Set depth buffer writing.
    if (newState.depthWriting != m_currentState.depthWriting)
      glDepthMask(newState.depthWriting ? GL_TRUE : GL_FALSE);

    if (newState.depthTesting)
    {
      // Set depth buffer function.
      if (newState.depthFunction != m_currentState.depthFunction)
      {
        glDepthFunc(convertToGL(newState.depthFunction));
        m_currentState.depthFunction = newState.depthFunction;
      }
    }
    else if (newState.depthWriting)
    {
      // NOTE: Special case; depth buffer filling.
      //       Set specific depth buffer function.
      const Function depthFunction = ALLOW_ALWAYS;

      if (m_currentState.depthFunction != depthFunction)
      {
        glDepthFunc(convertToGL(depthFunction));
        m_currentState.depthFunction = depthFunction;
      }
    }

    if (!(m_currentState.depthTesting || m_currentState.depthWriting))
      glEnable(GL_DEPTH_TEST);
  }
  else
  {
    if (m_currentState.depthTesting || m_currentState.depthWriting)
      glDisable(GL_DEPTH_TEST);
  }

  m_currentState.depthTesting = newState.depthTesting;
  m_currentState.depthWriting = newState.depthWriting;

  if (newState.colorWriting != m_currentState.colorWriting)
  {
    const GLboolean state = newState.colorWriting ? GL_TRUE : GL_FALSE;
    glColorMask(state, state, state, state);
    m_currentState.colorWriting = newState.colorWriting;
  }

  if (newState.stencilTesting != m_currentState.stencilTesting)
  {
    setBooleanState(GL_STENCIL_TEST, newState.stencilTesting);
    m_currentState.stencilTesting = newState.stencilTesting;
  }

  if (newState.stencilTesting)
  {
    if (newState.stencilFunction != m_currentState.stencilFunction ||
        newState.stencilRef != m_currentState.stencilRef ||
        newState.stencilMask != m_currentState.stencilMask)
    {
      glStencilFunc(convertToGL(newState.stencilFunction),
                    newState.stencilRef, newState.stencilMask);

      m_currentState.stencilFunction = newState.stencilFunction;
      m_currentState.stencilRef = newState.stencilRef;
      m_currentState.stencilMask = newState.stencilMask;
    }

    if (newState.stencilFailOp != m_currentState.stencilFailOp ||
        newState.depthFailOp != m_currentState.depthFailOp ||
        newState.depthPassOp != m_currentState.depthPassOp)
    {
      glStencilOp(convertToGL(newState.stencilFailOp),
                  convertToGL(newState.depthFailOp),
                  convertToGL(newState.depthPassOp));

      m_currentState.stencilFailOp = newState.stencilFailOp;
      m_currentState.depthFailOp = newState.depthFailOp;
      m_currentState.depthPassOp = newState.depthPassOp;
    }
  }

  if (newState.wireframe != m_currentState.wireframe)
  {
    const GLenum state = newState.wireframe ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, state);
    m_currentState.wireframe = newState.wireframe;
  }

  if (newState.lineSmoothing != m_currentState.lineSmoothing)
  {
    setBooleanState(GL_LINE_SMOOTH, newState.lineSmoothing);
    m_currentState.lineSmoothing = newState.lineSmoothing;
  }

  if (newState.multisampling != m_currentState.multisampling)
  {
    setBooleanState(GL_MULTISAMPLE, newState.multisampling);
    m_currentState.multisampling = newState.multisampling;
  }

  if (newState.lineWidth != m_currentState.lineWidth)
  {
    glLineWidth(newState.lineWidth);
    m_currentState.lineWidth = newState.lineWidth;
  }

#if WENDY_DEBUG
  checkGL("Error when applying render state");
#endif
}

void Context::forceState(const RenderState& newState)
{
  m_currentState = newState;

  CullMode cullMode = newState.cullMode;
  if (m_cullingInverted)
    cullMode = invertCullMode(cullMode);

  setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);
  if (cullMode != CULL_NONE)
    glCullFace(convertToGL(cullMode));

  setBooleanState(GL_BLEND, newState.srcFactor != BLEND_ONE ||
                            newState.dstFactor != BLEND_ZERO);
  glBlendFunc(convertToGL(newState.srcFactor), convertToGL(newState.dstFactor));

  glDepthMask(newState.depthWriting ? GL_TRUE : GL_FALSE);
  setBooleanState(GL_DEPTH_TEST, newState.depthTesting || newState.depthWriting);

  if (newState.depthWriting && !newState.depthTesting)
  {
    const Function depthFunction = ALLOW_ALWAYS;
    glDepthFunc(convertToGL(depthFunction));
    m_currentState.depthFunction = depthFunction;
  }
  else
    glDepthFunc(convertToGL(newState.depthFunction));

  const GLboolean state = newState.colorWriting ? GL_TRUE : GL_FALSE;
  glColorMask(state, state, state, state);

  const GLenum polygonMode = newState.wireframe ? GL_LINE : GL_FILL;
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

  setBooleanState(GL_LINE_SMOOTH, newState.lineSmoothing);
  glLineWidth(newState.lineWidth);

  setBooleanState(GL_MULTISAMPLE, newState.multisampling);

  setBooleanState(GL_STENCIL_TEST, newState.stencilTesting);
  glStencilFunc(convertToGL(newState.stencilFunction),
                newState.stencilRef, newState.stencilMask);
  glStencilOp(convertToGL(newState.stencilFailOp),
              convertToGL(newState.depthFailOp),
              convertToGL(newState.depthPassOp));

#if WENDY_DEBUG
  checkGL("Error when forcing render state");
#endif

  m_dirtyState = false;
}

void Context::onFrame()
{
#if WENDY_DEBUG
  checkGL("Uncaught OpenGL error during last frame");
#endif

  setCurrentProgram(nullptr);
  setCurrentVertexBuffer(nullptr);
  setCurrentIndexBuffer(nullptr);

  for (size_t i = 0;  i < m_textureUnits.size();  i++)
  {
    if (m_textureUnits[i])
    {
      setActiveTextureUnit(i);
      setCurrentTexture(nullptr);
    }
  }

  if (m_stats)
    m_stats->addFrame();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

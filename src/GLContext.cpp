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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Timer.h>
#include <wendy/Profile.h>

#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.h>

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
  switch (attribute.getType())
  {
    case ATTRIBUTE_FLOAT:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 1)
        return true;

      break;
    }

    case ATTRIBUTE_VEC2:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 2)
        return true;

      break;
    }

    case ATTRIBUTE_VEC3:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 3)
        return true;

      break;
    }

    case ATTRIBUTE_VEC4:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 4)
        return true;

      break;
    }
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
  frameCount(0),
  frameRate(0.f),
  textureCount(0),
  vertexBufferCount(0),
  indexBufferCount(0),
  programCount(0),
  textureSize(0),
  vertexBufferSize(0),
  indexBufferSize(0)
{
  frames.push_back(Frame());

  timer.start();
}

void Stats::addFrame()
{
  frameCount++;
  frameRate = 0.f;

  if (!frames.empty())
  {
    // Add the previous frame duration
    frames.front().duration = timer.getDeltaTime();

    // Calculate frame rate
    for (auto f = frames.begin();  f != frames.end();  f++)
      frameRate += float(f->duration);

    frameRate = float(frames.size()) / frameRate;
  }

  // Add new empty frame for recording the stats
  frames.push_front(Frame());
  if (frames.size() > 60)
    frames.pop_back();
}

void Stats::addStateChange()
{
  Frame& frame = frames.front();
  frame.stateChangeCount++;
}

void Stats::addPrimitives(PrimitiveType type, uint vertexCount)
{
  Frame& frame = frames.front();
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
  textureCount++;
  textureSize += size;
}

void Stats::removeTexture(size_t size)
{
  textureCount--;
  textureSize -= size;
}

void Stats::addVertexBuffer(size_t size)
{
  vertexBufferCount++;
  vertexBufferSize += size;
}

void Stats::removeVertexBuffer(size_t size)
{
  vertexBufferCount--;
  vertexBufferSize -= size;
}

void Stats::addIndexBuffer(size_t size)
{
  indexBufferCount++;
  indexBufferSize += size;
}

void Stats::removeIndexBuffer(size_t size)
{
  indexBufferCount--;
  indexBufferSize -= size;
}

void Stats::addProgram()
{
  programCount++;
}

void Stats::removeProgram()
{
  programCount--;
}

float Stats::getFrameRate() const
{
  return frameRate;
}

uint Stats::getFrameCount() const
{
  return frameCount;
}

const Stats::Frame& Stats::getCurrentFrame() const
{
  return frames.front();
}

uint Stats::getTextureCount() const
{
  return textureCount;
}

uint Stats::getVertexBufferCount() const
{
  return vertexBufferCount;
}

uint Stats::getIndexBufferCount() const
{
  return indexBufferCount;
}

uint Stats::getProgramCount() const
{
  return programCount;
}

size_t Stats::getTotalTextureSize() const
{
  return textureSize;
}

size_t Stats::getTotalVertexBufferSize() const
{
  return vertexBufferSize;
}

size_t Stats::getTotalIndexBufferSize() const
{
  return indexBufferSize;
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
  if (defaultFramebuffer)
    setDefaultFramebufferCurrent();

  setCurrentVertexBuffer(NULL);
  setCurrentIndexBuffer(NULL);
  setCurrentProgram(NULL);

  for (size_t i = 0;  i < textureUnits.size();  i++)
  {
    setActiveTextureUnit(i);
    setCurrentTexture(NULL);
  }

  if (handle)
  {
    glfwDestroyWindow(handle);
    handle = NULL;
  }
}

void Context::clearColorBuffer(const vec4& color)
{
  const RenderState previousState = currentState;

  RenderState clearState = currentState;
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
  const RenderState previousState = currentState;

  RenderState clearState = currentState;
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
  const RenderState previousState = currentState;

  RenderState clearState = currentState;
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
  const RenderState previousState = currentState;

  RenderState clearState = currentState;
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
    logWarning("Rendering empty primitive range with shader program \'%s\'",
               currentProgram->getName().c_str());
    return;
  }

  setCurrentVertexBuffer(range.getVertexBuffer());
  setCurrentIndexBuffer(range.getIndexBuffer());

  render(range.getType(), range.getStart(), range.getCount(), range.getBase());
}

void Context::render(PrimitiveType type,
                     uint start,
                     uint count,
                     uint base)
{
  ProfileNodeCall call("GL::Context::render");

  if (!currentProgram)
  {
    logError("Cannot render without a current shader program");
    return;
  }

  if (!currentVertexBuffer)
  {
    logError("Cannot render without a current vertex buffer");
    return;
  }

  if (dirtyBinding)
  {
    const VertexFormat& format = currentVertexBuffer->getFormat();

    if (currentProgram->getAttributeCount() > format.getComponentCount())
    {
      logError("Shader program \'%s\' has more attributes than vertex format has components",
               currentProgram->getName().c_str());
      return;
    }

    for (size_t i = 0;  i < currentProgram->getAttributeCount();  i++)
    {
      Attribute& attribute = currentProgram->getAttribute(i);

      const VertexComponent* component = format.findComponent(attribute.getName().c_str());
      if (!component)
      {
        logError("Attribute \'%s\' of program \'%s\' has no corresponding vertex format component",
                 attribute.getName().c_str(),
                 currentProgram->getName().c_str());
        return;
      }

      if (!isCompatible(attribute, *component))
      {
        logError("Attribute \'%s\' of shader program \'%s\' has incompatible type",
                 attribute.getName().c_str(),
                 currentProgram->getName().c_str());
        return;
      }

      attribute.bind(format.getSize(), component->getOffset());
    }

    dirtyBinding = false;
  }

#if WENDY_DEBUG
  if (!currentProgram->isValid())
    return;
#endif

  if (currentIndexBuffer)
  {
    const size_t size = IndexBuffer::getTypeSize(currentIndexBuffer->getType());

    glDrawElementsBaseVertex(convertToGL(type),
                             count,
                             convertToGL(currentIndexBuffer->getType()),
                             (GLvoid*) (size * start),
                             base);
  }
  else
    glDrawArrays(convertToGL(type), start, count);

  if (stats)
    stats->addPrimitives(type, count);
}

void Context::createSharedSampler(const char* name, SamplerType type, int ID)
{
  assert(ID != INVALID_SHARED_STATE_ID);

  if (getSharedSamplerID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  declaration += format("uniform %s %s;\n", Sampler::getTypeName(type), name);

  samplers.push_back(SharedSampler(name, type, ID));
}

void Context::createSharedUniform(const char* name, UniformType type, int ID)
{
  assert(ID != INVALID_SHARED_STATE_ID);

  if (getSharedUniformID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  declaration += format("uniform %s %s;\n", Uniform::getTypeName(type), name);

  uniforms.push_back(SharedUniform(name, type, ID));
}

int Context::getSharedSamplerID(const char* name, SamplerType type) const
{
  for (auto s = samplers.begin(); s != samplers.end(); s++)
  {
    if (s->name == name && s->type == type)
      return s->ID;
  }

  return INVALID_SHARED_STATE_ID;
}

int Context::getSharedUniformID(const char* name, UniformType type) const
{
  for (auto u = uniforms.begin(); u != uniforms.end(); u++)
  {
    if (u->name == name && u->type == type)
      return u->ID;
  }

  return INVALID_SHARED_STATE_ID;
}

SharedProgramState* Context::getCurrentSharedProgramState() const
{
  return currentSharedState;
}

void Context::setCurrentSharedProgramState(SharedProgramState* newState)
{
  currentSharedState = newState;
}

const char* Context::getSharedProgramStateDeclaration() const
{
  return declaration.c_str();
}

int Context::getSwapInterval() const
{
  return swapInterval;
}

void Context::setSwapInterval(int newInterval)
{
  glfwSwapInterval(newInterval);
  swapInterval = newInterval;
}

const Recti& Context::getScissorArea() const
{
  return scissorArea;
}

const Recti& Context::getViewportArea() const
{
  return viewportArea;
}

void Context::setScissorArea(const Recti& newArea)
{
  scissorArea = newArea;

  const uint width = currentFramebuffer->getWidth();
  const uint height = currentFramebuffer->getHeight();

  if (scissorArea == Recti(0, 0, width, height))
    glDisable(GL_SCISSOR_TEST);
  else
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor(scissorArea.position.x,
              scissorArea.position.y,
              scissorArea.size.x,
              scissorArea.size.y);
  }
}

void Context::setViewportArea(const Recti& newArea)
{
  viewportArea = newArea;

  glViewport(viewportArea.position.x,
             viewportArea.position.y,
             viewportArea.size.x,
             viewportArea.size.y);
}

Framebuffer& Context::getCurrentFramebuffer() const
{
  return *currentFramebuffer;
}

DefaultFramebuffer& Context::getDefaultFramebuffer() const
{
  return *defaultFramebuffer;
}

void Context::setDefaultFramebufferCurrent()
{
  setCurrentFramebuffer(*defaultFramebuffer);
}

bool Context::setCurrentFramebuffer(Framebuffer& newFramebuffer)
{
  currentFramebuffer = &newFramebuffer;
  currentFramebuffer->apply();

#if WENDY_DEBUG
  if (currentFramebuffer != defaultFramebuffer)
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

Program* Context::getCurrentProgram() const
{
  return currentProgram;
}

void Context::setCurrentProgram(Program* newProgram)
{
  if (newProgram != currentProgram)
  {
    if (currentProgram)
      currentProgram->unbind();

    currentProgram = newProgram;
    dirtyBinding = true;

    if (currentProgram)
      currentProgram->bind();
    else
      glUseProgram(0);
  }
}

VertexBuffer* Context::getCurrentVertexBuffer() const
{
  return currentVertexBuffer;
}

void Context::setCurrentVertexBuffer(VertexBuffer* newVertexBuffer)
{
  if (newVertexBuffer != currentVertexBuffer)
  {
    currentVertexBuffer = newVertexBuffer;
    dirtyBinding = true;

    if (currentVertexBuffer)
      glBindBuffer(GL_ARRAY_BUFFER, currentVertexBuffer->bufferID);
    else
      glBindBuffer(GL_ARRAY_BUFFER, 0);

#if WENDY_DEBUG
    if (!checkGL("Failed to make index buffer current"))
      return;
#endif
  }
}

IndexBuffer* Context::getCurrentIndexBuffer() const
{
  return currentIndexBuffer;
}

void Context::setCurrentIndexBuffer(IndexBuffer* newIndexBuffer)
{
  if (newIndexBuffer != currentIndexBuffer)
  {
    currentIndexBuffer = newIndexBuffer;
    dirtyBinding = true;

    if (currentIndexBuffer)
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentIndexBuffer->bufferID);
    else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#if WENDY_DEBUG
    if (!checkGL("Failed to apply index buffer"))
      return;
#endif
  }
}

Texture* Context::getCurrentTexture() const
{
  return textureUnits[activeTextureUnit];
}

void Context::setCurrentTexture(Texture* newTexture)
{
  if (textureUnits[activeTextureUnit] != newTexture)
  {
    Texture* oldTexture = textureUnits[activeTextureUnit];

    if (oldTexture)
    {
      if (!newTexture || oldTexture->type != newTexture->type)
      {
        glBindTexture(convertToGL(oldTexture->type), 0);

#if WENDY_DEBUG
        if (!checkGL("Failed to unbind texture \'%s\'",
                     oldTexture->getName().c_str()))
        {
          return;
        }
#endif
      }
    }

    if (newTexture)
    {
      glBindTexture(convertToGL(newTexture->type), newTexture->textureID);

#if WENDY_DEBUG
      if (!checkGL("Failed to bind texture \'%s\'",
                   newTexture->getName().c_str()))
      {
        return;
      }
#endif
    }

    textureUnits[activeTextureUnit] = newTexture;
  }
}

uint Context::getTextureUnitCount() const
{
  return (uint) textureUnits.size();
}

uint Context::getActiveTextureUnit() const
{
  return activeTextureUnit;
}

void Context::setActiveTextureUnit(uint unit)
{
  if (activeTextureUnit != unit)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    activeTextureUnit = unit;

#if WENDY_DEBUG
    if (!checkGL("Failed to activate texture unit %u", unit))
      return;
#endif
  }
}

bool Context::isCullingInverted()
{
  return cullingInverted;
}

void Context::setCullingInversion(bool newState)
{
  cullingInverted = newState;
}

const RenderState& Context::getCurrentRenderState() const
{
  return currentState;
}

void Context::setCurrentRenderState(const RenderState& newState)
{
  applyState(newState);
}

Stats* Context::getStats() const
{
  return stats;
}

void Context::setStats(Stats* newStats)
{
  stats = newStats;
}

ResourceCache& Context::getCache() const
{
  return cache;
}

Window& Context::getWindow()
{
  return window;
}

const Limits& Context::getLimits() const
{
  return *limits;
}

Context* Context::create(ResourceCache& cache,
                         const WindowConfig& wc,
                         const ContextConfig& cc)
{
  Ptr<Context> context(new Context(cache));
  if (!context->init(wc, cc))
    return NULL;

  return context.detachObject();
}

Context::Context(ResourceCache& initCache):
  cache(initCache),
  handle(NULL),
  dirtyBinding(true),
  dirtyState(true),
  cullingInverted(false),
  activeTextureUnit(0),
  stats(NULL)
{
}

Context::Context(const Context& source):
  cache(source.cache)
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

    GLFWmonitor* monitor = NULL;

    if (wc.mode == FULLSCREEN)
      monitor = glfwGetPrimaryMonitor();

    handle = glfwCreateWindow(wc.width, wc.height, wc.title.c_str(), monitor, NULL);
    if (!handle)
    {
      logError("Failed to create GLFW window");
      return false;
    }

    glfwSetWindowUserPointer(handle, this);
    glfwMakeContextCurrent(handle);

    log("OpenGL context version %i.%i created",
        glfwGetWindowAttrib(handle, GLFW_CONTEXT_VERSION_MAJOR),
        glfwGetWindowAttrib(handle, GLFW_CONTEXT_VERSION_MINOR));

    log("OpenGL context GLSL version is %s",
        (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

    log("OpenGL context renderer is %s by %s",
        (const char*) glGetString(GL_RENDERER),
        (const char*) glGetString(GL_VENDOR));

    window.init(handle);
    window.getFrameSignal().connect(*this, &Context::onFrame);
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
      glDebugMessageCallbackARB(debugCallback, NULL);
      glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
  }

  // Retrieve context limits and set up dependent caches
  {
    limits = new Limits(*this);

    const uint unitCount = max(limits->maxCombinedTextureImageUnits,
                               limits->maxTextureCoords);

    textureUnits.resize(unitCount);
  }

  // Create and apply default framebuffer
  {
    defaultFramebuffer = new DefaultFramebuffer(*this);

    // Read back actual (as opposed to desired) properties
    defaultFramebuffer->colorBits = getInteger(GL_RED_BITS) +
                                    getInteger(GL_GREEN_BITS) +
                                    getInteger(GL_BLUE_BITS);
    defaultFramebuffer->depthBits = getInteger(GL_DEPTH_BITS);
    defaultFramebuffer->stencilBits = getInteger(GL_STENCIL_BITS);
    defaultFramebuffer->samples = getInteger(GL_SAMPLES);

    setDefaultFramebufferCurrent();
  }

  // Force a known GL state
  {
    int width, height;
    glfwGetWindowSize(handle, &width, &height);

    setViewportArea(Recti(0, 0, width, height));
    setScissorArea(Recti(0, 0, width, height));

    setSwapInterval(1);
    forceState(currentState);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  return true;
}

void Context::applyState(const RenderState& newState)
{
  if (stats)
    stats->addStateChange();

  if (dirtyState)
  {
    forceState(newState);
    return;
  }

  CullMode cullMode = newState.cullMode;
  if (cullingInverted)
    cullMode = invertCullMode(cullMode);

  if (cullMode != currentState.cullMode)
  {
    if ((cullMode == CULL_NONE) != (currentState.cullMode == CULL_NONE))
      setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);

    if (cullMode != CULL_NONE)
      glCullFace(convertToGL(cullMode));

    currentState.cullMode = cullMode;
  }

  if (newState.srcFactor != currentState.srcFactor ||
      newState.dstFactor != currentState.dstFactor)
  {
    setBooleanState(GL_BLEND, newState.srcFactor != BLEND_ONE ||
                              newState.dstFactor != BLEND_ZERO);

    if (newState.srcFactor != BLEND_ONE || newState.dstFactor != BLEND_ZERO)
    {
      glBlendFunc(convertToGL(newState.srcFactor),
                  convertToGL(newState.dstFactor));
    }

    currentState.srcFactor = newState.srcFactor;
    currentState.dstFactor = newState.dstFactor;
  }

  if (newState.depthTesting || newState.depthWriting)
  {
    // Set depth buffer writing.
    if (newState.depthWriting != currentState.depthWriting)
      glDepthMask(newState.depthWriting ? GL_TRUE : GL_FALSE);

    if (newState.depthTesting)
    {
      // Set depth buffer function.
      if (newState.depthFunction != currentState.depthFunction)
      {
        glDepthFunc(convertToGL(newState.depthFunction));
        currentState.depthFunction = newState.depthFunction;
      }
    }
    else if (newState.depthWriting)
    {
      // NOTE: Special case; depth buffer filling.
      //       Set specific depth buffer function.
      const Function depthFunction = ALLOW_ALWAYS;

      if (currentState.depthFunction != depthFunction)
      {
        glDepthFunc(convertToGL(depthFunction));
        currentState.depthFunction = depthFunction;
      }
    }

    if (!(currentState.depthTesting || currentState.depthWriting))
      glEnable(GL_DEPTH_TEST);
  }
  else
  {
    if (currentState.depthTesting || currentState.depthWriting)
      glDisable(GL_DEPTH_TEST);
  }

  currentState.depthTesting = newState.depthTesting;
  currentState.depthWriting = newState.depthWriting;

  if (newState.colorWriting != currentState.colorWriting)
  {
    const GLboolean state = newState.colorWriting ? GL_TRUE : GL_FALSE;
    glColorMask(state, state, state, state);
    currentState.colorWriting = newState.colorWriting;
  }

  if (newState.stencilTesting != currentState.stencilTesting)
  {
    setBooleanState(GL_STENCIL_TEST, newState.stencilTesting);
    currentState.stencilTesting = newState.stencilTesting;
  }

  if (newState.stencilTesting)
  {
    if (newState.stencilFunction != currentState.stencilFunction ||
        newState.stencilRef != currentState.stencilRef ||
        newState.stencilMask != currentState.stencilMask)
    {
      glStencilFunc(convertToGL(newState.stencilFunction),
                    newState.stencilRef, newState.stencilMask);

      currentState.stencilFunction = newState.stencilFunction;
      currentState.stencilRef = newState.stencilRef;
      currentState.stencilMask = newState.stencilMask;
    }

    if (newState.stencilFailOp != currentState.stencilFailOp ||
        newState.depthFailOp != currentState.depthFailOp ||
        newState.depthPassOp != currentState.depthPassOp)
    {
      glStencilOp(convertToGL(newState.stencilFailOp),
                  convertToGL(newState.depthFailOp),
                  convertToGL(newState.depthPassOp));

      currentState.stencilFailOp = newState.stencilFailOp;
      currentState.depthFailOp = newState.depthFailOp;
      currentState.depthPassOp = newState.depthPassOp;
    }
  }

  if (newState.wireframe != currentState.wireframe)
  {
    const GLenum state = newState.wireframe ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, state);
    currentState.wireframe = newState.wireframe;
  }

  if (newState.lineSmoothing != currentState.lineSmoothing)
  {
    setBooleanState(GL_LINE_SMOOTH, newState.lineSmoothing);
    currentState.lineSmoothing = newState.lineSmoothing;
  }

  if (newState.multisampling != currentState.multisampling)
  {
    setBooleanState(GL_MULTISAMPLE, newState.multisampling);
    currentState.multisampling = newState.multisampling;
  }

  if (newState.lineWidth != currentState.lineWidth)
  {
    glLineWidth(newState.lineWidth);
    currentState.lineWidth = newState.lineWidth;
  }

#if WENDY_DEBUG
  checkGL("Error when applying render state");
#endif
}

void Context::forceState(const RenderState& newState)
{
  currentState = newState;

  CullMode cullMode = newState.cullMode;
  if (cullingInverted)
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
    currentState.depthFunction = depthFunction;
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

  dirtyState = false;
}

void Context::onFrame()
{
#if WENDY_DEBUG
  checkGL("Uncaught OpenGL error during last frame");
#endif

  setCurrentProgram(NULL);
  setCurrentVertexBuffer(NULL);
  setCurrentIndexBuffer(NULL);

  for (size_t i = 0;  i < textureUnits.size();  i++)
  {
    if (textureUnits[i])
    {
      setActiveTextureUnit(i);
      setCurrentTexture(NULL);
    }
  }

  if (stats)
    stats->addFrame();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

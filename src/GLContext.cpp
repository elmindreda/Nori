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

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.h>

#define GLFW_NO_GLU
#include <GL/glfw.h>

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

GLint getIntegerParameter(GLenum parameter)
{
  GLint value;
  glGetIntegerv(parameter, &value);
  return value;
}

GLfloat getFloatParameter(GLenum parameter)
{
  GLfloat value;
  glGetFloatv(parameter, &value);
  return value;
}

const char* getFramebufferStatusMessage(GLenum status)
{
  switch (status)
  {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      return "Framebuffer is complete";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      return "Incomplete framebuffer attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      return "Incomplete or missing framebuffer attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      return "Incomplete framebuffer dimensions";
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      return "Incomplete framebuffer formats";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      return "Incomplete framebuffer draw buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      return "Incomplete framebuffer read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      return "Framebuffer configuration is unsupported";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB:
      return "Framebuffer layer targets incomplete";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB:
      return "Framebuffer layer counts incomplete";
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

} /*namespace (and Gandalf)*/

///////////////////////////////////////////////////////////////////////

Version::Version():
  m(1),
  n(0)
{
}

Version::Version(unsigned int initM, unsigned int initN):
  m(initM),
  n(initN)
{
}

bool Version::operator < (const Version& other) const
{
  if (m < other.m)
    return true;

  if (m == other.m && n < other.n)
    return true;

  return false;
}

bool Version::operator > (const Version& other) const
{
  if (m > other.m)
    return true;

  if (m == other.m && n > other.n)
    return true;

  return false;
}

///////////////////////////////////////////////////////////////////////

WindowConfig::WindowConfig():
  title("Wendy"),
  width(640),
  height(480),
  mode(WINDOWED),
  resizable(true)
{
}

WindowConfig::WindowConfig(const String& initTitle):
  title(initTitle),
  width(640),
  height(480),
  mode(WINDOWED),
  resizable(true)
{
}

WindowConfig::WindowConfig(const String& initTitle,
                           unsigned int initWidth,
                           unsigned int initHeight,
                           WindowMode initMode,
                           bool initResizable):
  title(initTitle),
  width(initWidth),
  height(initHeight),
  mode(initMode),
  resizable(initResizable)
{
}

///////////////////////////////////////////////////////////////////////

ContextConfig::ContextConfig(unsigned int initColorBits,
                             unsigned int initDepthBits,
                             unsigned int initStencilBits,
                             unsigned int initSamples,
                             Version initVersion,
                             Profile initProfile):
  colorBits(initColorBits),
  depthBits(initDepthBits),
  stencilBits(initStencilBits),
  samples(initSamples),
  version(initVersion),
  profile(initProfile)
{
}

///////////////////////////////////////////////////////////////////////

Limits::Limits(Context& context)
{
  maxColorAttachments = getIntegerParameter(GL_MAX_COLOR_ATTACHMENTS_EXT);
  maxDrawBuffers = getIntegerParameter(GL_MAX_DRAW_BUFFERS);
  maxVertexTextureImageUnits = getIntegerParameter(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
  maxFragmentTextureImageUnits = getIntegerParameter(GL_MAX_TEXTURE_IMAGE_UNITS);
  maxCombinedTextureImageUnits = getIntegerParameter(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
  maxTextureSize = getIntegerParameter(GL_MAX_TEXTURE_SIZE);
  maxTexture3DSize = getIntegerParameter(GL_MAX_3D_TEXTURE_SIZE);
  maxTextureCubeSize = getIntegerParameter(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
  maxTextureRectangleSize = getIntegerParameter(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB);
  maxTextureCoords = getIntegerParameter(GL_MAX_TEXTURE_COORDS);
  maxVertexAttributes = getIntegerParameter(GL_MAX_VERTEX_ATTRIBS);

  Version version = context.getVersion();

  if (GLEW_EXT_texture_filter_anisotropic)
    maxTextureAnisotropy = getFloatParameter(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
  else
    maxTextureAnisotropy = 1.f;

  if (GLEW_ARB_geometry_shader4 || version > Version(3,1))
  {
    maxGeometryOutputVertices = getIntegerParameter(GL_MAX_GEOMETRY_OUTPUT_VERTICES);
    maxGeometryTextureImageUnits = getIntegerParameter(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS);
  }
  else
  {
    maxGeometryOutputVertices = 0;
    maxGeometryTextureImageUnits = 0;
  }

  if (GLEW_ARB_tessellation_shader || version > Version(3,3))
  {
    maxTessControlTextureImageUnits = getIntegerParameter(GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS);
    maxTessEvaluationTextureImageUnits = getIntegerParameter(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS);
  }
  else
  {
    maxTessControlTextureImageUnits = 0;
    maxTessEvaluationTextureImageUnits = 0;
  }
}

///////////////////////////////////////////////////////////////////////

Stats::Stats():
  frameCount(0),
  frameRate(0.f),
  textureCount(0),
  textureSize(0),
  vertexBufferCount(0),
  vertexBufferSize(0),
  indexBufferCount(0),
  indexBufferSize(0),
  renderBufferCount(0),
  renderBufferSize(0),
  programCount(0)
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
    for (FrameQueue::const_iterator f = frames.begin();  f != frames.end();  f++)
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

void Stats::addPrimitives(PrimitiveType type, unsigned int vertexCount)
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

void Stats::addRenderBuffer(size_t size)
{
  renderBufferCount++;
  renderBufferSize += size;
}

void Stats::removeRenderBuffer(size_t size)
{
  renderBufferCount--;
  renderBufferSize -= size;
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

unsigned int Stats::getFrameCount() const
{
  return frameCount;
}

const Stats::Frame& Stats::getFrame() const
{
  return frames.front();
}

unsigned int Stats::getTextureCount() const
{
  return textureCount;
}

unsigned int Stats::getVertexBufferCount() const
{
  return vertexBufferCount;
}

unsigned int Stats::getIndexBufferCount() const
{
  return indexBufferCount;
}

unsigned int Stats::getRenderBufferCount() const
{
  return renderBufferCount;
}

unsigned int Stats::getProgramCount() const
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

size_t Stats::getTotalRenderBufferSize() const
{
  return renderBufferSize;
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

SharedSampler::SharedSampler(const char* initName,
                             SamplerType initType,
                             int initID):
  name(initName),
  type(initType),
  ID(initID)
{
}

///////////////////////////////////////////////////////////////////////

SharedUniform::SharedUniform(const char* initName,
                             UniformType initType,
                             int initID):
  name(initName),
  type(initType),
  ID(initID)
{
}

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

  glfwCloseWindow();

  instance = NULL;
}

void Context::clearColorBuffer(const vec4& color)
{
  glPushAttrib(GL_COLOR_BUFFER_BIT);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
  glPopAttrib();

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif
}

void Context::clearDepthBuffer(float depth)
{
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_TRUE);
  glClearDepth(depth);
  glClear(GL_DEPTH_BUFFER_BIT);
  glPopAttrib();

#if WENDY_DEBUG
  checkGL("Error during depth buffer clearing");
#endif
}

void Context::clearStencilBuffer(unsigned int value)
{
  glPushAttrib(GL_STENCIL_BUFFER_BIT);
  glStencilMask(GL_TRUE);
  glClearStencil(value);
  glClear(GL_STENCIL_BUFFER_BIT);
  glPopAttrib();

#if WENDY_DEBUG
  checkGL("Error during stencil buffer clearing");
#endif
}

void Context::clearBuffers(const vec4& color, float depth, unsigned int value)
{
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glStencilMask(GL_TRUE);

  glClearColor(color.r, color.g, color.b, color.a);
  glClearDepth(depth);
  glClearStencil(value);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glPopAttrib();

#if WENDY_DEBUG
  checkGL("Error during buffer clearing");
#endif
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

  render(range.getType(), range.getStart(), range.getCount());
}

void Context::render(PrimitiveType type, unsigned int start, unsigned int count)
{
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

  GLenum mode;

  if (currentProgram->hasTessellationShaders())
  {
    mode = GL_PATCHES;

    switch (type)
    {
      case POINT_LIST:
        glPatchParameteri(GL_PATCH_VERTICES, 1);
        break;
      case LINE_LIST:
      case LINE_STRIP:
      case LINE_LOOP:
        glPatchParameteri(GL_PATCH_VERTICES, 2);
        break;
      case TRIANGLE_LIST:
      case TRIANGLE_STRIP:
      case TRIANGLE_FAN:
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        break;
    }
  }
  else
    mode = convertToGL(type);

  if (currentIndexBuffer)
  {
    const size_t size = IndexBuffer::getTypeSize(currentIndexBuffer->getType());

    glDrawElements(mode,
                   count,
                   convertToGL(currentIndexBuffer->getType()),
                   (GLvoid*) (size * start));
  }
  else
    glDrawArrays(mode, start, count);

  if (stats)
    stats->addPrimitives(type, count);
}

void Context::refresh()
{
  needsRefresh = true;
}

bool Context::update()
{
  glfwSwapBuffers();
  finishSignal();
  needsRefresh = false;

#if WENDY_DEBUG
  checkGL("Uncaught OpenGL error during last frame");
#endif

  if (stats)
    stats->addFrame();

  if (refreshMode == MANUAL_REFRESH)
  {
    while (!needsRefresh && !needsClosing)
      glfwWaitEvents();
  }
  else
    glfwPollEvents();

  return !needsClosing;
}

void Context::requestClose()
{
  closeCallback();
}

void Context::createSharedSampler(const char* name, SamplerType type, int ID)
{
  assert(ID != INVALID_SHARED_STATE_ID);

  if (getSharedSamplerID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  declaration += "uniform ";
  declaration += Sampler::getTypeName(type);
  declaration += " ";
  declaration += name;
  declaration += ";\n";

  samplers.push_back(SharedSampler(name, type, ID));
}

void Context::createSharedUniform(const char* name, UniformType type, int ID)
{
  assert(ID != INVALID_SHARED_STATE_ID);

  if (getSharedUniformID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  declaration += "uniform ";
  declaration += Uniform::getTypeName(type);
  declaration += " ";
  declaration += name;
  declaration += ";\n";

  uniforms.push_back(SharedUniform(name, type, ID));
}

int Context::getSharedSamplerID(const char* name, SamplerType type) const
{
  for (SamplerList::const_iterator s = samplers.begin(); s != samplers.end(); s++)
  {
    if (s->name == name && s->type == type)
      return s->ID;
  }

  return INVALID_SHARED_STATE_ID;
}

int Context::getSharedUniformID(const char* name, UniformType type) const
{
  for (UniformList::const_iterator u = uniforms.begin(); u != uniforms.end(); u++)
  {
    if (u->name == name && u->type == type)
      return u->ID;
  }

  return INVALID_SHARED_STATE_ID;
}

SharedProgramState* Context::getCurrentSharedProgramState() const
{
  return currentState;
}

void Context::setCurrentSharedProgramState(SharedProgramState* newState)
{
  currentState = newState;
}

const char* Context::getSharedProgramStateDeclaration() const
{
  return declaration.c_str();
}

WindowMode Context::getWindowMode() const
{
  return windowMode;
}

Context::RefreshMode Context::getRefreshMode() const
{
  return refreshMode;
}

void Context::setRefreshMode(RefreshMode newMode)
{
  refreshMode = newMode;
}

unsigned int Context::getSwapInterval() const
{
  return swapInterval;
}

void Context::setSwapInterval(unsigned int newInterval)
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

  const unsigned int width = currentFramebuffer->getWidth();
  const unsigned int height = currentFramebuffer->getHeight();

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
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status == 0)
      checkGL("Framebuffer status check failed");
    else if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
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

unsigned int Context::getActiveTextureUnit() const
{
  return activeTextureUnit;
}

void Context::setActiveTextureUnit(unsigned int unit)
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

Stats* Context::getStats() const
{
  return stats;
}

void Context::setStats(Stats* newStats)
{
  stats = newStats;
}

const String& Context::getTitle() const
{
  return title;
}

void Context::setTitle(const char* newTitle)
{
  glfwSetWindowTitle(newTitle);
  title = newTitle;
}

ResourceCache& Context::getCache() const
{
  return cache;
}

Version Context::getVersion() const
{
  return version;
}

const Limits& Context::getLimits() const
{
  return *limits;
}

SignalProxy0<void> Context::getFinishSignal()
{
  return finishSignal;
}

SignalProxy0<bool> Context::getCloseRequestSignal()
{
  return closeRequestSignal;
}

SignalProxy2<void, unsigned int, unsigned int> Context::getResizedSignal()
{
  return resizedSignal;
}

bool Context::createSingleton(ResourceCache& cache,
                              const WindowConfig& windowConfig,
                              const ContextConfig& contextConfig)
{
  Ptr<Context> context(new Context(cache));
  if (!context->init(windowConfig, contextConfig))
    return false;

  set(context.detachObject());
  return true;
}

Context::Context(ResourceCache& initCache):
  cache(initCache),
  refreshMode(AUTOMATIC_REFRESH),
  needsRefresh(false),
  needsClosing(false),
  dirtyBinding(true),
  activeTextureUnit(0),
  stats(NULL)
{
  // Necessary hack in case GLFW calls a callback before
  // we have had time to call Singleton::set.

  // TODO: Remove this upon the arrival of GLFW user pointers.

  instance = this;
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

bool Context::init(const WindowConfig& windowConfig,
                   const ContextConfig& contextConfig)
{
  if (!glfwInit())
  {
    logError("Failed to initialize GLFW");
    return false;
  }

  // Create context and window
  {
    unsigned int colorBits = contextConfig.colorBits;
    if (colorBits > 24)
      colorBits = 24;

    unsigned int mode;

    if (windowConfig.mode == WINDOWED)
      mode = GLFW_WINDOW;
    else
      mode = GLFW_FULLSCREEN;

    if (contextConfig.samples)
      glfwOpenWindowHint(GLFW_FSAA_SAMPLES, contextConfig.samples);

    version = contextConfig.version;
    if (version < Version(2,1))
      version = Version(2,1);

    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, version.m);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, version.n);

    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, !windowConfig.resizable);

    if (version > Version(3,1))
    {
      // Wendy still uses deprecated functionality
      glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    }

#if WENDY_DEBUG
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    if (!glfwOpenWindow(windowConfig.width, windowConfig.height,
                        colorBits / 3, colorBits / 3, colorBits / 3, 0,
                        contextConfig.depthBits, contextConfig.stencilBits, mode))
    {
      logError("Failed to create GLFW window");
      return false;
    }

    version = Version(glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR),
                      glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR));

    log("OpenGL context version %i.%i created", version.m, version.n);

    log("OpenGL context GLSL version is %s",
        (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

    log("OpenGL context renderer is %s by %s",
        (const char*) glGetString(GL_RENDERER),
        (const char*) glGetString(GL_VENDOR));

    windowMode = windowConfig.mode;
  }

  // Initialize GLEW and check extensions
  {
    if (glewInit() != GLEW_OK)
    {
      logError("Failed to initialize GLEW");
      return false;
    }

    if (!GLEW_ARB_texture_rectangle && version < Version(3,1))
    {
      logError("Rectangular textures (ARB_texture_rectangle) is required but not supported");
      return false;
    }

    if (!GLEW_EXT_framebuffer_object)
    {
      logError("Framebuffer objects (EXT_framebuffer_object) are required but not supported");
      return false;
    }

#if WENDY_DEBUG
    if (GLEW_ARB_debug_output)
    {
      glDebugMessageCallbackARB(debugCallback, NULL);
      glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
#endif
  }

  // All extensions are there; figure out their limits
  limits = new Limits(*this);

  // Set up texture unit cache
  {
    unsigned int unitCount = max(limits->maxCombinedTextureImageUnits,
                                 limits->maxTextureCoords);

    textureUnits.resize(unitCount);
  }

  // Apply default differences
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Create and apply default framebuffer
  {
    defaultFramebuffer = new DefaultFramebuffer(*this);

    // Read back actual (as opposed to desired) properties

    int width, height;
    glfwGetWindowSize(&width, &height);
    defaultFramebuffer->width = width;
    defaultFramebuffer->height = height;

    defaultFramebuffer->colorBits = glfwGetWindowParam(GLFW_RED_BITS) +
                                    glfwGetWindowParam(GLFW_GREEN_BITS) +
                                    glfwGetWindowParam(GLFW_BLUE_BITS);
    defaultFramebuffer->depthBits = glfwGetWindowParam(GLFW_DEPTH_BITS);
    defaultFramebuffer->stencilBits = glfwGetWindowParam(GLFW_STENCIL_BITS);
    defaultFramebuffer->samples = glfwGetWindowParam(GLFW_FSAA_SAMPLES);

    setDefaultFramebufferCurrent();

    setViewportArea(Recti(0, 0, width, height));
    setScissorArea(Recti(0, 0, width, height));
  }

  // Finish GLFW init
  {
    setTitle(windowConfig.title.c_str());
    setSwapInterval(1);

    glfwSetWindowSizeCallback(sizeCallback);
    glfwSetWindowCloseCallback(closeCallback);
    glfwSetWindowRefreshCallback(refreshCallback);
    glfwDisable(GLFW_AUTO_POLL_EVENTS);
    glfwPollEvents();
  }

  return true;
}

void Context::sizeCallback(int width, int height)
{
  instance->defaultFramebuffer->width = width;
  instance->defaultFramebuffer->height = height;
  instance->resizedSignal(width, height);
}

int Context::closeCallback()
{
  std::vector<bool> results;

  instance->closeRequestSignal(results);

  if (std::find(results.begin(), results.end(), false) == results.end())
    instance->needsClosing = true;

  return GL_TRUE;
}

void Context::refreshCallback()
{
  instance->needsRefresh = true;
}

Context* Context::instance = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

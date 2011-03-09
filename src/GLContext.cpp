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

#include <wendy/OpenGL.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

#include <GL/glfw.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLint getIntegerParameter(GLenum parameter)
{
  GLint value;
  glGetIntegerv(parameter, &value);
  return value;
}

const char* getFramebufferStatusMessage(GLenum status)
{
  switch (status)
  {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      return "Framebuffer is incomplete";
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
  }

  logError("Unknown OpenGL framebuffer status %u", status);
  return "Unknown framebuffer status";
}

GLenum convertToGL(ImageCanvas::Attachment attachment)
{
  switch (attachment)
  {
    case ImageCanvas::COLOR_BUFFER0:
      return GL_COLOR_ATTACHMENT0_EXT;
    case ImageCanvas::COLOR_BUFFER1:
      return GL_COLOR_ATTACHMENT1_EXT;
    case ImageCanvas::COLOR_BUFFER2:
      return GL_COLOR_ATTACHMENT2_EXT;
    case ImageCanvas::COLOR_BUFFER3:
      return GL_COLOR_ATTACHMENT3_EXT;
    case ImageCanvas::DEPTH_BUFFER:
      return GL_DEPTH_ATTACHMENT_EXT;
  }

  logError("Invalid image canvas attachment %u", attachment);
  return 0;
}

const char* getAttachmentName(ImageCanvas::Attachment attachment)
{
  switch (attachment)
  {
    case ImageCanvas::COLOR_BUFFER0:
      return "color buffer 0";
    case ImageCanvas::COLOR_BUFFER1:
      return "color buffer 1";
    case ImageCanvas::COLOR_BUFFER2:
      return "color buffer 2";
    case ImageCanvas::COLOR_BUFFER3:
      return "color buffer 3";
    case ImageCanvas::DEPTH_BUFFER:
      return "depth buffer";
  }

  logError("Invalid image canvas attachment %u", attachment);
  return "unknown buffer";
}

bool isColorAttachment(ImageCanvas::Attachment attachment)
{
  switch (attachment)
  {
    case ImageCanvas::COLOR_BUFFER0:
    case ImageCanvas::COLOR_BUFFER1:
    case ImageCanvas::COLOR_BUFFER2:
    case ImageCanvas::COLOR_BUFFER3:
      return true;
    default:
      return false;
  }
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

  logError("Invalid primitive type %u", type);
  return 0;
}

bool isCompatible(const Varying& varying, const VertexComponent& component)
{
  switch (varying.getType())
  {
    case Varying::FLOAT:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 1)
        return true;

      break;
    }

    case Varying::FLOAT_VEC2:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 2)
        return true;

      break;
    }

    case Varying::FLOAT_VEC3:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 3)
        return true;

      break;
    }

    case Varying::FLOAT_VEC4:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 4)
        return true;

      break;
    }
  }

  return false;
}

enum
{
  STATE_MODEL_MATRIX,
  STATE_VIEW_MATRIX,
  STATE_PROJECTION_MATRIX,
  STATE_MODELVIEW_MATRIX,
  STATE_VIEWPROJECTION_MATRIX,
  STATE_MODELVIEWPROJECTION_MATRIX,
};

} /*namespace (and Gandalf)*/

///////////////////////////////////////////////////////////////////////

ScreenMode::ScreenMode(void)
{
  setDefaults();
}

ScreenMode::ScreenMode(unsigned int initWidth,
                       unsigned int initHeight,
		       unsigned int initColorBits):
  width(initWidth),
  height(initHeight),
  colorBits(initColorBits)
{
}

void ScreenMode::setDefaults(void)
{
  set(640, 480, 0);
}

void ScreenMode::set(unsigned int newWidth,
                     unsigned int newHeight,
		     unsigned int newColorBits)
{
  width = newWidth;
  height = newHeight;
  colorBits = newColorBits;
}

///////////////////////////////////////////////////////////////////////

ContextMode::ContextMode(void)
{
  setDefaults();
}

ContextMode::ContextMode(unsigned int width,
                         unsigned int height,
			 unsigned int colorBits,
			 unsigned int initDepthBits,
			 unsigned int initStencilBits,
			 unsigned int initSamples,
			 WindowMode initMode):
  ScreenMode(width, height, colorBits),
  depthBits(initDepthBits),
  stencilBits(initStencilBits),
  samples(initSamples),
  mode(initMode)
{
}

void ContextMode::setDefaults(void)
{
  set(640, 480, 32, 32, 0, 0, WINDOWED);
}

void ContextMode::set(unsigned int width,
                      unsigned int height,
		      unsigned int colorBits,
		      unsigned int newDepthBits,
		      unsigned int newStencilBits,
		      unsigned int newSamples,
		      WindowMode newMode)
{
  ScreenMode::set(width, height, colorBits);

  depthBits = newDepthBits;
  stencilBits = newStencilBits;
  samples = newSamples;
  mode = newMode;
}

///////////////////////////////////////////////////////////////////////

Limits::Limits(Context& initContext):
  context(initContext),
  maxColorAttachments(0),
  maxDrawBuffers(0),
  maxClipPlanes(0),
  maxFragmentTextureImageUnits(0),
  maxVertexTextureImageUnits(0),
  maxTextureSize(0),
  maxTextureCubeSize(0),
  maxTextureRectangleSize(0),
  maxVertexAttributes(0)
{
  maxColorAttachments = getIntegerParameter(GL_MAX_COLOR_ATTACHMENTS_EXT);
  maxDrawBuffers = getIntegerParameter(GL_MAX_DRAW_BUFFERS_ARB);
  maxClipPlanes = getIntegerParameter(GL_MAX_CLIP_PLANES);
  maxFragmentTextureImageUnits = getIntegerParameter(GL_MAX_TEXTURE_IMAGE_UNITS);
  maxVertexTextureImageUnits = getIntegerParameter(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
  maxTextureSize = getIntegerParameter(GL_MAX_TEXTURE_SIZE);
  maxTextureCubeSize = getIntegerParameter(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB);
  maxTextureRectangleSize = getIntegerParameter(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB);
  maxVertexAttributes = getIntegerParameter(GL_MAX_VERTEX_ATTRIBS_ARB);
}

unsigned int Limits::getMaxColorAttachments(void) const
{
  return maxColorAttachments;
}

unsigned int Limits::getMaxDrawBuffers(void) const
{
  return maxDrawBuffers;
}

unsigned int Limits::getMaxClipPlanes(void) const
{
  return maxClipPlanes;
}

unsigned int Limits::getMaxFragmentTextureImageUnits(void) const
{
  return maxFragmentTextureImageUnits;
}

unsigned int Limits::getMaxVertexTextureImageUnits(void) const
{
  return maxVertexTextureImageUnits;
}

unsigned int Limits::getMaxTextureSize(void) const
{
  return maxTextureSize;
}

unsigned int Limits::getMaxTextureCubeSize(void) const
{
  return maxTextureCubeSize;
}

unsigned int Limits::getMaxTextureRectangleSize(void) const
{
  return maxTextureRectangleSize;
}

///////////////////////////////////////////////////////////////////////

Canvas::~Canvas(void)
{
}

float Canvas::getAspectRatio(void) const
{
  return getWidth() / (float) getHeight();
}

Context& Canvas::getContext(void) const
{
  return context;
}

Canvas::Canvas(Context& initContext):
  context(initContext)
{
}

void Canvas::apply(void) const
{
  current = this;
}

bool Canvas::isCurrent(void) const
{
  return this == current;
}

const Canvas* Canvas::getCurrent(void)
{
  return current;
}

Canvas::Canvas(const Canvas& source):
  context(source.context)
{
  // NOTE: Not implemented.
}

Canvas& Canvas::operator = (const Canvas& source)
{
  // NOTE: Not implemented.

  return *this;
}

const Canvas* Canvas::current = NULL;

///////////////////////////////////////////////////////////////////////

unsigned int ScreenCanvas::getColorBits(void) const
{
  return mode.colorBits;
}

unsigned int ScreenCanvas::getDepthBits(void) const
{
  return mode.depthBits;
}

unsigned int ScreenCanvas::getStencilBits(void) const
{
  return mode.stencilBits;
}

unsigned int ScreenCanvas::getWidth(void) const
{
  return mode.width;
}

unsigned int ScreenCanvas::getHeight(void) const
{
  return mode.height;
}

ScreenCanvas::ScreenCanvas(Context& context):
  Canvas(context)
{
  // TODO: Get screen size.
}

void ScreenCanvas::apply(void) const
{
  if (!isCurrent())
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

#if WENDY_DEBUG
    checkGL("Error when applying screen canvas");
#endif

    Canvas::apply();
  }
}

///////////////////////////////////////////////////////////////////////

Image::~Image(void)
{
}

///////////////////////////////////////////////////////////////////////

ImageCanvas::~ImageCanvas(void)
{
  if (bufferID)
    glDeleteFramebuffersEXT(1, &bufferID);
}

unsigned int ImageCanvas::getWidth(void) const
{
  return width;
}

unsigned int ImageCanvas::getHeight(void) const
{
  return height;
}

Image* ImageCanvas::getColorBuffer(void) const
{
  return buffers[COLOR_BUFFER0];
}

Image* ImageCanvas::getDepthBuffer(void) const
{
  return buffers[DEPTH_BUFFER];
}

Image* ImageCanvas::getBuffer(Attachment attachment) const
{
  return buffers[attachment];
}

bool ImageCanvas::setDepthBuffer(Image* newImage)
{
  return setBuffer(DEPTH_BUFFER, newImage);
}

bool ImageCanvas::setColorBuffer(Image* newImage)
{
  return setBuffer(COLOR_BUFFER0, newImage);
}

bool ImageCanvas::setBuffer(Attachment attachment, Image* newImage)
{
  if (newImage)
  {
    if (newImage->getWidth() != width || newImage->getHeight() != height)
    {
      logError("Specified %s image object does not match canvas dimensions",
               getAttachmentName(attachment));
      return false;
    }
  }

  if (isColorAttachment(attachment))
  {
    unsigned int index = attachment - COLOR_BUFFER0;

    if (index >= context.getLimits().getMaxColorAttachments())
    {
      logError("OpenGL context supports at most %u FBO color attachments",
               context.getLimits().getMaxColorAttachments());
      return false;
    }

    if (index >= context.getLimits().getMaxDrawBuffers())
    {
      logError("OpenGL context supports at most %u draw buffers",
               context.getLimits().getMaxDrawBuffers());
      return false;
    }
  }

  const Canvas* previous = getCurrent();
  apply();

  if (buffers[attachment])
    buffers[attachment]->detach(convertToGL(attachment));

  buffers[attachment] = newImage;

  if (buffers[attachment])
    buffers[attachment]->attach(convertToGL(attachment));

  previous->apply();
  return true;
}

ImageCanvas* ImageCanvas::create(Context& context,
                                 unsigned int width,
                                 unsigned int height)
{
  Ptr<ImageCanvas> canvas(new ImageCanvas(context));
  if (!canvas->init(width, height))
    return false;

  return canvas.detachObject();
}

ImageCanvas::ImageCanvas(Context& context):
  Canvas(context),
  width(0),
  height(0),
  bufferID(0)
{
}

bool ImageCanvas::init(unsigned int initWidth, unsigned int initHeight)
{
  width = initWidth;
  height = initHeight;

  glGenFramebuffersEXT(1, &bufferID);

#if WENDY_DEBUG
  if (!checkGL("Error during image canvas creation"))
    return false;
#endif

  return true;
}

void ImageCanvas::apply(void) const
{
  if (!isCurrent())
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bufferID);

    GLenum enables[5];
    size_t count = 0;

    for (size_t i = 0;  i < sizeof(enables) / sizeof(enables[0]);  i++)
    {
      Attachment attachment = (Attachment) i;

      if (buffers[i] && isColorAttachment(attachment))
        enables[count++] = convertToGL(attachment);
    }

    if (count)
    {
      if (GLEW_VERSION_2_0)
        glDrawBuffers(count, enables);
      else
        glDrawBuffersARB(count, enables);
    }
    else
      glDrawBuffer(GL_NONE);

#if WENDY_DEBUG
    checkGL("Error when applying image canvas");
#endif

    Canvas::apply();
  }
}

///////////////////////////////////////////////////////////////////////

Stats::Stats(void):
  frameCount(0),
  frameRate(0.f)
{
  frames.push_back(Frame());

  timer.start();
}

void Stats::addFrame(void)
{
  frameCount++;

  frames.push_front(Frame());
  if (frames.size() > 60)
    frames.pop_back();

  frameRate = 0.f;
  const float factor = 1.f / frames.size();

  for (FrameQueue::const_iterator f = frames.begin();  f != frames.end();  f++)
    frameRate += (float) f->duration * factor;
}

void Stats::addPasses(unsigned int count)
{
  Frame& frame = frames.front();
  frame.passCount += count;
}

void Stats::addPrimitives(PrimitiveType type, unsigned int count)
{
  if (!count)
    return;

  Frame& frame = frames.front();
  frame.vertexCount += count;

  switch (type)
  {
    case POINT_LIST:
      frame.pointCount += count;
      break;
    case LINE_LIST:
      frame.lineCount += count / 2;
      break;
    case LINE_STRIP:
      frame.lineCount += count - 1;
      break;
    case TRIANGLE_LIST:
      frame.triangleCount += count / 3;
      break;
    case TRIANGLE_STRIP:
      frame.triangleCount += count - 2;
      break;
    case TRIANGLE_FAN:
      frame.triangleCount += count - 1;
      break;
    default:
      logError("Invalid primitive type %u", type);
  }
}

float Stats::getFrameRate(void) const
{
  return frameRate;
}

unsigned int Stats::getFrameCount(void) const
{
  return frameCount;
}

const Stats::Frame& Stats::getFrame(void) const
{
  return frames.front();
}

///////////////////////////////////////////////////////////////////////

Stats::Frame::Frame(void):
  passCount(0),
  vertexCount(0),
  pointCount(0),
  lineCount(0),
  triangleCount(0),
  duration(0.0)
{
}

///////////////////////////////////////////////////////////////////////

GlobalStateListener::~GlobalStateListener(void)
{
}

///////////////////////////////////////////////////////////////////////

GlobalUniform::GlobalUniform(const String& initName,
                             Uniform::Type initType,
                             unsigned int initID):
  name(initName),
  type(initType),
  ID(initID),
  listener(NULL)
{
}

void GlobalUniform::applyTo(Uniform& uniform) const
{
  if (listener)
    listener->onStateApply(ID, uniform);
  else
    logError("Global uniform \'%s\' has no listener", name.c_str());
}

const String& GlobalUniform::getName(void) const
{
  return name;
}

Uniform::Type GlobalUniform::getType(void) const
{
  return type;
}

unsigned int GlobalUniform::getID(void) const
{
  return ID;
}

GlobalStateListener* GlobalUniform::getListener(void) const
{
  return listener;
}

void GlobalUniform::setListener(GlobalStateListener* newListener)
{
  listener = newListener;
}

///////////////////////////////////////////////////////////////////////

GlobalSampler::GlobalSampler(const String& initName,
                             Sampler::Type initType,
                             unsigned int initID):
  name(initName),
  type(initType),
  ID(initID),
  listener(NULL)
{
}

void GlobalSampler::applyTo(Sampler& sampler) const
{
  if (listener)
    listener->onStateApply(ID, sampler);
  else
    logError("Global sampler \'%s\' has no listener", name.c_str());
}

const String& GlobalSampler::getName(void) const
{
  return name;
}

Sampler::Type GlobalSampler::getType(void) const
{
  return type;
}

unsigned int GlobalSampler::getID(void) const
{
  return ID;
}

GlobalStateListener* GlobalSampler::getListener(void) const
{
  return listener;
}

void GlobalSampler::setListener(GlobalStateListener* newListener)
{
  listener = newListener;
}

///////////////////////////////////////////////////////////////////////

Context::~Context(void)
{
  destroySignal.emit();

  currentCanvas = NULL;

  if (cgContextID)
  {
    cgDestroyContext((CGcontext) cgContextID);
    cgContextID = NULL;
  }

  glfwCloseWindow();

  instance = NULL;
}

void Context::clearColorBuffer(const ColorRGBA& color)
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

void Context::render(const PrimitiveRange& range)
{
  if (range.isEmpty())
  {
    logWarning("Rendering empty primitive range with shader program \'%s\'",
               currentProgram->getPath().asString().c_str());
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
    logError("Unable to render without a current shader program");
    return;
  }

  if (!currentVertexBuffer)
  {
    logError("Unable to render without a current vertex buffer");
    return;
  }

  currentProgram->apply();

  if (dirtyBinding)
  {
    const VertexFormat& format = currentVertexBuffer->getFormat();

    if (currentProgram->getVaryingCount() > format.getComponentCount())
    {
      logError("Shader program \'%s\' has more varying parameters than vertex format has components",
              currentProgram->getPath().asString().c_str());
      return;
    }

    for (size_t i = 0;  i < currentProgram->getVaryingCount();  i++)
    {
      Varying& varying = currentProgram->getVarying(i);

      const VertexComponent* component = format.findComponent(varying.getName());
      if (!component)
      {
        logError("Varying parameter \'%s\' of shader program \'%s\' has no corresponding vertex format component",
                varying.getName().c_str(),
                currentProgram->getPath().asString().c_str());
        return;
      }

      if (!isCompatible(varying, *component))
      {
        logError("Varying parameter \'%s\' of shader program \'%s\' has incompatible type",
                varying.getName().c_str(),
                currentProgram->getPath().asString().c_str());
        return;
      }

      varying.enable(format.getSize(), component->getOffset());
    }
  }

  if (currentIndexBuffer)
  {
    size_t size = IndexBuffer::getTypeSize(currentIndexBuffer->getType());

    glDrawElements(convertToGL(type),
                   count,
		   convertToGL(currentIndexBuffer->getType()),
		   (GLvoid*) (size * start));
  }
  else
    glDrawArrays(convertToGL(type), start, count);

  if (stats)
    stats->addPrimitives(type, count);
}

void Context::refresh(void)
{
  needsRefresh = true;
}

bool Context::update(void)
{
  glfwSwapBuffers();
  finishSignal.emit();
  needsRefresh = false;

  if (stats)
    stats->addFrame();

  if (refreshMode == MANUAL_REFRESH)
  {
    while (!needsRefresh && !needsClosing)
      glfwWaitEvents();
  }

  return !needsClosing;
}

GlobalUniform& Context::createGlobalUniform(const String& name,
                                            Uniform::Type type,
                                            unsigned int ID)
{
  GlobalUniform* state = findGlobalUniform(name, type);
  if (state)
  {
    if (state->getID() != ID)
      throw Exception("Global uniform internal ID mismatch");
  }
  else
  {
    state = new GlobalUniform(name, type, ID);
    globalUniforms.push_back(state);
  }

  return *state;
}

GlobalSampler& Context::createGlobalSampler(const String& name,
                                            Sampler::Type type,
                                            unsigned int ID)
{
  GlobalSampler* state = findGlobalSampler(name, type);
  if (state)
  {
    if (state->getID() != ID)
      throw Exception("Global sampler uniform internal ID mismatch");
  }
  else
  {
    state = new GlobalSampler(name, type, ID);
    globalSamplers.push_back(state);
  }

  return *state;
}

GlobalUniform* Context::findGlobalUniform(const String& name, Uniform::Type type) const
{
  for (UniformList::const_iterator u = globalUniforms.begin();  u != globalUniforms.end();  u++)
  {
    if ((*u)->getName() == name && (*u)->getType() == type)
      return *u;
  }

  return NULL;
}

GlobalSampler* Context::findGlobalSampler(const String& name, Sampler::Type type) const
{
  for (SamplerList::const_iterator s = globalSamplers.begin();  s != globalSamplers.end();  s++)
  {
    if ((*s)->getName() == name && (*s)->getType() == type)
      return *s;
  }

  return NULL;
}

Context::RefreshMode Context::getRefreshMode(void) const
{
  return refreshMode;
}

void Context::setRefreshMode(RefreshMode newMode)
{
  refreshMode = newMode;

  if (refreshMode == AUTOMATIC_REFRESH)
    glfwEnable(GLFW_AUTO_POLL_EVENTS);
  else
    glfwDisable(GLFW_AUTO_POLL_EVENTS);
}

const Rect& Context::getScissorArea(void) const
{
  return scissorArea;
}

const Rect& Context::getViewportArea(void) const
{
  return viewportArea;
}

void Context::setScissorArea(const Rect& newArea)
{
  scissorArea = newArea;
  updateScissorArea();
}

void Context::setViewportArea(const Rect& newArea)
{
  viewportArea = newArea;
  updateViewportArea();
}

Canvas& Context::getCurrentCanvas(void) const
{
  return *currentCanvas;
}

ScreenCanvas& Context::getScreenCanvas(void) const
{
  return *screenCanvas;
}

void Context::setScreenCanvasCurrent(void)
{
  setCurrentCanvas(*screenCanvas);
}

bool Context::setCurrentCanvas(Canvas& newCanvas)
{
  currentCanvas = &newCanvas;
  currentCanvas->apply();

#if WENDY_DEBUG
  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    logError("Image canvas is incomplete: %s", getFramebufferStatusMessage(status));
#endif

  updateViewportArea();
  updateScissorArea();

  return true;
}

Program* Context::getCurrentProgram(void) const
{
  return currentProgram;
}

void Context::setCurrentProgram(Program* newProgram)
{
  if (newProgram != currentProgram)
  {
    currentProgram = newProgram;
    dirtyBinding = true;
  }
}

VertexBuffer* Context::getCurrentVertexBuffer(void) const
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
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, currentVertexBuffer->bufferID);
    else
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

#if WENDY_DEBUG
    if (!checkGL("Failed to make index buffer current"))
      return;
#endif
  }
}

IndexBuffer* Context::getCurrentIndexBuffer(void) const
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
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, currentIndexBuffer->bufferID);
    else
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

#if WENDY_DEBUG
    if (!checkGL("Failed to apply index buffer"))
      return;
#endif
  }
}

const Context::PlaneList& Context::getClipPlanes(void) const
{
  return planes;
}

bool Context::setClipPlanes(const PlaneList& newPlanes)
{
  if (planes.size() > limits->getMaxClipPlanes())
    return false;

  planes = newPlanes;
  unsigned int index = 0;

  for (PlaneList::const_iterator p = planes.begin();  p != planes.end();  p++)
  {
    // TODO: Verify this.

    const double equation[4] = { p->normal.x, p->normal.y, p->normal.z, p->distance };

    glEnable(GL_CLIP_PLANE0 + index);
    glClipPlane(GL_CLIP_PLANE0 + index, equation);

    index++;
  }

  for ( ;  index < limits->getMaxClipPlanes();  index++)
    glDisable(GL_CLIP_PLANE0 + index);

#if WENDY_DEBUG
  checkGL("Error during user clip plane setup");
#endif

  return true;
}

const Mat4& Context::getModelMatrix(void) const
{
  return modelMatrix;
}

void Context::setModelMatrix(const Mat4& newMatrix)
{
  modelMatrix = newMatrix;
  dirtyModelView = dirtyModelViewProj = true;
}

const Mat4& Context::getViewMatrix(void) const
{
  return viewMatrix;
}

void Context::setViewMatrix(const Mat4& newMatrix)
{
  viewMatrix = newMatrix;
  dirtyModelView = dirtyViewProj = dirtyModelViewProj = true;
}

const Mat4& Context::getProjectionMatrix(void) const
{
  return projectionMatrix;
}

void Context::setProjectionMatrix(const Mat4& newMatrix)
{
  projectionMatrix = newMatrix;
  dirtyViewProj = dirtyModelViewProj = true;
}

void Context::setOrthoProjectionMatrix(float width, float height)
{
  projectionMatrix.setOrthoProjection(width, height);
  dirtyViewProj = dirtyModelViewProj = true;
}

void Context::setOrthoProjectionMatrix(const AABB& volume)
{
  projectionMatrix.setOrthoProjection(volume);
  dirtyViewProj = dirtyModelViewProj = true;
}

void Context::setPerspectiveProjectionMatrix(float FOV, float aspect, float nearZ, float farZ)
{
  if (aspect == 0.f)
  {
    aspect = (currentCanvas->getWidth() * viewportArea.size.x) /
             (currentCanvas->getHeight() * viewportArea.size.y);
  }

  projectionMatrix.setPerspectiveProjection(FOV, aspect, nearZ, farZ);
  dirtyViewProj = dirtyModelViewProj = true;
}

Stats* Context::getStats(void) const
{
  return stats;
}

void Context::setStats(Stats* newStats)
{
  stats = newStats;
}

const String& Context::getTitle(void) const
{
  return title;
}

void Context::setTitle(const String& newTitle)
{
  glfwSetWindowTitle(newTitle.c_str());
  title = newTitle;
}

ResourceIndex& Context::getIndex(void) const
{
  return index;
}

const Limits& Context::getLimits(void) const
{
  return *limits;
}

SignalProxy0<void> Context::getFinishSignal(void)
{
  return finishSignal;
}

SignalProxy0<bool> Context::getCloseRequestSignal(void)
{
  return closeRequestSignal;
}

SignalProxy2<void, unsigned int, unsigned int> Context::getResizedSignal(void)
{
  return resizedSignal;
}

bool Context::createSingleton(ResourceIndex& index, const ContextMode& mode)
{
  Ptr<Context> context(new Context(index));
  if (!context->init(mode))
    return false;

  set(context.detachObject());
  createSignal.emit();
  return true;
}

SignalProxy0<void> Context::getCreateSignal(void)
{
  return createSignal;
}

SignalProxy0<void> Context::getDestroySignal(void)
{
  return destroySignal;
}

void Context::getScreenModes(ScreenModeList& result)
{
  GLFWvidmode modes[256];

  const size_t count = glfwGetVideoModes(modes, sizeof(modes) / sizeof(GLFWvidmode));

  for (size_t i = 0;  i < count;  i++)
  {
    result.push_back(ScreenMode(modes[i].Width,
                                modes[i].Height,
				modes[i].RedBits + modes[i].GreenBits + modes[i].BlueBits));
  }
}

Context::Context(ResourceIndex& initIndex):
  index(initIndex),
  cgContextID(NULL),
  cgVertexProfile(CG_PROFILE_UNKNOWN),
  cgFragmentProfile(CG_PROFILE_UNKNOWN),
  refreshMode(AUTOMATIC_REFRESH),
  needsRefresh(false),
  needsClosing(false),
  dirtyModelView(true),
  dirtyViewProj(true),
  dirtyModelViewProj(true),
  dirtyBinding(true),
  stats(NULL)
{
  // Necessary hack in case GLFW calls a callback before
  // we have had time to call Singleton::set.

  // TODO: Remove this upon the arrival of GLFW_USER_POINTER.

  instance = this;
}

Context::Context(const Context& source):
  index(source.index)
{
  // NOTE: Not implemented.
}

Context& Context::operator = (const Context& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool Context::init(const ContextMode& initMode)
{
  scissorArea.set(0.f, 0.f, 1.f, 1.f);
  viewportArea.set(0.f, 0.f, 1.f, 1.f);

  // Create context and window
  {
    unsigned int colorBits = initMode.colorBits;
    if (colorBits > 24)
      colorBits = 24;

    unsigned int mode;

    if (initMode.mode == WINDOWED)
      mode = GLFW_WINDOW;
    else
      mode = GLFW_FULLSCREEN;

    if (initMode.samples)
      glfwOpenWindowHint(GLFW_FSAA_SAMPLES, initMode.samples);

    if (!glfwOpenWindow(initMode.width, initMode.height,
                        colorBits / 3, colorBits / 3, colorBits / 3, 0,
                        initMode.depthBits, initMode.stencilBits, mode))
    {
      logError("Unable to create GLFW window");
      return false;
    }
  }

  // Initialize GLEW and check extensions
  {
    if (glewInit() != GLEW_OK)
    {
      logError("Unable to initialize GLEW");
      return false;
    }

    if (!GLEW_ARB_vertex_buffer_object)
    {
      logError("Vertex buffer objects (ARB_vertex_buffer_object) are required but not supported");
      return false;
    }

    if (!GLEW_ARB_texture_cube_map)
    {
      logError("Cube map textures (ARB_texture_cube_map) are required but not supported");
      return false;
    }

    if (!GLEW_ARB_texture_rectangle && !GLEW_EXT_texture_rectangle)
    {
      logError("Rectangular textures ({ARB|EXT}_texture_rectangle) are required but not supported");
      return false;
    }

    if (!GLEW_VERSION_2_0 && !GLEW_ARB_draw_buffers)
    {
      logError("Draw buffers are required but not supported");
      return false;
    }

    if (!GLEW_EXT_framebuffer_object)
    {
      logError("Framebuffer objects (EXT_framebuffer_object) are required but not supported");
      return false;
    }
  }

  // All extensions are there; figure out their limits
  limits = new Limits(*this);

  // Initialize Cg context and profiles
  {
    cgContextID = cgCreateContext();
    if (!cgContextID)
    {
      logError("Unable to create Cg context: %s", cgGetErrorString(cgGetError()));
      return false;
    }

    cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    if (cgVertexProfile == CG_PROFILE_UNKNOWN)
    {
      logError("Unable to find any usable Cg vertex profile");
      return false;
    }

    log("Cg vertex profile %s selected",
        cgGetProfileString((CGprofile) cgVertexProfile));

    cgGLEnableProfile((CGprofile) cgVertexProfile);
    cgGLSetOptimalOptions((CGprofile) cgVertexProfile);

    if (!checkCg("Failed to set up Cg vertex profile"))
      return false;

    cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    if (cgFragmentProfile == CG_PROFILE_UNKNOWN)
    {
      logError("Unable to find any usable Cg fragment profile");
      return false;
    }

    log("Cg fragment profile %s selected",
        cgGetProfileString((CGprofile) cgFragmentProfile));

    cgGLEnableProfile((CGprofile) cgFragmentProfile);
    cgGLSetOptimalOptions((CGprofile) cgFragmentProfile);

    if (!checkCg("Failed to set up Cg fragment profile"))
      return false;

    cgGLSetManageTextureParameters((CGcontext) cgContextID, CG_TRUE);
    cgSetLockingPolicy(CG_NO_LOCKS_POLICY);
    cgSetParameterSettingMode((CGcontext) cgContextID, CG_IMMEDIATE_PARAMETER_SETTING);

#if WENDY_DEBUG
    cgGLSetDebugMode(CG_TRUE);
#else
    cgGLSetDebugMode(CG_FALSE);
#endif

    if (!checkCg("Failed to set Cg options"))
      return false;
  }

  // Create and apply screen canvas
  {
    int width, height;
    glfwGetWindowSize(&width, &height);

    // This needs to be done before setting the window size callback
    screenCanvas = new ScreenCanvas(*this);
    screenCanvas->mode.width = width;
    screenCanvas->mode.height = height;

    // Read back actual (as opposed to desired) framebuffer properties
    screenCanvas->mode.colorBits = glfwGetWindowParam(GLFW_RED_BITS) +
                                   glfwGetWindowParam(GLFW_GREEN_BITS) +
                                   glfwGetWindowParam(GLFW_BLUE_BITS);
    screenCanvas->mode.depthBits = glfwGetWindowParam(GLFW_DEPTH_BITS);
    screenCanvas->mode.stencilBits = glfwGetWindowParam(GLFW_STENCIL_BITS);
    screenCanvas->mode.samples = glfwGetWindowParam(GLFW_FSAA_SAMPLES);
    screenCanvas->mode.mode = initMode.mode;

    setScreenCanvasCurrent();
  }

  // Finish GLFW init
  {
    setTitle("Wendy");
    glfwPollEvents();

    glfwSetWindowSizeCallback(sizeCallback);
    glfwSetWindowCloseCallback(closeCallback);
    glfwSetWindowRefreshCallback(refreshCallback);

    glfwSwapInterval(1);
  }

  createGlobalUniform("M", Uniform::FLOAT_MAT4, STATE_MODEL_MATRIX).setListener(this);
  createGlobalUniform("V", Uniform::FLOAT_MAT4, STATE_VIEW_MATRIX).setListener(this);
  createGlobalUniform("P", Uniform::FLOAT_MAT4, STATE_PROJECTION_MATRIX).setListener(this);
  createGlobalUniform("MV", Uniform::FLOAT_MAT4, STATE_MODELVIEW_MATRIX).setListener(this);
  createGlobalUniform("VP", Uniform::FLOAT_MAT4, STATE_VIEWPROJECTION_MATRIX).setListener(this);
  createGlobalUniform("MVP", Uniform::FLOAT_MAT4, STATE_MODELVIEWPROJECTION_MATRIX).setListener(this);

  return true;
}

void Context::updateScissorArea(void)
{
  if (scissorArea == Rect(0.f, 0.f, 1.f, 1.f))
    glDisable(GL_SCISSOR_TEST);
  else
  {
    const unsigned int width = currentCanvas->getWidth();
    const unsigned int height = currentCanvas->getHeight();

    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint) floorf(scissorArea.position.x * width),
	      (GLint) floorf(scissorArea.position.y * height),
	      (GLsizei) ceilf(scissorArea.size.x * width),
	      (GLsizei) ceilf(scissorArea.size.y * height));
  }
}

void Context::updateViewportArea(void)
{
  const unsigned int width = currentCanvas->getWidth();
  const unsigned int height = currentCanvas->getHeight();

  glViewport((GLint) (viewportArea.position.x * width),
             (GLint) (viewportArea.position.y * height),
	     (GLsizei) (viewportArea.size.x * width),
	     (GLsizei) (viewportArea.size.y * height));
}

void Context::sizeCallback(int width, int height)
{
  instance->screenCanvas->mode.width = width;
  instance->screenCanvas->mode.height = height;

  if (instance->currentCanvas == instance->screenCanvas)
  {
    instance->updateViewportArea();
    instance->updateScissorArea();
  }

  instance->resizedSignal.emit(width, height);
}

int Context::closeCallback(void)
{
  std::vector<bool> results;

  instance->closeRequestSignal.emit(results);

  if (std::find(results.begin(), results.end(), false) == results.end())
    instance->needsClosing = true;

  return GL_TRUE;
}

void Context::refreshCallback(void)
{
  instance->needsRefresh = true;
}

void Context::onStateApply(unsigned int stateID, Uniform& uniform)
{
  switch (stateID)
  {
    case STATE_MODEL_MATRIX:
    {
      uniform.setValue(modelMatrix);
      break;
    }

    case STATE_VIEW_MATRIX:
    {
      uniform.setValue(viewMatrix);
      break;
    }

    case STATE_PROJECTION_MATRIX:
    {
      uniform.setValue(projectionMatrix);
      break;
    }

    case STATE_MODELVIEW_MATRIX:
    {
      if (dirtyModelView)
      {
        modelViewMatrix = viewMatrix;
        modelViewMatrix *= modelMatrix;
        dirtyModelView = false;
      }

      uniform.setValue(modelViewMatrix);
      break;
    }

    case STATE_VIEWPROJECTION_MATRIX:
    {
      if (dirtyViewProj)
      {
        viewProjMatrix = projectionMatrix;
        viewProjMatrix *= viewMatrix;
        dirtyViewProj = false;
      }

      uniform.setValue(viewProjMatrix);
      break;
    }

    case STATE_MODELVIEWPROJECTION_MATRIX:
    {
      if (dirtyModelViewProj)
      {
        if (dirtyViewProj)
        {
          viewProjMatrix = projectionMatrix;
          viewProjMatrix *= viewMatrix;
          dirtyViewProj = false;
        }

        modelViewProjMatrix = viewProjMatrix;
        modelViewProjMatrix *= modelMatrix;
        dirtyModelViewProj = false;
      }

      uniform.setValue(modelViewProjMatrix);
      break;
    }
  }
}

void Context::onStateApply(unsigned int stateID, Sampler& sampler)
{
}

Context* Context::instance = NULL;

Signal0<void> Context::createSignal;
Signal0<void> Context::destroySignal;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

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

#include <wendy/GLImage.h>
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

  Log::writeError("Unknown OpenGL framebuffer status %u", status);
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

  Log::writeError("Invalid image canvas attachment %u", attachment);
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

  Log::writeError("Invalid image canvas attachment %u", attachment);
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

  Log::writeError("Invalid primitive type %u", type);
  return 0;
}

bool compatible(const Varying& varying, const VertexComponent& component)
{
  switch (varying.getType())
  {
    case Varying::FLOAT:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 1)
        return true;

      break;
    }

    case Varying::FLOAT_VEC2:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 2)
        return true;

      break;
    }

    case Varying::FLOAT_VEC3:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 3)
        return true;

      break;
    }

    case Varying::FLOAT_VEC4:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 4)
        return true;

      break;
    }
  }

  return false;
}

void requestModelMatrix(Uniform& uniform)
{
  Context* context = Context::get();
  uniform.setValue(context->getModelMatrix());
}

void requestViewMatrix(Uniform& uniform)
{
  Context* context = Context::get();
  uniform.setValue(context->getViewMatrix());
}

void requestProjectionMatrix(Uniform& uniform)
{
  Context* context = Context::get();
  uniform.setValue(context->getProjectionMatrix());
}

void requestModelViewMatrix(Uniform& uniform)
{
  Context* context = Context::get();
  Mat4 value = context->getViewMatrix();
  value *= context->getModelMatrix();
  uniform.setValue(value);
}

void requestViewProjectionMatrix(Uniform& uniform)
{
  Context* context = Context::get();
  Mat4 value = context->getProjectionMatrix();
  value *= context->getViewMatrix();
  uniform.setValue(value);
}

void requestModelViewProjectionMatrix(Uniform& uniform)
{
  Context* context = Context::get();
  Mat4 value = context->getProjectionMatrix();
  value *= context->getViewMatrix();
  value *= context->getModelMatrix();
  uniform.setValue(value);
}

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
			 unsigned int initFlags):
  ScreenMode(width, height, colorBits),
  depthBits(initDepthBits),
  stencilBits(initStencilBits),
  samples(initSamples),
  flags(initFlags)
{
}

void ContextMode::setDefaults(void)
{
  set(640, 480, 32, 32, 0, DEFAULT);
}

void ContextMode::set(unsigned int width,
                      unsigned int height,
		      unsigned int colorBits,
		      unsigned int newDepthBits,
		      unsigned int newStencilBits,
		      unsigned int newSamples,
		      unsigned int newFlags)
{
  ScreenMode::set(width, height, colorBits);

  depthBits = newDepthBits;
  stencilBits = newStencilBits;
  samples = newSamples;
  flags = newFlags;
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
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying screen canvas: %s", gluErrorString(error));
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
      Log::writeError("Specified %s image object does not match canvas dimensions",
                      getAttachmentName(attachment));
      return false;
    }
  }

  if (isColorAttachment(attachment))
  {
    unsigned int index = attachment - COLOR_BUFFER0;

    if (index >= context.getLimits().getMaxColorAttachments())
    {
      Log::writeError("OpenGL context supports at most %u FBO color attachments",
                      context.getLimits().getMaxColorAttachments());
      return false;
    }

    if (index >= context.getLimits().getMaxDrawBuffers())
    {
      Log::writeError("OpenGL context supports at most %u draw buffers",
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

ImageCanvas* ImageCanvas::createInstance(Context& context, unsigned int width, unsigned int height)
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
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during image canvas creation: %s", gluErrorString(error));
    return false;
  }
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
      glDrawBuffersARB(count, enables);
    else
      glDrawBuffer(GL_NONE);

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying image canvas: %s", gluErrorString(error));
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

  for (unsigned int i = 0;  i < frames.size();  i++)
    frameRate += (float) frames[i].duration / frames.size();
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
      Log::writeError("Invalid primitive type %u", type);
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
}

void Context::clearDepthBuffer(float depth)
{
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_TRUE);
  glClearDepth(depth);
  glClear(GL_DEPTH_BUFFER_BIT);
  glPopAttrib();
}

void Context::clearStencilBuffer(unsigned int value)
{
  glPushAttrib(GL_STENCIL_BUFFER_BIT);
  glStencilMask(GL_TRUE);
  glClearStencil(value);
  glClear(GL_STENCIL_BUFFER_BIT);
  glPopAttrib();
}

void Context::render(const PrimitiveRange& range)
{
  if (range.isEmpty())
  {
    Log::writeWarning("Rendering empty primitive range");
    return;
  }

  if (!currentProgram)
  {
    Log::writeError("Unable to render without a current shader program");
    return;
  }

  // TODO: Optimize this method.

  Program& program = *currentProgram;
  program.apply();

  const VertexBuffer& vertexBuffer = *(range.getVertexBuffer());
  vertexBuffer.apply();

  const IndexBuffer* indexBuffer = range.getIndexBuffer();
  if (indexBuffer)
    indexBuffer->apply();

  const VertexFormat& format = vertexBuffer.getFormat();

  if (program.getVaryingCount() > format.getComponentCount())
  {
    Log::writeError("Shader program \'%s\' has more varying parameters than vertex format has components",
                    program.getName().c_str());
    return;
  }

  for (unsigned int i = 0;  i < program.getVaryingCount();  i++)
  {
    Varying& varying = program.getVarying(i);

    const VertexComponent* component = format.findComponent(varying.getName());
    if (!component)
    {
      Log::writeError("Varying parameter \'%s\' of shader program \'%s\' has no corresponding vertex format component",
                      varying.getName().c_str(),
                      program.getName().c_str());
      return;
    }

    if (!compatible(varying, *component))
    {
      Log::writeError("Varying parameter \'%s\' of shader program \'%s\' has incompatible type",
                      varying.getName().c_str(),
                      program.getName().c_str());
      return;
    }

    varying.enable(format.getSize(), component->getOffset());
  }

  for (UniformList::const_iterator u = reservedUniforms.begin();  u != reservedUniforms.end();  u++)
  {
    if (Uniform* uniform = program.findUniform(u->name))
    {
      if (uniform->getType() == u->type)
        u->signal.emit(*uniform);
    }
  }

  for (SamplerList::const_iterator s = reservedSamplers.begin();  s != reservedSamplers.end();  s++)
  {
    if (Sampler* sampler = program.findSampler(s->name))
    {
      if (sampler->getType() == s->type)
        s->signal.emit(*sampler);
    }
  }

  if (indexBuffer)
  {
    glDrawElements(convertToGL(range.getType()),
                   range.getCount(),
		   convertToGL(indexBuffer->getType()),
		   (GLvoid*) (IndexBuffer::getTypeSize(indexBuffer->getType()) * range.getStart()));
  }
  else
  {
    glDrawArrays(convertToGL(range.getType()),
                 range.getStart(),
		 range.getCount());
  }

  if (stats)
    stats->addPrimitives(range.getType(), range.getCount());

  for (unsigned int i = 0;  i < program.getVaryingCount();  i++)
    program.getVarying(i).disable();
}

bool Context::update(void)
{
  glfwSwapBuffers();

  finishSignal.emit();

  if (stats)
    stats->addFrame();

  return glfwGetWindowParam(GLFW_OPENED) == GL_TRUE;
}

SignalProxy1<void, Uniform&> Context::reserveUniform(const String& name, Uniform::Type type)
{
  if (isReservedUniform(name))
    throw Exception("Uniform already reserved");

  reservedUniforms.push_back(ReservedUniform());

  ReservedUniform& slot = reservedUniforms.back();
  slot.name = name;
  slot.type = type;

  return slot.signal;
}

SignalProxy1<void, Sampler&> Context::reserveSampler(const String& name, Sampler::Type type)
{
  if (isReservedSampler(name))
    throw Exception("Sampler already reserved");

  reservedSamplers.push_back(ReservedSampler());

  ReservedSampler& slot = reservedSamplers.back();
  slot.name = name;
  slot.type = type;

  return slot.signal;
}

bool Context::isReservedUniform(const String& name) const
{
  for (UniformList::const_iterator u = reservedUniforms.begin();  u != reservedUniforms.end();  u++)
  {
    if (u->name == name)
      return true;
  }

  return false;
}

bool Context::isReservedSampler(const String& name) const
{
  for (SamplerList::const_iterator s = reservedSamplers.begin();  s != reservedSamplers.end();  s++)
  {
    if (s->name == name)
      return true;
  }

  return false;
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
    Log::writeError("Image canvas is incomplete: %s", getFramebufferStatusMessage(status));
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
  currentProgram = newProgram;
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

    const double equation[4] = { (*p).normal.x, (*p).normal.y, (*p).normal.z, (*p).distance };

    glEnable(GL_CLIP_PLANE0 + index);
    glClipPlane(GL_CLIP_PLANE0 + index, equation);

    index++;
  }

  for ( ;  index < limits->getMaxClipPlanes();  index++)
  {
    glDisable(GL_CLIP_PLANE0 + index);
  }

  return true;
}

const Mat4& Context::getModelMatrix(void) const
{
  return modelMatrix;
}

void Context::setModelMatrix(const Mat4& newMatrix)
{
  modelMatrix = newMatrix;
}

const Mat4& Context::getViewMatrix(void) const
{
  return viewMatrix;
}

void Context::setViewMatrix(const Mat4& newMatrix)
{
  viewMatrix = newMatrix;
}

const Mat4& Context::getProjectionMatrix(void) const
{
  return projectionMatrix;
}

void Context::setProjectionMatrix(const Mat4& newMatrix)
{
  projectionMatrix = newMatrix;
}

void Context::setProjectionMatrix2D(float width, float height)
{
  projectionMatrix.setProjection2D(width, height);
}

void Context::setProjectionMatrix3D(float FOV, float aspect, float nearZ, float farZ)
{
  if (aspect == 0.f)
  {
    aspect = (currentCanvas->getWidth() * viewportArea.size.x) /
             (currentCanvas->getHeight() * viewportArea.size.y);
  }

  projectionMatrix.setProjection3D(FOV, aspect, nearZ, farZ);
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

bool Context::create(const ContextMode& mode)
{
  Ptr<Context> context(new Context());
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

Context::Context(void):
  cgContextID(NULL),
  cgVertexProfile(CG_PROFILE_UNKNOWN),
  cgFragmentProfile(CG_PROFILE_UNKNOWN),
  currentProgram(NULL),
  stats(NULL)
{
  // Necessary hack in case GLFW calls a callback before
  // we have had time to call Singleton::set.

  // TODO: Remove this upon the arrival of GLFW_USER_POINTER.

  instance = this;
}

Context::Context(const Context& source)
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

    unsigned int flags;

    if (initMode.flags & ContextMode::WINDOWED)
      flags = GLFW_WINDOW;
    else
      flags = GLFW_FULLSCREEN;

    if (initMode.samples)
      glfwOpenWindowHint(GLFW_FSAA_SAMPLES, initMode.samples);

    if (!glfwOpenWindow(initMode.width, initMode.height,
                        colorBits / 3, colorBits / 3, colorBits / 3, 0,
                        initMode.depthBits, initMode.stencilBits, flags))
    {
      Log::writeError("Unable to create GLFW window");
      return false;
    }
  }

  // Initialize GLEW and check extensions
  {
    if (glewInit() != GLEW_OK)
    {
      Log::writeError("Unable to initialize GLEW");
      return false;
    }

    if (!GLEW_ARB_vertex_buffer_object)
    {
      Log::writeError("Vertex buffer objects (ARB_vertex_buffer_object) are required but not supported");
      return false;
    }

    if (!GLEW_ARB_texture_cube_map)
    {
      Log::writeError("Cube map textures (ARB_texture_cube_map) are required but not supported");
      return false;
    }

    if (!GLEW_ARB_texture_rectangle && !GLEW_EXT_texture_rectangle)
    {
      Log::writeError("Rectangular textures ({ARB|EXT}_texture_rectangle) are required but not supported");
      return false;
    }

    if (!GLEW_ARB_draw_buffers)
    {
      Log::writeError("Draw buffers (ARB_draw_buffers) are required but not supported");
      return false;
    }

    if (!GLEW_EXT_framebuffer_object)
    {
      Log::writeError("Framebuffer objects (EXT_framebuffer_object) are required but not supported");
      return false;
    }
  }

  // All extensions are there; figure out their limits
  limits = new Limits(*this);

  // Initialize Cg context and profiles
  {
    CGerror error;

    cgContextID = cgCreateContext();
    if (!cgContextID)
    {
      Log::writeError("Unable to create Cg context: %s", cgGetErrorString(cgGetError()));
      return false;
    }

    cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    if (cgVertexProfile == CG_PROFILE_UNKNOWN)
    {
      Log::writeError("Unable to find any usable Cg vertex profile");
      return false;
    }

    Log::write("Cg vertex profile %s selected",
               cgGetProfileString((CGprofile) cgVertexProfile));

    cgGLEnableProfile((CGprofile) cgVertexProfile);
    cgGLSetOptimalOptions((CGprofile) cgVertexProfile);

    error = cgGetError();
    if (error != CG_NO_ERROR)
    {
      Log::writeError("Failed to set up Cg vertex profile: %s", cgGetErrorString(error));
      return false;
    }

    cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    if (cgFragmentProfile == CG_PROFILE_UNKNOWN)
    {
      Log::writeError("Unable to find any usable Cg fragment profile");
      return false;
    }

    Log::write("Cg fragment profile %s selected",
               cgGetProfileString((CGprofile) cgFragmentProfile));

    cgGLEnableProfile((CGprofile) cgFragmentProfile);
    cgGLSetOptimalOptions((CGprofile) cgFragmentProfile);

    error = cgGetError();
    if (error != CG_NO_ERROR)
    {
      Log::writeError("Failed to set up Cg fragment profile: %s", cgGetErrorString(error));
      return false;
    }

    cgGLSetManageTextureParameters((CGcontext) cgContextID, CG_TRUE);
    cgSetLockingPolicy(CG_NO_LOCKS_POLICY);
    cgSetParameterSettingMode((CGcontext) cgContextID, CG_IMMEDIATE_PARAMETER_SETTING);

#if WENDY_DEBUG
    cgGLSetDebugMode(CG_TRUE);
#else
    cgGLSetDebugMode(CG_FALSE);
#endif

    error = cgGetError();
    if (error != CG_NO_ERROR)
    {
      Log::writeError("Failed to set Cg options: %s", cgGetErrorString(error));
      return false;
    }
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
    screenCanvas->mode.flags = initMode.flags;

    setScreenCanvasCurrent();
  }

  // Finish GLFW init
  {
    setTitle("Wendy");
    glfwPollEvents();

    glfwSetWindowSizeCallback(sizeCallback);
    glfwSetWindowCloseCallback(closeCallback);

    glfwSwapInterval(1);
  }

  reserveUniform("M", Uniform::FLOAT_MAT4).connect(requestModelMatrix);
  reserveUniform("V", Uniform::FLOAT_MAT4).connect(requestViewMatrix);
  reserveUniform("P", Uniform::FLOAT_MAT4).connect(requestProjectionMatrix);
  reserveUniform("MV", Uniform::FLOAT_MAT4).connect(requestModelViewMatrix);
  reserveUniform("VP", Uniform::FLOAT_MAT4).connect(requestViewProjectionMatrix);
  reserveUniform("MVP", Uniform::FLOAT_MAT4).connect(requestModelViewProjectionMatrix);

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
  typedef std::list<bool> ResultList;
  ResultList results;

  instance->closeRequestSignal.emit(results);

  if (std::find(results.begin(), results.end(), false) == results.end())
    return 1;

  return 0;
}

Context* Context::instance = NULL;

Signal0<void> Context::createSignal;
Signal0<void> Context::destroySignal;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

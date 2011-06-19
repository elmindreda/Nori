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
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

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

const char* asString(ImageCanvas::Attachment attachment)
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

bool isCompatible(const Attribute& attribute, const VertexComponent& component)
{
  switch (attribute.getType())
  {
    case Attribute::FLOAT:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 1)
        return true;

      break;
    }

    case Attribute::VEC2:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 2)
        return true;

      break;
    }

    case Attribute::VEC3:
    {
      if (component.getType() == VertexComponent::FLOAT32 &&
          component.getElementCount() == 3)
        return true;

      break;
    }

    case Attribute::VEC4:
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
  context(initContext)
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
}

unsigned int Limits::getMaxColorAttachments(void) const
{
  return maxColorAttachments;
}

unsigned int Limits::getMaxDrawBuffers(void) const
{
  return maxDrawBuffers;
}

unsigned int Limits::getMaxVertexTextureImageUnits(void) const
{
  return maxVertexTextureImageUnits;
}

unsigned int Limits::getMaxFragmentTextureImageUnits(void) const
{
  return maxFragmentTextureImageUnits;
}

unsigned int Limits::getMaxCombinedTextureImageUnits(void) const
{
  return maxCombinedTextureImageUnits;
}

unsigned int Limits::getMaxTextureSize(void) const
{
  return maxTextureSize;
}

unsigned int Limits::getMaxTexture3DSize(void) const
{
  return maxTexture3DSize;
}

unsigned int Limits::getMaxTextureCubeSize(void) const
{
  return maxTextureCubeSize;
}

unsigned int Limits::getMaxTextureRectangleSize(void) const
{
  return maxTextureRectangleSize;
}

unsigned int Limits::getMaxTextureCoords(void) const
{
  return maxTextureCoords;
}

unsigned int Limits::getMaxVertexAttributes(void) const
{
  return maxVertexAttributes;
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

bool ImageCanvas::setBuffer(Attachment attachment, Image* newImage, unsigned int z)
{
  if (newImage)
  {
    if (newImage->getWidth() != width || newImage->getHeight() != height)
    {
      logError("Specified %s image object does not match canvas dimensions",
               asString(attachment));
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
    buffers[attachment]->attach(convertToGL(attachment), z);

  previous->apply();
  return true;
}

ImageCanvas* ImageCanvas::create(Context& context,
                                 unsigned int width,
                                 unsigned int height)
{
  Ptr<ImageCanvas> canvas(new ImageCanvas(context));
  if (!canvas->init(width, height))
    return NULL;

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
      glDrawBuffers(count, enables);
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

SharedSampler::SharedSampler(const String& initName,
                             Sampler::Type initType,
                             int initID):
  name(initName),
  type(initType),
  ID(initID)
{
}

///////////////////////////////////////////////////////////////////////

SharedUniform::SharedUniform(const String& initName,
                             Uniform::Type initType,
                             int initID):
  name(initName),
  type(initType),
  ID(initID)
{
}

///////////////////////////////////////////////////////////////////////

Context::~Context(void)
{
  destroySignal.emit();

  setScreenCanvasCurrent();
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

  if (dirtyBinding)
  {
    const VertexFormat& format = currentVertexBuffer->getFormat();

    if (currentProgram->getAttributeCount() > format.getComponentCount())
    {
      logError("Shader program \'%s\' has more attributes than vertex format has components",
               currentProgram->getPath().asString().c_str());
      return;
    }

    for (size_t i = 0;  i < currentProgram->getAttributeCount();  i++)
    {
      Attribute& attribute = currentProgram->getAttribute(i);

      const VertexComponent* component = format.findComponent(attribute.getName());
      if (!component)
      {
        logError("Attribute \'%s\' of program \'%s\' has no corresponding vertex format component",
                 attribute.getName().c_str(),
                 currentProgram->getPath().asString().c_str());
        return;
      }

      if (!isCompatible(attribute, *component))
      {
        logError("Attribute \'%s\' of shader program \'%s\' has incompatible type",
                 attribute.getName().c_str(),
                 currentProgram->getPath().asString().c_str());
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

void Context::requestClose(void)
{
  closeCallback();
}

void Context::createSharedSampler(const String& name, Sampler::Type type, int ID)
{
  if (ID == INVALID_SHARED_STATE_ID)
  {
    logError("Cannot create shared sampler with invalid ID");
    return;
  }

  if (getSharedSamplerID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  declaration.append("uniform ");
  declaration.append(Sampler::getTypeName(type));
  declaration.append(" ");
  declaration.append(name);
  declaration.append(";\n");

  samplers.push_back(SharedSampler(name, type, ID));
}

void Context::createSharedUniform(const String& name, Uniform::Type type, int ID)
{
  if (ID == INVALID_SHARED_STATE_ID)
  {
    logError("Cannot create shared uniform with invalid ID");
    return;
  }

  if (getSharedUniformID(name, type) != INVALID_SHARED_STATE_ID)
    return;

  declaration.append("uniform ");
  declaration.append(Uniform::getTypeName(type));
  declaration.append(" ");
  declaration.append(name);
  declaration.append(";\n");

  uniforms.push_back(SharedUniform(name, type, ID));
}

int Context::getSharedSamplerID(const String& name, Sampler::Type type) const
{
  for (SamplerList::const_iterator s = samplers.begin(); s != samplers.end(); s++)
  {
    if (s->name == name && s->type == type)
      return s->ID;
  }

  return INVALID_SHARED_STATE_ID;
}

int Context::getSharedUniformID(const String& name, Uniform::Type type) const
{
  for (UniformList::const_iterator u = uniforms.begin(); u != uniforms.end(); u++)
  {
    if (u->name == name && u->type == type)
      return u->ID;
  }

  return INVALID_SHARED_STATE_ID;
}

SharedProgramState* Context::getCurrentSharedProgramState(void) const
{
  return currentState;
}

void Context::setCurrentSharedProgramState(SharedProgramState* newState)
{
  currentState = newState;
}

const String& Context::getSharedProgramStateDeclaration(void) const
{
  return declaration;
}

Context::RefreshMode Context::getRefreshMode(void) const
{
  return refreshMode;
}

void Context::setRefreshMode(RefreshMode newMode)
{
  refreshMode = newMode;
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
  if (currentCanvas != screenCanvas)
  {
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status == 0)
      checkGL("Framebuffer status check failed");
    else if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
      logError("Image canvas is incomplete: %s", getFramebufferStatusMessage(status));
  }
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
      glBindBuffer(GL_ARRAY_BUFFER, currentVertexBuffer->bufferID);
    else
      glBindBuffer(GL_ARRAY_BUFFER, 0);

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
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentIndexBuffer->bufferID);
    else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#if WENDY_DEBUG
    if (!checkGL("Failed to apply index buffer"))
      return;
#endif
  }
}

Texture* Context::getCurrentTexture(void) const
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
                    oldTexture->getPath().asString().c_str()))
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
                  newTexture->getPath().asString().c_str()))
      {
        return;
      }
#endif
    }

    textureUnits[activeTextureUnit] = newTexture;
  }
}

unsigned int Context::getActiveTextureUnit(void) const
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

    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

    if (!glfwOpenWindow(initMode.width, initMode.height,
                        colorBits / 3, colorBits / 3, colorBits / 3, 0,
                        initMode.depthBits, initMode.stencilBits, mode))
    {
      logError("Unable to create GLFW window");
      return false;
    }

    log("OpenGL context version %i.%i created",
        glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR),
        glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR));

    log("OpenGL context GLSL version is %s",
        (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

    log("OpenGL context renderer is %s by %s",
        (const char*) glGetString(GL_RENDERER),
        (const char*) glGetString(GL_VENDOR));
  }

  // Initialize GLEW and check extensions
  {
    if (glewInit() != GLEW_OK)
    {
      logError("Unable to initialize GLEW");
      return false;
    }

    if (!GLEW_ARB_texture_rectangle)
    {
      logError("Rectangular textures (ARB_texture_rectangle) is required but not supported");
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

  // Set up texture unit cache
  {
    unsigned int unitCount = max(limits->getMaxCombinedTextureImageUnits(),
                                 limits->getMaxTextureCoords());

    textureUnits.resize(unitCount);
  }

  // Apply default differences
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
    glfwDisable(GLFW_AUTO_POLL_EVENTS);

    glfwSwapInterval(1);
  }

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

Context* Context::instance = NULL;

Signal0<void> Context::createSignal;
Signal0<void> Context::destroySignal;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

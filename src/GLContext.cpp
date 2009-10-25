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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GL/glfw.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#if MOIRA_HAVE_CTYPE_H
#include <ctype.h>
#endif

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

GLint getIntegerParameter(GLenum parameter)
{
  GLint value;
  glGetIntegerv(parameter, &value);
  return value;
}

}

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
  set(640, 480, 0, 0, 0, DEFAULT);
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
  maxClipPlanes(0),
  maxFragmentTextureImageUnits(0),
  maxVertexTextureImageUnits(0),
  maxTextureSize(0),
  maxTextureCubeSize(0),
  maxTextureRectangleSize(0),
  maxVertexAttributes(0)
{
  maxClipPlanes = getIntegerParameter(GL_MAX_CLIP_PLANES);
  maxFragmentTextureImageUnits = getIntegerParameter(GL_MAX_TEXTURE_IMAGE_UNITS);
  maxVertexTextureImageUnits = getIntegerParameter(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
  maxTextureSize = getIntegerParameter(GL_MAX_TEXTURE_SIZE);
  maxTextureCubeSize = getIntegerParameter(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB);
  maxTextureRectangleSize = getIntegerParameter(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB);
  maxVertexAttributes = getIntegerParameter(GL_MAX_VERTEX_ATTRIBS_ARB);
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

Canvas::~Canvas(void)
{
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

///////////////////////////////////////////////////////////////////////

unsigned int ScreenCanvas::getWidth(void) const
{
  return width;
}

unsigned int ScreenCanvas::getHeight(void) const
{
  return height;
}

ScreenCanvas::ScreenCanvas(Context& context):
  Canvas(context)
{
  // TODO: Get screen size.
}

void ScreenCanvas::apply(void) const
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

///////////////////////////////////////////////////////////////////////

Image::~Image(void)
{
}

///////////////////////////////////////////////////////////////////////

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
  return colorBuffer;
}

Image* ImageCanvas::getDepthBuffer(void) const
{
  return depthBuffer;
}

bool ImageCanvas::setColorBuffer(Image* newImage)
{
  if (newImage)
  {
    if (newImage->getWidth() != width || newImage->getHeight() != height)
    {
      Log::writeError("Specified color buffer does not match canvas dimensions");
      return false;
    }
  }

  colorBuffer = newImage;
  return true;
}

bool ImageCanvas::setDepthBuffer(Image* newImage)
{
  if (newImage)
  {
    if (newImage->getWidth() != width || newImage->getHeight() != height)
    {
      Log::writeError("Specified depth buffer does not match canvas dimensions");
      return false;
    }
  }

  depthBuffer = newImage;
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
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bufferID);

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during color renderbuffer creation: %s", gluErrorString(error));
    return false;
  }
#endif

  return true;
}

void ImageCanvas::apply(void) const
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bufferID);
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

bool Context::update(void)
{
  glfwSwapBuffers();

  finishSignal.emit();

  return glfwGetWindowParam(GLFW_OPENED) == GL_TRUE;
}

bool Context::isWindowed(void) const
{
  return (mode.flags & ContextMode::WINDOWED) != 0;
}

unsigned int Context::getColorBits(void) const
{
  return mode.colorBits;
}

unsigned int Context::getDepthBits(void) const
{
  return mode.depthBits;
}

unsigned int Context::getStencilBits(void) const
{
  return mode.stencilBits;
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

  updateViewportArea();
  updateScissorArea();

  return true;
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

  const int count = glfwGetVideoModes(modes, sizeof(modes) / sizeof(GLFWvidmode));

  for (unsigned int i = 0;  i < count;  i++)
  {
    result.push_back(ScreenMode(modes[i].Width,
                                modes[i].Height,
				modes[i].RedBits + modes[i].GreenBits + modes[i].BlueBits));
  }
}

Context::Context(void):
  currentCanvas(NULL),
  screenCanvas(NULL),
  cgContextID(NULL),
  cgVertexProfile(CG_PROFILE_UNKNOWN),
  cgFragmentProfile(CG_PROFILE_UNKNOWN)
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

    mode.width = initMode.width;
    mode.height = initMode.height;

    // Read back actual (as opposed to desired) framebuffer properties
    mode.colorBits = glfwGetWindowParam(GLFW_RED_BITS) +
                     glfwGetWindowParam(GLFW_GREEN_BITS) +
                     glfwGetWindowParam(GLFW_BLUE_BITS);
    mode.depthBits = glfwGetWindowParam(GLFW_DEPTH_BITS);
    mode.stencilBits = glfwGetWindowParam(GLFW_STENCIL_BITS);
    mode.samples = glfwGetWindowParam(GLFW_FSAA_SAMPLES);
    mode.flags = initMode.flags;
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
      Log::writeError("Vertex buffer objects (ARB_vertex_buffer_object) is required but not supported");
      return false;
    }

    if (!GLEW_ARB_texture_cube_map)
    {
      Log::writeError("Cube map textures (ARB_texture_cube_map) are required but not supported");
      return false;
    }

    if (!GLEW_ARB_texture_rectangle)
    {
      Log::writeError("Rectangular textures (ARB_texture_rectangle) are required but not supported");
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
    cgGLSetDebugMode(CG_TRUE);

    error = cgGetError();
    if (error != CG_NO_ERROR)
    {
      Log::writeError("Failed to set Cg options: %s", cgGetErrorString(error));
      return false;
    }
  }

  // This needs to be done before setting the window size callback
  screenCanvas = new ScreenCanvas(*this);
  screenCanvas->width = mode.width;
  screenCanvas->height = mode.height;

  setScreenCanvasCurrent();

  // Finish GLFW init
  {
    setTitle("Wendy");
    glfwPollEvents();

    glfwSetWindowSizeCallback(sizeCallback);
    glfwSetWindowCloseCallback(closeCallback);

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
  instance->mode.width = width;
  instance->mode.height = height;

  instance->screenCanvas->width = width;
  instance->screenCanvas->height = height;

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

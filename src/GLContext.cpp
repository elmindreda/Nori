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
  maxTextureCoords(0),
  maxFragmentTextureImageUnits(0),
  maxVertexTextureImageUnits(0),
  maxTextureSize(0),
  maxTextureCubeSize(0),
  maxTextureRectangleSize(0),
  maxVertexAttributes(0)
{
  glGetIntegerv(GL_MAX_TEXTURE_COORDS, (GLint*) &maxTextureCoords);
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*) &maxFragmentTextureImageUnits);
  glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, (GLint*) &maxVertexTextureImageUnits);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*) &maxTextureSize);
  glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, (GLint*) &maxTextureCubeSize);
  glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, (GLint*) &maxTextureCubeSize);
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, (GLint*) maxVertexAttributes);
}

unsigned int Limits::getMaxTextureCoords(void) const
{
  return maxTextureCoords;
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

Context::~Context(void)
{
  destroySignal.emit();

  if (cgContextID)
  {
    cgDestroyContext((CGcontext) cgContextID);
    cgContextID = NULL;
  }

  glfwCloseWindow();

  instance = NULL;
}

bool Context::update(void)
{
  bool stopped = false;

  typedef std::list<bool> ResultList;
  ResultList results;

  renderSignal.emit(results);  

  // Stop run loop if anyone returned false
  if (std::find(results.begin(), results.end(), false) != results.end())
    stopped = true;

  glfwSwapBuffers();

  // Stop run loop if the user closed the window
  // NOTE: This is here because we only find out about this after events
  //       have been processed by SwapBuffers.
  if (glfwGetWindowParam(GLFW_OPENED) != GL_TRUE)
    stopped = true;

  finishSignal.emit();

  if (stopped)
    return false;

  return true;
}

EntryPoint Context::findEntryPoint(const String& name)
{
  return (EntryPoint) glfwGetProcAddress(name.c_str());
}

bool Context::isWindowed(void) const
{
  return (mode.flags & ContextMode::WINDOWED) != 0;
}

bool Context::hasExtension(const String& name) const
{
  return glfwExtensionSupported(name.c_str()) != 0;
}

unsigned int Context::getWidth(void) const
{
  return mode.width;
}

unsigned int Context::getHeight(void) const
{
  return mode.height;
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

Image* Context::getColorBuffer(void) const
{
  // TODO: Update this when adding FBO support.
  // TODO: Eliminate stack and pixel option usage.

  Ptr<Image> result = new Image(ImageFormat::RGB888, mode.width, mode.height);

  glPushAttrib(GL_PIXEL_MODE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glReadPixels(0, 0, mode.width, mode.height,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       result->getPixels());

  glPopAttrib();

  // OpenGL images and Moira images are upside down to each other.
  result->flipHorizontal();

  return result.detachObject();
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

SignalProxy0<bool> Context::getRenderSignal(void)
{
  return renderSignal;
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
  Ptr<Context> context = new Context();
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
    Log::writeError("Cube map textures are required but not supported");
    return false;
  }

  if (!GLEW_ARB_texture_rectangle)
  {
    Log::writeError("Rectangular textures are required but not supported");
    return false;
  }

  limits = new Limits(*this);

  mode.colorBits = glfwGetWindowParam(GLFW_RED_BITS) +
                   glfwGetWindowParam(GLFW_GREEN_BITS) +
		   glfwGetWindowParam(GLFW_BLUE_BITS);
  mode.depthBits = glfwGetWindowParam(GLFW_DEPTH_BITS);
  mode.stencilBits = glfwGetWindowParam(GLFW_STENCIL_BITS);
  mode.samples = glfwGetWindowParam(GLFW_FSAA_SAMPLES);
  mode.flags = initMode.flags;

  cgContextID = cgCreateContext();
  if (!cgContextID)
  {
    Log::writeError("Unable to create Cg context");
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

  cgGLSetManageTextureParameters((CGcontext) cgContextID, CG_TRUE);
  cgSetLockingPolicy(CG_NO_LOCKS_POLICY);
  cgSetParameterSettingMode((CGcontext) cgContextID, CG_IMMEDIATE_PARAMETER_SETTING);
  cgGLSetDebugMode(CG_TRUE);

  glfwSetWindowSizeCallback(sizeCallback);
  glfwSetWindowCloseCallback(closeCallback);

  glfwSwapInterval(1);

  setTitle("Wendy");
  glfwPollEvents();

  return true;
}

void Context::sizeCallback(int width, int height)
{
  instance->mode.width = width;
  instance->mode.height = height;

  instance->resizedSignal.emit(width, height);
}

int Context::closeCallback(void)
{
  typedef std::list<bool> ResultList;
  ResultList results;

  instance->closeRequestSignal.emit(results);

  for (ResultList::const_iterator i = results.begin();  i != results.end();  i++)
  {
    if (!(*i))
      return 0;
  }

  return 1;
}

Context* Context::instance = NULL;

Signal0<void> Context::createSignal;
Signal0<void> Context::destroySignal;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

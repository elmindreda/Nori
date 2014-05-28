///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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
#include <wendy/Time.hpp>
#include <wendy/Profile.hpp>
#include <wendy/Primitive.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/OpenGL.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLFW_NO_GLU
#include <GLFW/glfw3.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
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

void GLAPIENTRY debugCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar* message,
                              const GLvoid* userParam)
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

GLenum convertToGL(PolygonFace face)
{
  switch (face)
  {
    case FACE_NONE:
      break;
    case FACE_FRONT:
      return GL_FRONT;
    case FACE_BACK:
      return GL_BACK;
    case FACE_BOTH:
      return GL_FRONT_AND_BACK;
  }

  panic("Invalid polygon face %u", face);
}

PolygonFace invertFace(PolygonFace face)
{
  switch (face)
  {
    case FACE_NONE:
      return FACE_BOTH;
    case FACE_FRONT:
      return FACE_BACK;
    case FACE_BACK:
      return FACE_FRONT;
    case FACE_BOTH:
      return FACE_NONE;
  }

  panic("Invalid polygon face %u", face);
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

GLenum convertToGL(FragmentFunction function)
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

GLenum convertToGL(StencilOp operation)
{
  switch (operation)
  {
    case STENCIL_KEEP:
      return GL_KEEP;
    case STENCIL_ZERO:
      return GL_ZERO;
    case STENCIL_REPLACE:
      return GL_REPLACE;
    case STENCIL_INCREASE:
      return GL_INCR;
    case STENCIL_DECREASE:
      return GL_DECR;
    case STENCIL_INVERT:
      return GL_INVERT;
    case STENCIL_INCREASE_WRAP:
      return GL_INCR_WRAP;
    case STENCIL_DECREASE_WRAP:
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

RenderConfig::RenderConfig(uint initColorBits,
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
  cullFace(FACE_BACK),
  srcFactor(BLEND_ONE),
  dstFactor(BLEND_ZERO),
  depthFunction(ALLOW_LESSER)
{
  for (uint i = 0;  i < 2;  i++)
  {
    stencil[i].function = ALLOW_ALWAYS;
    stencil[i].reference = 0;
    stencil[i].mask = ~0u;
    stencil[i].stencilFailOp = STENCIL_KEEP;
    stencil[i].depthFailOp = STENCIL_KEEP;
    stencil[i].depthPassOp = STENCIL_KEEP;
  }
}

///////////////////////////////////////////////////////////////////////

RenderLimits::RenderLimits(RenderContext& context)
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

  if (GREG_EXT_texture_filter_anisotropic)
    maxTextureAnisotropy = getFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
  else
    maxTextureAnisotropy = 1.f;
}

///////////////////////////////////////////////////////////////////////

RenderStats::RenderStats():
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

void RenderStats::addFrame()
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

void RenderStats::addStateChange()
{
  Frame& frame = m_frames.front();
  frame.stateChangeCount++;
}

void RenderStats::addPrimitives(PrimitiveType type, uint vertexCount)
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

void RenderStats::addTexture(size_t size)
{
  m_textureCount++;
  m_textureSize += size;
}

void RenderStats::removeTexture(size_t size)
{
  m_textureCount--;
  m_textureSize -= size;
}

void RenderStats::addVertexBuffer(size_t size)
{
  m_vertexBufferCount++;
  m_vertexBufferSize += size;
}

void RenderStats::removeVertexBuffer(size_t size)
{
  m_vertexBufferCount--;
  m_vertexBufferSize -= size;
}

void RenderStats::addIndexBuffer(size_t size)
{
  m_indexBufferCount++;
  m_indexBufferSize += size;
}

void RenderStats::removeIndexBuffer(size_t size)
{
  m_indexBufferCount--;
  m_indexBufferSize -= size;
}

void RenderStats::addProgram()
{
  m_programCount++;
}

void RenderStats::removeProgram()
{
  m_programCount--;
}

///////////////////////////////////////////////////////////////////////

RenderStats::Frame::Frame():
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

SharedProgramState::SharedProgramState():
  m_dirtyModelView(true),
  m_dirtyViewProj(true),
  m_dirtyModelViewProj(true),
  m_dirtyInvModel(true),
  m_dirtyInvView(true),
  m_dirtyInvProj(true),
  m_dirtyInvModelView(true),
  m_dirtyInvViewProj(true),
  m_dirtyInvModelViewProj(true),
  m_cameraNearZ(0.f),
  m_cameraFarZ(0.f),
  m_cameraAspect(0.f),
  m_cameraFOV(0.f),
  m_viewportWidth(0.f),
  m_viewportHeight(0.f),
  m_time(0.f)
{
}

void SharedProgramState::cameraProperties(vec3& position,
                                          float& FOV,
                                          float& aspect,
                                          float& nearZ,
                                          float& farZ) const
{
  position = m_cameraPos;
  FOV = m_cameraFOV;
  aspect = m_cameraAspect;
  nearZ = m_cameraNearZ;
  farZ = m_cameraFarZ;
}

void SharedProgramState::setModelMatrix(const mat4& newMatrix)
{
  m_modelMatrix = newMatrix;
  m_dirtyModelView = m_dirtyModelViewProj = true;
  m_dirtyInvModel = m_dirtyInvModelView = m_dirtyInvModelViewProj = true;
}

void SharedProgramState::setViewMatrix(const mat4& newMatrix)
{
  m_viewMatrix = newMatrix;
  m_dirtyModelView = m_dirtyViewProj = m_dirtyModelViewProj = true;
  m_dirtyInvView = m_dirtyInvModelView = m_dirtyInvViewProj = m_dirtyInvModelViewProj = true;
}

void SharedProgramState::setProjectionMatrix(const mat4& newMatrix)
{
  m_projectionMatrix = newMatrix;
  m_dirtyViewProj = m_dirtyModelViewProj = true;
  m_dirtyInvProj = m_dirtyInvViewProj = m_dirtyInvModelViewProj = true;
}

void SharedProgramState::setOrthoProjectionMatrix(float width, float height)
{
  setProjectionMatrix(ortho(0.f, width, 0.f, height));
}

void SharedProgramState::setOrthoProjectionMatrix(const AABB& volume)
{
  vec3 minimum, maximum;
  volume.bounds(minimum, maximum);

  setProjectionMatrix(ortho(minimum.x, maximum.x,
                            minimum.y, maximum.y,
                            minimum.z, maximum.z));
}

void SharedProgramState::setPerspectiveProjectionMatrix(float FOV,
                                                        float aspect,
                                                        float nearZ,
                                                        float farZ)
{
  setProjectionMatrix(perspective(FOV, aspect, nearZ, farZ));
}

void SharedProgramState::setCameraProperties(const vec3& position,
                                             float FOV,
                                             float aspect,
                                             float nearZ,
                                             float farZ)
{
  m_cameraPos = position;
  m_cameraFOV = FOV;
  m_cameraAspect = aspect;
  m_cameraNearZ = nearZ;
  m_cameraFarZ = farZ;
}

void SharedProgramState::setViewportSize(float newWidth, float newHeight)
{
  m_viewportWidth = newWidth;
  m_viewportHeight = newHeight;
}

void SharedProgramState::setTime(float newTime)
{
  m_time = newTime;
}

void SharedProgramState::updateTo(Sampler& sampler)
{
  logError("Unknown shared sampler uniform %s requested",
           sampler.name().c_str());
}

void SharedProgramState::updateTo(Uniform& uniform)
{
  switch (uniform.sharedID())
  {
    case SHARED_MODEL_MATRIX:
    {
      uniform.copyFrom(value_ptr(m_modelMatrix));
      return;
    }

    case SHARED_VIEW_MATRIX:
    {
      uniform.copyFrom(value_ptr(m_viewMatrix));
      return;
    }

    case SHARED_PROJECTION_MATRIX:
    {
      uniform.copyFrom(value_ptr(m_projectionMatrix));
      return;
    }

    case SHARED_MODELVIEW_MATRIX:
    {
      if (m_dirtyModelView)
      {
        m_modelViewMatrix = m_viewMatrix;
        m_modelViewMatrix *= m_modelMatrix;
        m_dirtyModelView = false;
      }

      uniform.copyFrom(value_ptr(m_modelViewMatrix));
      return;
    }

    case SHARED_VIEWPROJECTION_MATRIX:
    {
      if (m_dirtyViewProj)
      {
        m_viewProjMatrix = m_projectionMatrix;
        m_viewProjMatrix *= m_viewMatrix;
        m_dirtyViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_viewProjMatrix));
      return;
    }

    case SHARED_MODELVIEWPROJECTION_MATRIX:
    {
      if (m_dirtyModelViewProj)
      {
        if (m_dirtyViewProj)
        {
          m_viewProjMatrix = m_projectionMatrix;
          m_viewProjMatrix *= m_viewMatrix;
          m_dirtyViewProj = false;
        }

        m_modelViewProjMatrix = m_viewProjMatrix;
        m_modelViewProjMatrix *= m_modelMatrix;
        m_dirtyModelViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_modelViewProjMatrix));
      return;
    }

    case SHARED_INVERSE_MODEL_MATRIX:
    {
      if (m_dirtyInvModel)
      {
        m_invModelMatrix = inverse(m_modelMatrix);
        m_dirtyInvModel = false;
      }

      uniform.copyFrom(value_ptr(m_invModelMatrix));
      return;
    }

    case SHARED_INVERSE_VIEW_MATRIX:
    {
      if (m_dirtyInvView)
      {
        m_invViewMatrix = inverse(m_viewMatrix);
        m_dirtyInvView = false;
      }

      uniform.copyFrom(value_ptr(m_invViewMatrix));
      return;
    }

    case SHARED_INVERSE_PROJECTION_MATRIX:
    {
      if (m_dirtyInvProj)
      {
        m_invProjMatrix = inverse(m_projectionMatrix);
        m_dirtyInvProj = false;
      }

      uniform.copyFrom(value_ptr(m_invProjMatrix));
      return;
    }

    case SHARED_INVERSE_MODELVIEW_MATRIX:
    {
      if (m_dirtyInvModelView)
      {
        if (m_dirtyModelView)
        {
          m_modelViewMatrix = m_viewMatrix;
          m_modelViewMatrix *= m_modelMatrix;
          m_dirtyModelView = false;
        }

        m_invModelViewMatrix = inverse(m_modelViewMatrix);
        m_dirtyInvModelView = false;
      }

      uniform.copyFrom(value_ptr(m_invModelViewMatrix));
      return;
    }

    case SHARED_INVERSE_VIEWPROJECTION_MATRIX:
    {
      if (m_dirtyInvViewProj)
      {
        if (m_dirtyViewProj)
        {
          m_viewProjMatrix = m_projectionMatrix;
          m_viewProjMatrix *= m_viewMatrix;
          m_dirtyViewProj = false;
        }

        m_invViewProjMatrix = inverse(m_viewProjMatrix);
        m_dirtyInvViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_invViewProjMatrix));
      return;
    }

    case SHARED_INVERSE_MODELVIEWPROJECTION_MATRIX:
    {
      if (m_dirtyInvModelViewProj)
      {
        if (m_dirtyModelViewProj)
        {
          if (m_dirtyViewProj)
          {
            m_viewProjMatrix = m_projectionMatrix;
            m_viewProjMatrix *= m_viewMatrix;
            m_dirtyViewProj = false;
          }

          m_modelViewProjMatrix = m_viewProjMatrix;
          m_modelViewProjMatrix *= m_modelMatrix;
          m_dirtyModelViewProj = false;
        }

        m_invModelViewProjMatrix = inverse(m_modelViewProjMatrix);
        m_dirtyInvModelViewProj = false;
      }

      uniform.copyFrom(value_ptr(m_invModelViewProjMatrix));
      return;
    }

    case SHARED_CAMERA_POSITION:
    {
      uniform.copyFrom(value_ptr(m_cameraPos));
      return;
    }

    case SHARED_CAMERA_NEAR_Z:
    {
      uniform.copyFrom(&m_cameraNearZ);
      return;
    }

    case SHARED_CAMERA_FAR_Z:
    {
      uniform.copyFrom(&m_cameraFarZ);
      return;
    }

    case SHARED_CAMERA_ASPECT_RATIO:
    {
      uniform.copyFrom(&m_cameraAspect);
      return;
    }

    case SHARED_CAMERA_FOV:
    {
      uniform.copyFrom(&m_cameraFOV);
      return;
    }

    case SHARED_VIEWPORT_WIDTH:
    {
      uniform.copyFrom(&m_viewportWidth);
      return;
    }

    case SHARED_VIEWPORT_HEIGHT:
    {
      uniform.copyFrom(&m_viewportHeight);
      return;
    }

    case SHARED_TIME:
    {
      uniform.copyFrom(&m_time);
      return;
    }
  }

  logError("Unknown shared uniform %s requested",
           uniform.name().c_str());
}

///////////////////////////////////////////////////////////////////////

class RenderContext::SharedSampler
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

class RenderContext::SharedUniform
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

RenderContext::~RenderContext()
{
  if (m_defaultFramebuffer)
    setDefaultFramebufferCurrent();

  setCurrentVertexBuffer(nullptr);
  setCurrentIndexBuffer(nullptr);
  setCurrentProgram(nullptr);

  for (uint i = 0;  i < m_textureUnits.size();  i++)
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

void RenderContext::clearColorBuffer(const vec4& color)
{
  if (m_dirtyState)
    forceState(m_currentState);
  if (!m_currentState.colorWriting)
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  if (!m_currentState.colorWriting)
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void RenderContext::clearDepthBuffer(float depth)
{
  if (m_dirtyState)
    forceState(m_currentState);
  if (!m_currentState.depthWriting)
    glDepthMask(GL_TRUE);

  glClearDepth(depth);
  glClear(GL_DEPTH_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  if (!m_currentState.depthWriting)
    glDepthMask(GL_FALSE);
}

void RenderContext::clearStencilBuffer(uint value)
{
  if (m_dirtyState)
    forceState(m_currentState);
  if (m_currentState.stencilTesting)
    glDisable(GL_STENCIL_TEST);

  glClearStencil(value);
  glClear(GL_STENCIL_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  if (m_currentState.stencilTesting)
    glEnable(GL_STENCIL_TEST);
}

void RenderContext::clearBuffers(const vec4& color, float depth, uint value)
{
  if (m_dirtyState)
    forceState(m_currentState);
  if (!m_currentState.colorWriting)
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  if (!m_currentState.depthWriting)
    glDepthMask(GL_TRUE);
  if (m_currentState.stencilTesting)
    glDisable(GL_STENCIL_TEST);

  glClearColor(color.r, color.g, color.b, color.a);
  glClearDepth(depth);
  glClearStencil(value);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#if WENDY_DEBUG
  checkGL("Error during color buffer clearing");
#endif

  if (!m_currentState.colorWriting)
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  if (!m_currentState.depthWriting)
    glDepthMask(GL_FALSE);
  if (m_currentState.stencilTesting)
    glEnable(GL_STENCIL_TEST);
}

void RenderContext::render(const PrimitiveRange& range)
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

void RenderContext::render(PrimitiveType type, uint start, uint count, uint base)
{
  ProfileNodeCall call("RenderContext::render");

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

VertexRange RenderContext::allocateVertices(uint count, const VertexFormat& format)
{
  if (!count)
    return VertexRange();

  Slot* slot = nullptr;

  for (auto& s : m_slots)
  {
    if (s.buffer->format() == format && s.available >= count)
    {
      slot = &s;
      break;
    }
  }

  if (!slot)
  {
    m_slots.push_back(Slot());
    slot = &(m_slots.back());

    const size_t granularity = 16384;

    const uint actualCount = granularity * ((count + granularity - 1) / granularity);

    slot->buffer = VertexBuffer::create(*this,
                                        actualCount,
                                        format,
                                        USAGE_DYNAMIC);
    if (!slot->buffer)
    {
      m_slots.pop_back();
      return VertexRange();
    }

    log("Allocated vertex pool of size %u format %s",
        actualCount,
        format.asString().c_str());

    slot->available = slot->buffer->count();
  }

  const uint start = slot->buffer->count() - slot->available;

  slot->available -= count;

  return VertexRange(*(slot->buffer), start, count);
}

void RenderContext::createSharedSampler(const char* name, SamplerType type, int ID)
{
  assert(ID != -1);

  if (sharedSamplerID(name, type) != -1)
    return;

  m_declaration += format("uniform %s %s;\n", Sampler::typeName(type), name);

  m_samplers.push_back(SharedSampler(name, type, ID));
}

void RenderContext::createSharedUniform(const char* name, UniformType type, int ID)
{
  assert(ID != -1);

  if (sharedUniformID(name, type) != -1)
    return;

  m_declaration += format("uniform %s %s;\n", Uniform::typeName(type), name);

  m_uniforms.push_back(SharedUniform(name, type, ID));
}

int RenderContext::sharedSamplerID(const char* name, SamplerType type) const
{
  for (auto& s : m_samplers)
  {
    if (s.name == name && s.type == type)
      return s.ID;
  }

  return -1;
}

int RenderContext::sharedUniformID(const char* name, UniformType type) const
{
  for (auto& u : m_uniforms)
  {
    if (u.name == name && u.type == type)
      return u.ID;
  }

  return -1;
}

SharedProgramState* RenderContext::currentSharedProgramState() const
{
  return m_currentSharedState;
}

void RenderContext::setCurrentSharedProgramState(SharedProgramState* newState)
{
  m_currentSharedState = newState;
}

const char* RenderContext::sharedProgramStateDeclaration() const
{
  return m_declaration.c_str();
}

int RenderContext::swapInterval() const
{
  return m_swapInterval;
}

void RenderContext::setSwapInterval(int newInterval)
{
  glfwSwapInterval(newInterval);
  m_swapInterval = newInterval;
}

const Recti& RenderContext::scissorArea() const
{
  return m_scissorArea;
}

const Recti& RenderContext::viewportArea() const
{
  return m_viewportArea;
}

void RenderContext::setScissorArea(const Recti& newArea)
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

void RenderContext::setViewportArea(const Recti& newArea)
{
  m_viewportArea = newArea;

  glViewport(m_viewportArea.position.x,
             m_viewportArea.position.y,
             m_viewportArea.size.x,
             m_viewportArea.size.y);
}

Framebuffer& RenderContext::currentFramebuffer() const
{
  return *m_currentFramebuffer;
}

DefaultFramebuffer& RenderContext::defaultFramebuffer() const
{
  return *m_defaultFramebuffer;
}

void RenderContext::setDefaultFramebufferCurrent()
{
  setCurrentFramebuffer(*m_defaultFramebuffer);
}

bool RenderContext::setCurrentFramebuffer(Framebuffer& newFramebuffer)
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

void RenderContext::setCurrentProgram(Program* newProgram)
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

void RenderContext::setCurrentVertexBuffer(VertexBuffer* newVertexBuffer)
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

void RenderContext::setCurrentIndexBuffer(IndexBuffer* newIndexBuffer)
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

void RenderContext::setCurrentTexture(Texture* newTexture)
{
  if (m_textureUnits[m_activeTextureUnit] != newTexture)
  {
    Texture* oldTexture = m_textureUnits[m_activeTextureUnit];

    if (newTexture)
    {
      glBindTexture(convertToGL(newTexture->type()), newTexture->m_textureID);

#if WENDY_DEBUG
      if (!checkGL("Failed to bind texture %s", newTexture->name().c_str()))
        return;
#endif
    }
    else if (oldTexture)
    {
      glBindTexture(convertToGL(oldTexture->type()), 0);

#if WENDY_DEBUG
      if (!checkGL("Failed to unbind texture %s", oldTexture->name().c_str()))
        return;
#endif
    }

    m_textureUnits[m_activeTextureUnit] = newTexture;
  }
}

void RenderContext::setActiveTextureUnit(uint unit)
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

bool RenderContext::isCullingInverted()
{
  return m_cullingInverted;
}

void RenderContext::setCullingInversion(bool newState)
{
  m_cullingInverted = newState;
}

const RenderState& RenderContext::currentRenderState() const
{
  return m_currentState;
}

void RenderContext::setCurrentRenderState(const RenderState& newState)
{
  applyState(newState);
}

RenderStats* RenderContext::stats() const
{
  return m_stats;
}

void RenderContext::setStats(RenderStats* newStats)
{
  m_stats = newStats;
}

ResourceCache& RenderContext::cache() const
{
  return m_cache;
}

Window& RenderContext::window()
{
  return m_window;
}

const RenderLimits& RenderContext::limits() const
{
  return *m_limits;
}

RenderContext* RenderContext::create(ResourceCache& cache,
                                     const WindowConfig& wc,
                                     const RenderConfig& rc)
{
  std::unique_ptr<RenderContext> context(new RenderContext(cache));
  if (!context->init(wc, rc))
    return nullptr;

  return context.release();
}

RenderContext::RenderContext(ResourceCache& cache):
  m_cache(cache),
  m_handle(nullptr),
  m_dirtyBinding(true),
  m_dirtyState(true),
  m_cullingInverted(false),
  m_activeTextureUnit(0),
  m_stats(nullptr)
{
}

bool RenderContext::init(const WindowConfig& wc, const RenderConfig& rc)
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
    const uint colorBits = min(rc.colorBits, 24u);

    glfwWindowHint(GLFW_RED_BITS, colorBits / 3);
    glfwWindowHint(GLFW_GREEN_BITS, colorBits / 3);
    glfwWindowHint(GLFW_BLUE_BITS, colorBits / 3);
    glfwWindowHint(GLFW_DEPTH_BITS, rc.depthBits);
    glfwWindowHint(GLFW_STENCIL_BITS, rc.stencilBits);
    glfwWindowHint(GLFW_SAMPLES, rc.samples);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, rc.debug);

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

    m_window.init(m_handle);
    m_window.frameSignal().connect(*this, &RenderContext::onFrame);
  }

  // Initialize greg and check extensions
  {
    if (!gregInit())
    {
      logError("Failed to load OpenGL functions");
      return false;
    }

    log("OpenGL context version %i.%i created",
        glfwGetWindowAttrib(m_handle, GLFW_CONTEXT_VERSION_MAJOR),
        glfwGetWindowAttrib(m_handle, GLFW_CONTEXT_VERSION_MINOR));

    log("OpenGL context GLSL version is %s",
        (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

    log("OpenGL context renderer is %s by %s",
        (const char*) glGetString(GL_RENDERER),
        (const char*) glGetString(GL_VENDOR));

    if (rc.debug && GREG_ARB_debug_output)
    {
      glDebugMessageCallbackARB(debugCallback, nullptr);
      glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
  }

  // Retrieve context limits and set up dependent caches
  {
    m_limits.reset(new RenderLimits(*this));

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
    forceState(RenderState());

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_PROGRAM_POINT_SIZE);
  }

  createSharedUniform("wyM", UNIFORM_MAT4, SHARED_MODEL_MATRIX);
  createSharedUniform("wyV", UNIFORM_MAT4, SHARED_VIEW_MATRIX);
  createSharedUniform("wyP", UNIFORM_MAT4, SHARED_PROJECTION_MATRIX);
  createSharedUniform("wyMV", UNIFORM_MAT4, SHARED_MODELVIEW_MATRIX);
  createSharedUniform("wyVP", UNIFORM_MAT4, SHARED_VIEWPROJECTION_MATRIX);
  createSharedUniform("wyMVP", UNIFORM_MAT4, SHARED_MODELVIEWPROJECTION_MATRIX);

  createSharedUniform("wyInvM", UNIFORM_MAT4, SHARED_INVERSE_MODEL_MATRIX);
  createSharedUniform("wyInvV", UNIFORM_MAT4, SHARED_INVERSE_VIEW_MATRIX);
  createSharedUniform("wyInvP", UNIFORM_MAT4, SHARED_INVERSE_PROJECTION_MATRIX);
  createSharedUniform("wyInvMV", UNIFORM_MAT4, SHARED_INVERSE_MODELVIEW_MATRIX);
  createSharedUniform("wyInvVP", UNIFORM_MAT4, SHARED_INVERSE_VIEWPROJECTION_MATRIX);
  createSharedUniform("wyInvMVP", UNIFORM_MAT4, SHARED_INVERSE_MODELVIEWPROJECTION_MATRIX);

  createSharedUniform("wyCameraNearZ", UNIFORM_FLOAT, SHARED_CAMERA_NEAR_Z);
  createSharedUniform("wyCameraFarZ", UNIFORM_FLOAT, SHARED_CAMERA_FAR_Z);
  createSharedUniform("wyCameraAspectRatio", UNIFORM_FLOAT, SHARED_CAMERA_ASPECT_RATIO);
  createSharedUniform("wyCameraFOV", UNIFORM_FLOAT, SHARED_CAMERA_FOV);
  createSharedUniform("wyCameraPosition", UNIFORM_VEC3, SHARED_CAMERA_POSITION);

  createSharedUniform("wyViewportWidth", UNIFORM_FLOAT, SHARED_VIEWPORT_WIDTH);
  createSharedUniform("wyViewportHeight", UNIFORM_FLOAT, SHARED_VIEWPORT_HEIGHT);

  createSharedUniform("wyTime", UNIFORM_FLOAT, SHARED_TIME);

  return true;
}

void RenderContext::applyState(const RenderState& newState)
{
  if (m_stats)
    m_stats->addStateChange();

  if (m_dirtyState)
  {
    forceState(newState);
    return;
  }

  PolygonFace cullFace = newState.cullFace;
  if (m_cullingInverted)
    cullFace = invertFace(cullFace);

  if (cullFace != m_currentState.cullFace)
  {
    if ((cullFace == FACE_NONE) != (m_currentState.cullFace == FACE_NONE))
      setBooleanState(GL_CULL_FACE, cullFace != FACE_NONE);

    if (cullFace != FACE_NONE)
      glCullFace(convertToGL(cullFace));

    m_currentState.cullFace = cullFace;
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
      const FragmentFunction depthFunction = ALLOW_ALWAYS;

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

  if (newState.stencilTesting)
  {
    const GLenum faces[] = { GL_FRONT, GL_BACK };

    for (uint i = 0;  i < 2;  i++)
    {
      if (newState.stencil[i].function != m_currentState.stencil[i].function ||
          newState.stencil[i].reference != m_currentState.stencil[i].reference ||
          newState.stencil[i].mask != m_currentState.stencil[i].mask)
      {
        glStencilFuncSeparate(faces[i],
                              convertToGL(newState.stencil[i].function),
                              newState.stencil[i].reference,
                              newState.stencil[i].mask);
      }

      if (newState.stencil[i].stencilFailOp != m_currentState.stencil[i].stencilFailOp ||
          newState.stencil[i].depthFailOp != m_currentState.stencil[i].depthFailOp ||
          newState.stencil[i].depthPassOp != m_currentState.stencil[i].depthPassOp)
      {
        glStencilOpSeparate(faces[i],
                            convertToGL(newState.stencil[i].stencilFailOp),
                            convertToGL(newState.stencil[i].depthFailOp),
                            convertToGL(newState.stencil[i].depthPassOp));
      }

      m_currentState.stencil[i] = newState.stencil[i];
    }
  }

#if WENDY_DEBUG
  checkGL("Error when applying render state");
#endif
}

void RenderContext::forceState(const RenderState& newState)
{
  m_currentState = newState;

  PolygonFace cullFace = newState.cullFace;
  if (m_cullingInverted)
    cullFace = invertFace(cullFace);

  setBooleanState(GL_CULL_FACE, cullFace != FACE_NONE);
  if (cullFace != FACE_NONE)
    glCullFace(convertToGL(cullFace));

  setBooleanState(GL_BLEND, newState.srcFactor != BLEND_ONE ||
                            newState.dstFactor != BLEND_ZERO);
  glBlendFunc(convertToGL(newState.srcFactor), convertToGL(newState.dstFactor));

  glDepthMask(newState.depthWriting ? GL_TRUE : GL_FALSE);
  setBooleanState(GL_DEPTH_TEST, newState.depthTesting || newState.depthWriting);

  if (newState.depthWriting && !newState.depthTesting)
  {
    const FragmentFunction depthFunction = ALLOW_ALWAYS;
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
  glStencilFuncSeparate(GL_FRONT,
                        convertToGL(newState.stencil[FACE_FRONT].function),
                        newState.stencil[FACE_FRONT].reference,
                        newState.stencil[FACE_FRONT].mask);
  glStencilFuncSeparate(GL_BACK,
                        convertToGL(newState.stencil[FACE_BACK].function),
                        newState.stencil[FACE_BACK].reference,
                        newState.stencil[FACE_BACK].mask);
  glStencilOpSeparate(GL_FRONT,
                      convertToGL(newState.stencil[FACE_FRONT].stencilFailOp),
                      convertToGL(newState.stencil[FACE_FRONT].depthFailOp),
                      convertToGL(newState.stencil[FACE_FRONT].depthPassOp));
  glStencilOpSeparate(GL_BACK,
                      convertToGL(newState.stencil[FACE_BACK].stencilFailOp),
                      convertToGL(newState.stencil[FACE_BACK].depthFailOp),
                      convertToGL(newState.stencil[FACE_BACK].depthPassOp));

#if WENDY_DEBUG
  checkGL("Error when forcing render state");
#endif

  m_dirtyState = false;
}

void RenderContext::onFrame()
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

  for (auto& s : m_slots)
  {
    s.available = s.buffer->count();
    s.buffer->discard();
  }

  if (m_stats)
    m_stats->addFrame();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

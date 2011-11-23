///////////////////////////////////////////////////////////////////////
// Wendy deferred renderer
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderPool.h>

#include <wendy/Deferred.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace deferred
  {

///////////////////////////////////////////////////////////////////////

namespace
{

struct LightVertex
{
  vec2 position;
  vec2 texCoord;
  vec2 clipOverF;
  static VertexFormat format;
};

VertexFormat LightVertex::format("2f:wyPosition 2f:wyTexCoord 2f:wyClipOverF");

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Config::Config(unsigned int initWidth, unsigned int initHeight):
  width(initWidth),
  height(initHeight)
{
}

///////////////////////////////////////////////////////////////////////

void Renderer::render(const render::Scene& scene, const render::Camera& camera)
{
  GL::Context& context = pool.getContext();

  Ref<GL::SharedProgramState> prevState = context.getCurrentSharedProgramState();
  context.setCurrentSharedProgramState(state);

  Ref<GL::Framebuffer> prevFramebuffer = &(context.getCurrentFramebuffer());
  context.setCurrentFramebuffer(*framebuffer);

  const Recti prevViewportArea = context.getViewportArea();
  context.setViewportArea(Recti(0, 0, framebuffer->getWidth(), framebuffer->getHeight()));

  context.clearDepthBuffer();
  context.clearColorBuffer();

  state->setViewMatrix(camera.getViewTransform());
  state->setPerspectiveProjectionMatrix(camera.getFOV(),
                                        camera.getAspectRatio(),
                                        camera.getNearZ(),
                                        camera.getFarZ());
  state->setCameraProperties(camera.getTransform().position,
                             camera.getFOV(),
                             camera.getAspectRatio(),
                             camera.getNearZ(),
                             camera.getFarZ());

  renderOperations(scene.getOpaqueQueue());

  context.setCurrentFramebuffer(*prevFramebuffer);
  context.setViewportArea(prevViewportArea);

  state->setOrthoProjectionMatrix(1.f, 1.f);

  vec3 ambient = scene.getAmbientIntensity();
  if (ambient.r > 0.f || ambient.g > 0.f || ambient.b > 0.f)
    renderAmbientLight(camera, ambient);

  for (unsigned int i = 0;  i < scene.getLightCount();  i++)
    renderLight(camera, scene.getLight(i));

  context.setCurrentSharedProgramState(prevState);
}

SharedProgramState& Renderer::getSharedProgramState()
{
  return *state;
}

render::GeometryPool& Renderer::getGeometryPool()
{
  return pool;
}

GL::Texture& Renderer::getColorTexture() const
{
  return *colorTexture;
}

GL::Texture& Renderer::getNormalTexture() const
{
  return *normalTexture;
}

GL::Texture& Renderer::getDepthTexture() const
{
  return *depthTexture;
}

Renderer* Renderer::create(render::GeometryPool& pool, const Config& config)
{
  Ptr<Renderer> renderer(new Renderer(pool));
  if (!renderer->init(config))
    return NULL;

  return renderer.detachObject();
}

Renderer::Renderer(render::GeometryPool& initPool):
  pool(initPool)
{
}

bool Renderer::init(const Config& config)
{
  GL::Context& context = pool.getContext();
  ResourceCache& cache = context.getCache();

  if (config.state)
    state = config.state;
  else
    state = new SharedProgramState();

  state->reserveSupported(context);

  // Create G-buffer color/emission texture
  {
    Image image(cache, PixelFormat::RGBA8, config.width, config.height);

    colorTexture = GL::Texture::create(cache,
                                       context,
                                       image,
                                       GL::Texture::RECTANGULAR);
    if (!colorTexture)
    {
      logError("Failed to create color texture for deferred renderer");
      return false;
    }

    colorTexture->setFilterMode(GL::FILTER_NEAREST);
  }

  // Create G-buffer normal/specularity texture
  {
    Image image(cache, PixelFormat::RGBA8, config.width, config.height);

    normalTexture = GL::Texture::create(cache,
                                        context,
                                        image,
                                        GL::Texture::RECTANGULAR);
    if (!normalTexture)
    {
      logError("Failed to create normal/specularity texture for deferred renderer");
      return false;
    }

    normalTexture->setFilterMode(GL::FILTER_NEAREST);
  }

  // Create G-buffer depth texture
  {
    Image image(cache, PixelFormat::DEPTH32, config.width, config.height);

    depthTexture = GL::Texture::create(cache,
                                       context,
                                       image,
                                       GL::Texture::RECTANGULAR);
    if (!depthTexture)
    {
      logError("Failed to create depth texture for deferred renderer");
      return false;
    }

    depthTexture->setFilterMode(GL::FILTER_NEAREST);
  }

  // Set up G-buffer framebuffer
  {
    framebuffer = GL::ImageFramebuffer::create(context);
    if (!framebuffer)
    {
      logError("Failed to create G-buffer framebuffer for deferred renderer");
      return false;
    }

    if (!framebuffer->setBuffer(GL::ImageFramebuffer::COLOR_BUFFER0, colorTexture->getImage(0)))
    {
      logError("Failed to attach color texture to G-buffer");
      return false;
    }

    if (!framebuffer->setBuffer(GL::ImageFramebuffer::COLOR_BUFFER1, normalTexture->getImage(0)))
    {
      logError("Failed to attach normal/specularity texture to G-buffer");
      return false;
    }

    if (!framebuffer->setBuffer(GL::ImageFramebuffer::DEPTH_BUFFER, depthTexture->getImage(0)))
    {
      logError("Failed to attach depth texture to G-buffer");
      return false;
    }
  }

  // Set up ambient light pass
  {
    Path path("wendy/DeferredAmbientLight.program");

    Ref<GL::Program> program = GL::Program::read(context, path);
    if (!program)
    {
      logError("Failed to read deferred ambient light program \'%s\'",
               path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("colorTexture", GL::Sampler::SAMPLER_RECT);
    interface.addUniform("light.color", GL::Uniform::VEC3);
    interface.addAttribute("wyPosition", GL::Attribute::VEC2);
    interface.addAttribute("wyTexCoord", GL::Attribute::VEC2);

    if (!interface.matches(*program, true))
    {
      logError("Deferred ambient light program \'%s\' does not match the required interface",
               program->getPath().asString().c_str());
      return false;
    }

    ambientLightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    ambientLightPass.setDepthTesting(false);
    ambientLightPass.setDepthWriting(false);
    ambientLightPass.setProgram(program);
    ambientLightPass.setSamplerState("colorTexture", colorTexture);
  }

  // Set up directional light pass
  {
    Path path("wendy/DeferredDirLight.program");

    Ref<GL::Program> program = GL::Program::read(context, path);
    if (!program)
    {
      logError("Failed to read deferred directional light program \'%s\'",
               path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("colorTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("normalTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("depthTexture", GL::Sampler::SAMPLER_RECT);
    interface.addUniform("nearZ", GL::Uniform::FLOAT);
    interface.addUniform("nearOverFarZminusOne", GL::Uniform::FLOAT);
    interface.addUniform("light.direction", GL::Uniform::VEC3);
    interface.addUniform("light.color", GL::Uniform::VEC3);
    interface.addAttribute("wyPosition", GL::Attribute::VEC2);
    interface.addAttribute("wyTexCoord", GL::Attribute::VEC2);
    interface.addAttribute("wyClipOverF", GL::Attribute::VEC2);

    if (!interface.matches(*program, true))
    {
      logError("Deferred directional light program \'%s\' does not match the required interface",
               program->getPath().asString().c_str());
      return false;
    }

    dirLightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    dirLightPass.setDepthTesting(false);
    dirLightPass.setDepthWriting(false);
    dirLightPass.setProgram(program);
    dirLightPass.setSamplerState("colorTexture", colorTexture);
    dirLightPass.setSamplerState("normalTexture", normalTexture);
    dirLightPass.setSamplerState("depthTexture", depthTexture);
  }

  // Set up point light pass
  {
    Path path("wendy/DeferredPointLight.program");

    Ref<GL::Program> program = GL::Program::read(context, path);
    if (!program)
    {
      logError("Failed to read deferred point light program \'%s\'",
               path.asString().c_str());
      return false;
    }

    path = "wendy/DistanceRamp.texture";

    Ref<GL::Texture> distanceRamp = GL::Texture::read(context, path);
    if (!distanceRamp)
    {
      logError("Failed to read attenuation texture \'%s\'",
               path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;

    interface.addSampler("colorTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("normalTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("depthTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("distanceRamp", GL::Sampler::SAMPLER_1D);
    interface.addUniform("nearZ", GL::Uniform::FLOAT);
    interface.addUniform("nearOverFarZminusOne", GL::Uniform::FLOAT);
    interface.addUniform("light.position", GL::Uniform::VEC3);
    interface.addUniform("light.color", GL::Uniform::VEC3);
    interface.addUniform("light.radius", GL::Uniform::FLOAT);
    interface.addAttribute("wyPosition", GL::Attribute::VEC2);
    interface.addAttribute("wyTexCoord", GL::Attribute::VEC2);
    interface.addAttribute("wyClipOverF", GL::Attribute::VEC2);

    if (!interface.matches(*program, true))
    {
      logError("Deferred point light program \'%s\' does not match the required interface",
               program->getPath().asString().c_str());
      return false;
    }

    pointLightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    pointLightPass.setDepthTesting(false);
    pointLightPass.setDepthWriting(false);
    pointLightPass.setProgram(program);
    pointLightPass.setSamplerState("colorTexture", colorTexture);
    pointLightPass.setSamplerState("normalTexture", normalTexture);
    pointLightPass.setSamplerState("depthTexture", depthTexture);
    pointLightPass.setSamplerState("distanceRamp", distanceRamp);
  }

  return true;
}

void Renderer::renderLightQuad(const render::Camera& camera)
{
  GL::VertexRange range;

  if (!pool.allocateVertices(range, 4, LightVertex::format))
  {
    logError("Failed to allocate vertices for deferred lighting");
    return;
  }

  LightVertex vertices[4];

  const float f = tan(radians(camera.getFOV()) / 2.f);
  const float aspect = camera.getAspectRatio();

  vertices[0].texCoord = vec2(0.5f, 0.5f);
  vertices[0].position = vec2(0.f, 0.f);
  vertices[0].clipOverF = vec2(-f * aspect, -f);

  vertices[1].texCoord = vec2(framebuffer->getWidth() + 0.5f, 0.5f);
  vertices[1].position = vec2(1.f, 0.f);
  vertices[1].clipOverF = vec2(f * aspect, -f);

  vertices[2].texCoord = vec2(framebuffer->getWidth() + 0.5f, framebuffer->getHeight() + 0.5f);
  vertices[2].position = vec2(1.f, 1.f);
  vertices[2].clipOverF = vec2(f * aspect, f);

  vertices[3].texCoord = vec2(0.5f, framebuffer->getHeight() + 0.5f);
  vertices[3].position = vec2(0.f, 1.f);
  vertices[3].clipOverF = vec2(-f * aspect, f);

  range.copyFrom(vertices);

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
}

void Renderer::renderAmbientLight(const render::Camera& camera, const vec3& color)
{
  ambientLightPass.setUniformState("light.color", color);
  ambientLightPass.apply();

  renderLightQuad(camera);
}

void Renderer::renderLight(const render::Camera& camera, const render::Light& light)
{
  const float nearZ = camera.getNearZ();
  const float nearOverFarZminusOne = nearZ / camera.getFarZ() - 1.f;

  if (light.getType() == render::Light::POINT)
  {
    pointLightPass.setUniformState("nearZ", nearZ);
    pointLightPass.setUniformState("nearOverFarZminusOne", nearOverFarZminusOne);

    vec3 position = light.getPosition();
    camera.getViewTransform().transformVector(position);
    pointLightPass.setUniformState("light.position", position);

    pointLightPass.setUniformState("light.color", light.getColor());
    pointLightPass.setUniformState("light.radius", light.getRadius());

    pointLightPass.apply();
  }
  else if (light.getType() == render::Light::DIRECTIONAL)
  {
    dirLightPass.setUniformState("nearZ", nearZ);
    dirLightPass.setUniformState("nearOverFarZminusOne", nearOverFarZminusOne);

    vec3 direction = light.getDirection();
    camera.getViewTransform().rotateVector(direction);
    dirLightPass.setUniformState("light.direction", direction);

    dirLightPass.setUniformState("light.color", light.getColor());

    dirLightPass.apply();
  }
  else
  {
    logError("Unsupported light type %u", light.getType());
    return;
  }

  renderLightQuad(camera);
}

void Renderer::renderOperations(const render::Queue& queue)
{
  GL::Context& context = pool.getContext();
  const render::SortKeyList& keys = queue.getSortKeys();
  const render::OperationList& operations = queue.getOperations();

  for (render::SortKeyList::const_iterator k = keys.begin();  k != keys.end();  k++)
  {
    const render::Operation& op = operations[k->index];

    state->setModelMatrix(op.transform);
    op.state->apply();

    context.render(op.range);
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace deferred*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

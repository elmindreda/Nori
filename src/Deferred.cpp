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
  Vec2 position;
  Vec2 mapping;
  Vec2 clipOverF;
  static VertexFormat format;
};

VertexFormat LightVertex::format("2f:position 2f:mapping 2f:clipOverF");

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Config::Config(unsigned int initWidth, unsigned int initHeight):
  width(initWidth),
  height(initHeight)
{
}

///////////////////////////////////////////////////////////////////////

void Renderer::render(const render::Queue& queue)
{
  context.setCurrentCanvas(*canvas);
  context.clearDepthBuffer();
  context.clearColorBuffer(ColorRGBA::BLACK);

  queue.render();

  context.setScreenCanvasCurrent();
  context.setProjectionMatrix2D(1.f, 1.f);

  const render::LightState& lights = queue.getLights();
  for (unsigned int i = 0;  i < lights.getLightCount();  i++)
    renderLight(queue.getCamera(), lights.getLight(i));
}

void Renderer::renderLight(const render::Camera& camera, const render::Light& light)
{
  const float nearZ = camera.getMinDepth();
  const float nearOverFarZminusOne = nearZ / camera.getMaxDepth() - 1.f;

  if (light.getType() == render::Light::POINT)
  {
    pointLightPass.getUniformState("nearZ").setValue(nearZ);
    pointLightPass.getUniformState("nearOverFarZminusOne").setValue(nearOverFarZminusOne);

    Vec3 position = light.getPosition();
    camera.getViewTransform().transformVector(position);
    pointLightPass.getUniformState("light.position").setValue(position);

    pointLightPass.getUniformState("light.color").setValue(light.getColor());
    pointLightPass.getUniformState("light.radius").setValue(light.getRadius());
    pointLightPass.getSamplerState("light.distAttTexture").setTexture(light.getDistAttTexture());

    pointLightPass.apply();
  }
  else if (light.getType() == render::Light::DIRECTIONAL)
  {
    dirLightPass.getUniformState("nearZ").setValue(nearZ);
    dirLightPass.getUniformState("nearOverFarZminusOne").setValue(nearOverFarZminusOne);

    Vec3 direction = light.getDirection();
    camera.getViewTransform().rotation.rotateVector(direction);
    dirLightPass.getUniformState("light.direction").setValue(direction);

    dirLightPass.getUniformState("light.color").setValue(light.getColor());

    dirLightPass.apply();
  }
  else
  {
    Log::writeError("Unsupported light type %u", light.getType());
    return;
  }

  GL::VertexRange range;

  if (!render::GeometryPool::get()->allocateVertices(range, 4, LightVertex::format))
    return;

  LightVertex vertices[4];

  const float radians = camera.getFOV() * (float) M_PI / 180.f;
  const float f = tanf(radians / 2.f);
  const float aspect = camera.getAspectRatio();

  vertices[0].mapping.set(0.5f, 0.5f);
  vertices[0].position.set(0.f, 0.f);
  vertices[0].clipOverF.set(-f * aspect, -f);

  vertices[1].mapping.set(canvas->getWidth() + 0.5f, 0.5f);
  vertices[1].position.set(1.f, 0.f);
  vertices[1].clipOverF.set(f * aspect, -f);

  vertices[2].mapping.set(canvas->getWidth() + 0.5f, canvas->getHeight() + 0.5f);
  vertices[2].position.set(1.f, 1.f);
  vertices[2].clipOverF.set(f * aspect, f);

  vertices[3].mapping.set(0.5f, canvas->getHeight() + 0.5f);
  vertices[3].position.set(0.f, 1.f);
  vertices[3].clipOverF.set(-f * aspect, f);

  range.copyFrom(vertices);

  context.render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
}

GL::Texture& Renderer::getColorTexture(void) const
{
  return *colorTexture;
}

GL::Texture& Renderer::getNormalTexture(void) const
{
  return *normalTexture;
}

GL::Texture& Renderer::getDepthTexture(void) const
{
  return *depthTexture;
}

Renderer* Renderer::create(GL::Context& context, const Config& config)
{
  Ptr<Renderer> renderer(new Renderer(context));
  if (!renderer->init(config))
    return NULL;

  return renderer.detachObject();
}

Renderer::Renderer(GL::Context& initContext):
  context(initContext)
{
}

bool Renderer::init(const Config& config)
{
  // Create G-buffer color/emission texture
  {
    Image data(PixelFormat::RGBA8, config.width, config.height);

    colorTexture = GL::Texture::createInstance(context, data, GL::Texture::RECTANGULAR);
    if (!colorTexture)
    {
      Log::writeError("Failed to create color texture for deferred renderer");
      return false;
    }

    colorTexture->setFilterMode(GL::FILTER_NEAREST);
  }

  // Create G-buffer normal/specularity texture
  {
    Image data(PixelFormat::RGBA8, config.width, config.height);

    normalTexture = GL::Texture::createInstance(context, data, GL::Texture::RECTANGULAR);
    if (!normalTexture)
    {
      Log::writeError("Failed to create normal/specularity texture for deferred renderer");
      return false;
    }

    normalTexture->setFilterMode(GL::FILTER_NEAREST);
  }

  // Create G-buffer depth texture
  {
    Image data(PixelFormat::DEPTH32, config.width, config.height);

    depthTexture = GL::Texture::createInstance(context, data, GL::Texture::RECTANGULAR);
    if (!depthTexture)
    {
      Log::writeError("Failed to create depth texture for deferred renderer");
      return false;
    }

    depthTexture->setFilterMode(GL::FILTER_NEAREST);
  }

  // Set up G-buffer canvas
  {
    canvas = GL::ImageCanvas::createInstance(context, config.width, config.height);
    if (!canvas)
    {
      Log::writeError("Failed to create image canvas for deferred renderer");
      return false;
    }

    if (!canvas->setBuffer(GL::ImageCanvas::COLOR_BUFFER0, &(colorTexture->getImage(0))))
    {
      Log::writeError("Failed to attach color texture to G-buffer");
      return false;
    }

    if (!canvas->setBuffer(GL::ImageCanvas::COLOR_BUFFER1, &(normalTexture->getImage(0))))
    {
      Log::writeError("Failed to attach normal/specularity texture to G-buffer");
      return false;
    }

    if (!canvas->setBuffer(GL::ImageCanvas::DEPTH_BUFFER, &(depthTexture->getImage(0))))
    {
      Log::writeError("Failed to attach depth texture to G-buffer");
      return false;
    }
  }

  // Set up directional light pass
  {
    GL::ProgramRef lightProgram = GL::Program::readInstance("DeferredDirLight");
    if (!lightProgram)
    {
      Log::writeError("Failed to read deferred renderer directional light program");
      return false;
    }

    GL::ProgramInterface interface;

    interface.addSampler("colorTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("normalTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("depthTexture", GL::Sampler::SAMPLER_RECT);

    interface.addUniform("nearZ", GL::Uniform::FLOAT);
    interface.addUniform("nearOverFarZminusOne", GL::Uniform::FLOAT);

    interface.addUniform("light.direction", GL::Uniform::FLOAT_VEC3);
    interface.addUniform("light.color", GL::Uniform::FLOAT_VEC3);

    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);
    interface.addVarying("clipOverF", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*lightProgram, true))
    {
      Log::writeError("Deferred renderer directional light program does not match the required interface");
      return false;
    }

    dirLightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    dirLightPass.setDepthTesting(false);
    dirLightPass.setDepthWriting(false);
    dirLightPass.setProgram(lightProgram);
    dirLightPass.getSamplerState("colorTexture").setTexture(colorTexture);
    dirLightPass.getSamplerState("normalTexture").setTexture(normalTexture);
    dirLightPass.getSamplerState("depthTexture").setTexture(depthTexture);
  }

  // Set up point light pass
  {
    GL::ProgramRef lightProgram = GL::Program::readInstance("DeferredPointLight");
    if (!lightProgram)
    {
      Log::writeError("Failed to read deferred renderer point light program");
      return false;
    }

    GL::ProgramInterface interface;

    interface.addSampler("colorTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("normalTexture", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("depthTexture", GL::Sampler::SAMPLER_RECT);

    interface.addUniform("nearZ", GL::Uniform::FLOAT);
    interface.addUniform("nearOverFarZminusOne", GL::Uniform::FLOAT);

    interface.addUniform("light.position", GL::Uniform::FLOAT_VEC3);
    interface.addUniform("light.color", GL::Uniform::FLOAT_VEC3);
    interface.addUniform("light.radius", GL::Uniform::FLOAT);
    interface.addSampler("light.distAttTexture", GL::Sampler::SAMPLER_1D);

    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);
    interface.addVarying("clipOverF", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*lightProgram, true))
    {
      Log::writeError("Deferred renderer point light program does not match the required interface");
      return false;
    }

    pointLightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    pointLightPass.setDepthTesting(false);
    pointLightPass.setDepthWriting(false);
    pointLightPass.setProgram(lightProgram);
    pointLightPass.getSamplerState("colorTexture").setTexture(colorTexture);
    pointLightPass.getSamplerState("normalTexture").setTexture(normalTexture);
    pointLightPass.getSamplerState("depthTexture").setTexture(depthTexture);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace deferred*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

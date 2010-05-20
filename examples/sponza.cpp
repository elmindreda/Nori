
#include <wendy/Wendy.h>

using namespace wendy;

struct LightVertex
{
  Vec2 position;
  Vec2 mapping;
  Vec2 clipOverF;
  static VertexFormat format;
};

VertexFormat LightVertex::format("2f:position 2f:mapping 2f:clipOverF");

class Demo : public Trackable
{
public:
  Demo(void);
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  void renderLight(const render::Light& light);
  void onKeyPressed(input::Key key, bool pressed);
  void onButtonClicked(input::Button button, bool clicked);
  input::SpectatorCamera controller;
  Ref<GL::ImageCanvas> canvas;
  GL::RenderState dirLightPass;
  GL::RenderState pointLightPass;
  GL::RenderState blitPass;
  GL::TextureRef colorTexture;
  GL::TextureRef depthTexture;
  GL::TextureRef normalTexture;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
  scene::LightNode* lightNode;
  Timer timer;
  Time currentTime;
  bool quitting;
  bool debugging;
};

Demo::Demo(void):
  quitting(false),
  debugging(false),
  currentTime(0.0),
  cameraNode(NULL)
{
}

Demo::~Demo(void)
{
  graph.destroyRootNodes();

  camera = NULL;

  canvas = NULL;

  colorTexture = NULL;
  depthTexture = NULL;
  normalTexture = NULL;

  dirLightPass.setProgram(NULL);
  pointLightPass.setProgram(NULL);
  blitPass.setProgram(NULL);

  input::Context::destroy();
  GL::Renderer::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  Image::addSearchPath(Path("media/sponza"));
  Mesh::addSearchPath(Path("media/sponza"));
  GL::Texture::addSearchPath(Path("media/sponza"));
  GL::VertexProgram::addSearchPath(Path("media/sponza"));
  GL::FragmentProgram::addSearchPath(Path("media/sponza"));
  GL::Program::addSearchPath(Path("media/sponza"));
  render::Material::addSearchPath(Path("media/sponza"));

  if (!GL::Context::create(GL::ContextMode()))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Sponza Atrium");

  const unsigned int width = context->getCurrentCanvas().getWidth();
  const unsigned int height = context->getCurrentCanvas().getHeight();

  if (!input::Context::create(*context))
    return false;

  input::Context::get()->getKeyPressedSignal().connect(*this, &Demo::onKeyPressed);
  input::Context::get()->getButtonClickedSignal().connect(*this, &Demo::onButtonClicked);

  if (!GL::Renderer::create(*context))
    return false;

  // Set up G-buffer
  {
    canvas = GL::ImageCanvas::createInstance(*context, width, height);
    if (!canvas)
      return false;

    colorTexture = GL::Texture::createInstance(*context,
                                              Image(PixelFormat::RGBA8, width, height),
                                              GL::Texture::RECTANGULAR,
                                              "colorbuffer");
    if (!colorTexture)
      return false;

    colorTexture->setFilterMode(GL::FILTER_NEAREST);
    canvas->setBuffer(GL::ImageCanvas::COLOR_BUFFER0, &(colorTexture->getImage(0)));

    normalTexture = GL::Texture::createInstance(*context,
                                                Image(PixelFormat::RGBA8, width, height),
                                                GL::Texture::RECTANGULAR,
                                                "normalbuffer");
    if (!normalTexture)
      return false;

    normalTexture->setFilterMode(GL::FILTER_NEAREST);
    canvas->setBuffer(GL::ImageCanvas::COLOR_BUFFER1, &(normalTexture->getImage(0)));

    depthTexture = GL::Texture::createInstance(*context,
                                              Image(PixelFormat::DEPTH32, width, height),
                                              GL::Texture::RECTANGULAR,
                                              "depthbuffer");
    if (!depthTexture)
      return false;

    depthTexture->setFilterMode(GL::FILTER_NEAREST);
    canvas->setBuffer(GL::ImageCanvas::DEPTH_BUFFER, &(depthTexture->getImage(0)));
  }

  // Set up lighting pass
  {
    GL::ProgramRef lightProgram = GL::Program::readInstance("dirlight");
    if (!lightProgram)
      return false;

    GL::ProgramInterface interface;
    interface.addSampler("colorbuffer", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("normalbuffer", GL::Sampler::SAMPLER_RECT);
    interface.addUniform("nearZ", GL::Uniform::FLOAT);
    interface.addUniform("nearOverFarZminusOne", GL::Uniform::FLOAT);
    interface.addUniform("light.direction", GL::Uniform::FLOAT_VEC3);
    interface.addUniform("light.color", GL::Uniform::FLOAT_VEC3);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);
    interface.addVarying("clipOverF", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*lightProgram, true))
      return false;

    dirLightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    dirLightPass.setDepthTesting(false);
    dirLightPass.setDepthWriting(false);
    dirLightPass.setProgram(lightProgram);
    dirLightPass.getSamplerState("colorbuffer").setTexture(colorTexture);
    dirLightPass.getSamplerState("normalbuffer").setTexture(normalTexture);
    dirLightPass.getSamplerState("depthbuffer").setTexture(depthTexture);
  }

  // Set up lighting pass
  {
    GL::ProgramRef lightProgram = GL::Program::readInstance("pointlight");
    if (!lightProgram)
      return false;

    GL::ProgramInterface interface;
    interface.addSampler("colorbuffer", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("normalbuffer", GL::Sampler::SAMPLER_RECT);
    interface.addSampler("depthbuffer", GL::Sampler::SAMPLER_RECT);
    interface.addUniform("nearZ", GL::Uniform::FLOAT);
    interface.addUniform("nearOverFarZminusOne", GL::Uniform::FLOAT);
    interface.addUniform("light.position", GL::Uniform::FLOAT_VEC3);
    interface.addUniform("light.color", GL::Uniform::FLOAT_VEC3);
    interface.addUniform("light.linear", GL::Uniform::FLOAT);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);
    interface.addVarying("clipOverF", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*lightProgram, true))
      return false;

    pointLightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    pointLightPass.setDepthTesting(false);
    pointLightPass.setDepthWriting(false);
    pointLightPass.setProgram(lightProgram);
    pointLightPass.getSamplerState("colorbuffer").setTexture(colorTexture);
    pointLightPass.getSamplerState("normalbuffer").setTexture(normalTexture);
    pointLightPass.getSamplerState("depthbuffer").setTexture(depthTexture);
  }

  // Set up debug buffer blit pass
  {
    GL::ProgramRef blitProgram = GL::Program::readInstance("blit");
    if (!blitProgram)
      return false;

    GL::ProgramInterface interface;
    interface.addSampler("image", GL::Sampler::SAMPLER_RECT);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*blitProgram, true))
      return false;

    blitPass.setDepthTesting(false);
    blitPass.setDepthWriting(false);
    blitPass.setProgram(blitProgram);
  }

  Ref<render::Mesh> mesh = render::Mesh::readInstance("sponza");
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  scene::MeshNode* meshNode = new scene::MeshNode();
  meshNode->setMesh(mesh);
  graph.addRootNode(*meshNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setDepthRange(0.9f, 500.f);
  camera->setAspectRatio((float) width / height);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  graph.addRootNode(*cameraNode);

  render::LightRef light = new render::Light();
  light->setType(render::Light::POINT);
  light->setLinearAttenuation(0.05f);
  light->setBounds(Sphere(Vec3::ZERO, 500.f));

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  timer.start();

  input::Context::get()->setFocus(&controller);

  return true;
}

void Demo::run(void)
{
  GL::Context* context = GL::Context::get();
  GL::Renderer* renderer = GL::Renderer::get();

  render::Queue queue(*camera);

  do
  {
    const Time deltaTime = timer.getTime() - currentTime;
    currentTime += deltaTime;

    lightNode->getLocalTransform().position.y = sinf(currentTime) * 40.f + 45.f;

    controller.update(deltaTime);
    cameraNode->getLocalTransform() = controller.getTransform();

    graph.update();

    context->setCurrentCanvas(*canvas);
    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    graph.enqueue(queue);
    queue.render();

    context->setScreenCanvasCurrent();
    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    renderer->setProjectionMatrix2D(1.f, 1.f);

    const render::LightState& lights = queue.getLights();
    for (unsigned int i = 0;  i < lights.getLightCount();  i++)
      renderLight(lights.getLight(i));

    if (debugging)
    {
      render::Sprite2 sprite;
      sprite.size.set(0.25f, 0.25f);
      sprite.mapping.set(0.25f, 0.25f,
                         0.25f + canvas->getWidth(),
                         0.25f + canvas->getHeight());

      blitPass.getSamplerState("image").setTexture(colorTexture);
      blitPass.apply();

      sprite.position.set(0.f + sprite.size.x / 2.f, 1.f - 0.5f * sprite.size.y);
      sprite.render();

      blitPass.getSamplerState("image").setTexture(normalTexture);
      blitPass.apply();

      sprite.position.set(0.f + sprite.size.x / 2.f, 1.f - 1.5f * sprite.size.y);
      sprite.render();
    }

    queue.destroyOperations();
    queue.detachLights();
  }
  while (not quitting and context->update());
}

void Demo::renderLight(const render::Light& light)
{
  if (light.getType() == render::Light::POINT)
  {
    pointLightPass.getUniformState("nearZ").setValue(camera->getMinDepth());
    pointLightPass.getUniformState("nearOverFarZminusOne").setValue(camera->getMinDepth() / camera->getMaxDepth() - 1.f);

    Vec3 position = light.getPosition();
    camera->getViewTransform().transformVector(position);
    pointLightPass.getUniformState("light.position").setValue(position);

    Vec3 color(light.getColor().r, light.getColor().g, light.getColor().b);
    pointLightPass.getUniformState("light.color").setValue(color);

    pointLightPass.getUniformState("light.linear").setValue(light.getLinearAttenuation());

    pointLightPass.apply();
  }
  else if (light.getType() == render::Light::DIRECTIONAL)
  {
    dirLightPass.getUniformState("nearZ").setValue(camera->getMinDepth());
    dirLightPass.getUniformState("nearOverFarZminusOne").setValue(camera->getMinDepth() / camera->getMaxDepth() - 1.f);

    Vec3 direction = light.getDirection();
    camera->getViewTransform().rotation.rotateVector(direction);
    dirLightPass.getUniformState("light.direction").setValue(direction);

    Vec3 color(light.getColor().r, light.getColor().g, light.getColor().b);
    dirLightPass.getUniformState("light.color").setValue(color);

    dirLightPass.apply();
  }

  GL::Renderer* renderer = GL::Renderer::get();

  GL::VertexRange range;

  if (!renderer->allocateVertices(range, 4, LightVertex::format))
    return;

  LightVertex vertices[4];

  const float radians = camera->getFOV() * (float) M_PI / 180.f;
  const float f = tanf(radians / 2.f);
  const float aspect = camera->getAspectRatio();

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

  renderer->render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
}

void Demo::onKeyPressed(input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::Key::TAB:
      debugging = not debugging;
      break;

    case input::Key::ESCAPE:
      quitting = true;
      break;
  }
}

void Demo::onButtonClicked(input::Button button, bool clicked)
{
  if (!clicked)
    return;

  if (button == input::Button::LEFT)
  {
  }
}

int main()
{
  if (!wendy::initialize())
    exit(1);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
    demo->run();

  demo = NULL;

  wendy::shutdown();
  exit(0);
}


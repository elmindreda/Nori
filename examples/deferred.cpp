
#include <wendy/Wendy.h>

using namespace wendy;

struct Light
{
  Vec3 direction;
  ColorRGB color;
};

typedef std::vector<Light> LightList;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  bool render(void);
  input::MayaCamera controller;
  Ref<GL::ImageCanvas> canvas;
  GL::RenderState lightPass;
  GL::TextureRef colorTexture;
  GL::TextureRef depthTexture;
  GL::TextureRef normalTexture;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::Node* rootNode;
  scene::CameraNode* cameraNode;
  LightList lights;
  Timer timer;
  Time currentTime;
};

Demo::~Demo(void)
{
  input::Context::destroy();
  GL::Renderer::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  Image::addSearchPath(Path("media/deferred"));
  Mesh::addSearchPath(Path("media/deferred"));
  GL::Texture::addSearchPath(Path("media/deferred"));
  GL::VertexProgram::addSearchPath(Path("media/deferred"));
  GL::FragmentProgram::addSearchPath(Path("media/deferred"));
  GL::Program::addSearchPath(Path("media/deferred"));
  render::Material::addSearchPath(Path("media/deferred"));

  if (!GL::Context::create(GL::ContextMode()))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Deferred Rendering");

  const unsigned int width = context->getCurrentCanvas().getWidth();
  const unsigned int height = context->getCurrentCanvas().getHeight();

  if (!GL::Renderer::create(*context))
    return false;

  if (!input::Context::create(*context))
    return false;

  input::Context::get()->setFocus(&controller);

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

  GL::ProgramRef lightProgram = GL::Program::readInstance("deflight");
  if (!lightProgram)
    return false;

  GL::ProgramInterface interface;
  interface.addSampler("colorbuffer", GL::Sampler::SAMPLER_RECT);
  interface.addSampler("normalbuffer", GL::Sampler::SAMPLER_RECT);
  interface.addUniform("light.direction", GL::Uniform::FLOAT_VEC3);
  interface.addUniform("light.color", GL::Uniform::FLOAT_VEC3);
  interface.addVarying("position", GL::Varying::FLOAT_VEC2);
  interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);

  if (!interface.matches(*lightProgram, true))
    return false;

  lightPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
  lightPass.setDepthTesting(false);
  lightPass.setDepthWriting(false);
  lightPass.setProgram(lightProgram);
  lightPass.getSamplerState("colorbuffer").setTexture(colorTexture);
  lightPass.getSamplerState("normalbuffer").setTexture(normalTexture);

  Ref<render::Mesh> mesh = render::Mesh::readInstance("cube");
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  rootNode = new scene::Node();
  graph.addRootNode(*rootNode);

  RandomRange angle(0.f, M_PI * 2.f);
  RandomVolume axis(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f));
  RandomVolume position(Vec3(-2.f, -2.f, -2.f), Vec3(2.f, 2.f, 2.f));

  for (size_t i = 0;  i < 20;  i++)
  {
    scene::MeshNode* meshNode = new scene::MeshNode();
    meshNode->setMesh(mesh);
    meshNode->getLocalTransform().position = position.generate();
    meshNode->getLocalTransform().rotation.setAxisRotation(axis.generate().normalized(),
                                                           angle.generate());
    rootNode->addChild(*meshNode);
  }

  camera = new render::Camera();
  camera->setFOV(60.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = mesh->getBounds().radius * 3.f;
  graph.addRootNode(*cameraNode);

  lights.resize(2);
  lights[0].direction = Vec3(1.f, 1.f, 1.f).normalized();
  lights[0].color.set(1.f, 0.3f, 0.3f);
  lights[1].direction = Vec3(-1.f, 0.f, 0.f).normalized();
  lights[1].color.set(0.7f, 0.2f, 0.8f);

  timer.start();

  return true;
}

void Demo::run(void)
{
  GL::Context* context = GL::Context::get();
  GL::Renderer* renderer = GL::Renderer::get();

  render::Queue queue(*camera);

  render::Sprite2 sprite;
  sprite.position.set(0.5f, 0.5f);
  sprite.mapping.set(0.25f, 0.25f,
                     0.25f + canvas->getWidth(),
                     0.25f + canvas->getHeight());

  do
  {
    currentTime = timer.getTime();

    rootNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);
    cameraNode->getLocalTransform() = controller.getTransform();

    graph.update();

    context->setCurrentCanvas(*canvas);
    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    graph.enqueue(queue);
    queue.render();
    queue.destroyOperations();

    context->setScreenCanvasCurrent();
    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    renderer->setProjectionMatrix2D(1.f, 1.f);

    for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    {
      Vec3 direction = i->direction;
      controller.getTransform().rotation.rotateVector(direction);
      lightPass.getUniformState("light.direction").setValue(direction);

      Vec3 color(i->color.r, i->color.g, i->color.b);
      lightPass.getUniformState("light.color").setValue(color);

      lightPass.apply();
      sprite.render();
    }
  }
  while (context->update());
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



#include <wendy/Wendy.h>

using namespace wendy;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  bool render(void);
  Ref<GL::ImageCanvas> canvas;
  GL::RenderState lightPass;
  GL::TextureRef colorTexture;
  GL::TextureRef depthTexture;
  GL::TextureRef normalTexture;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::MeshNode* meshNode;
  scene::CameraNode* cameraNode;
  Timer timer;
  Time currentTime;
};

Demo::~Demo(void)
{
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

  canvas = GL::ImageCanvas::createInstance(*context, width, height);
  if (!canvas)
    return false;

  colorTexture = GL::Texture::createInstance(*context,
                                             Image(PixelFormat::RGBA8, width, height),
                                             GL::Texture::RECTANGULAR,
                                             "colorbuffer");
  if (!colorTexture)
    return false;

  canvas->setBuffer(GL::ImageCanvas::COLOR_BUFFER0, &(colorTexture->getImage(0)));

  normalTexture = GL::Texture::createInstance(*context,
                                              Image(PixelFormat::RGBA8, width, height),
                                              GL::Texture::RECTANGULAR,
                                              "normalbuffer");
  if (!normalTexture)
    return false;

  canvas->setBuffer(GL::ImageCanvas::COLOR_BUFFER1, &(normalTexture->getImage(0)));

  depthTexture = GL::Texture::createInstance(*context,
                                             Image(PixelFormat::DEPTH32, width, height),
                                             GL::Texture::RECTANGULAR,
                                             "depthbuffer");
  if (!depthTexture)
    return false;

  canvas->setBuffer(GL::ImageCanvas::DEPTH_BUFFER, &(depthTexture->getImage(0)));

  GL::ProgramRef lightProgram = GL::Program::readInstance("deflight");
  if (!lightProgram)
    return false;

  GL::ProgramInterface interface;
  interface.addSampler("colorbuffer", GL::Sampler::SAMPLER_RECT);
  interface.addSampler("normalbuffer", GL::Sampler::SAMPLER_RECT);
  /*
  interface.addUniform("light.direction", GL::Uniform::FLOAT_VEC3);
  interface.addUniform("light.color", GL::Uniform::FLOAT_VEC3);
  */
  interface.addVarying("position", GL::Varying::FLOAT_VEC2);
  interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);

  if (!interface.matches(*lightProgram, true))
    return false;

  lightPass.setProgram(lightProgram);
  lightPass.getSamplerState("colorbuffer").setTexture(colorTexture);
  lightPass.getSamplerState("normalbuffer").setTexture(normalTexture);
  /*
  lightPass.getUniformState("light.direction").setValue(Vec3(0.58f, 0.58f, 0.58f));
  lightPass.getUniformState("light.color").setValue(Vec3(0.9f, 0.3f, 0.3f));
  */

  Ref<render::Mesh> mesh = render::Mesh::readInstance("cube");
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  meshNode = new scene::MeshNode();
  meshNode->setMesh(mesh);
  graph.addRootNode(*meshNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setAspectRatio(0.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = mesh->getBounds().radius * 3.f;
  graph.addRootNode(*cameraNode);

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
  sprite.mapping.set(0.25f, 0.25f, 0.25f + canvas->getWidth(), 0.25f + canvas->getHeight());

  do
  {
    currentTime = timer.getTime();

    meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);

    graph.update();

    context->setCurrentCanvas(*canvas);
    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    graph.enqueue(queue);
    queue.render();

    /*
    GL::TextureImage& image = normalTexture->getImage(0);

    Image data(image.getFormat(), image.getWidth(), image.getHeight());

    image.copyTo(data);

    Image::writeInstance(Path("foo.png"), data);
    */

    context->setScreenCanvasCurrent();
    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    renderer->setProjectionMatrix2D(1.f, 1.f);

    lightPass.apply();
    sprite.render();

    queue.destroyOperations();
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


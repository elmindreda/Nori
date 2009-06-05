
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  void renderQueuePass(const render::Queue& queue, const String& name);
  Ref<GL::Texture> texture;
  Ptr<GL::TextureCanvas> canvas;
  GL::RenderState blackPass;
  GL::RenderState horzPass;
  GL::RenderState vertPass;
  GL::RenderState composePass;
  render::Camera camera;
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
  Image::addSearchPath(Path("../media"));
  Mesh::addSearchPath(Path("../media"));
  GL::Texture::addSearchPath(Path("../media"));
  GL::VertexProgram::addSearchPath(Path("../media"));
  GL::FragmentProgram::addSearchPath(Path("../media"));
  GL::Program::addSearchPath(Path("../media"));
  render::Material::addSearchPath(Path("../media"));

  GL::ContextMode mode(640, 480, 32, 16, 0, 0, GL::ContextMode::WINDOWED);
  if (!GL::Context::create(mode))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Bloom");

  if (!GL::Renderer::create(*context))
    return false;

  texture = GL::Texture::createInstance(Image(ImageFormat::RGB888, 64, 64), 0);
  if (!texture)
    return false;

  canvas = GL::TextureCanvas::createInstance(*context, 32, 32);
  if (!canvas)
    return false;

  canvas->setColorBufferTexture(texture);

  Ref<GL::Program> program;
  
  program = GL::Program::readInstance("black");
  if (!program)
    return false;

  blackPass.setProgram(program);

  program = GL::Program::readInstance("horzblur");
  if (!program)
    return false;

  horzPass.setProgram(program);
  horzPass.getSamplerState("image").setTexture(texture);

  program = GL::Program::readInstance("vertblur");
  if (!program)
    return false;

  vertPass.setProgram(program);
  vertPass.getSamplerState("image").setTexture(texture);

  program = GL::Program::readInstance("compose");
  if (!program)
    return false;

  composePass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
  composePass.setProgram(program);
  composePass.getSamplerState("image").setTexture(texture);

  Ref<render::Mesh> mesh = render::Mesh::readInstance("cube");
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  meshNode = new scene::MeshNode();
  meshNode->setMesh(mesh);
  graph.addNode(*meshNode);

  camera.setFOV(60.f);
  camera.setAspectRatio(0.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCameraName(camera.getName());
  cameraNode->getLocalTransform().position.z = mesh->getBounds().radius * 3.f;
  graph.addNode(*cameraNode);

  timer.start();

  return true;
}

void Demo::run(void)
{
  do
  {
    currentTime = timer.getTime();

    meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   currentTime);

    graph.setTimeElapsed(currentTime);

    render::Queue queue(camera);
    graph.enqueue(queue);

    GL::Renderer* renderer = GL::Renderer::get();

    renderer->setCurrentCanvas(*canvas);
    renderer->clearDepthBuffer();
    renderer->clearColorBuffer(ColorRGBA(0.f, 0.f, 0.f, 1.f));

    renderQueuePass(queue, "bloom");

    renderer->setScreenCanvasCurrent();
    renderer->clearDepthBuffer();
    renderer->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

    queue.render();

    renderer->setProjectionMatrix2D(4.f, 4.f);

    composePass.apply();

    render::Sprite2 sprite;
    sprite.position.set(0.5f, 0.5f);
    sprite.render();
  }
  while (GL::Context::get()->update());
}

void Demo::renderQueuePass(const render::Queue& queue, const String& name)
{
  GL::Renderer* renderer = GL::Renderer::get();

  queue.getCamera().apply();

  typedef render::OperationList List;
  const List& operations = queue.getOperations();

  for (List::const_iterator o = operations.begin();  o != operations.end();  o++)
  {
    const render::Operation& operation = **o;

    if (const render::Pass* pass = operation.technique->findPass(name))
      pass->apply();
    else
      blackPass.apply();

    renderer->setModelMatrix(operation.transform);
    renderer->setCurrentPrimitiveRange(operation.range);
    renderer->render();
  }
}

int main(void)
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



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
  void onButtonClicked(input::Button button, bool clicked);
  void onCursorMoved(const Vec2i& position);
  void onWheelTurned(int position);
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
  Vec2i oldCursorPosition;
};

Demo::~Demo(void)
{
  input::Context::destroy();
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

  if (!input::Context::create(*context))
    return false;

  input::Context::get()->getCursorMovedSignal().connect(*this, &Demo::onCursorMoved);
  input::Context::get()->getButtonClickedSignal().connect(*this, &Demo::onButtonClicked);
  input::Context::get()->getWheelTurnedSignal().connect(*this, &Demo::onWheelTurned);

  texture = GL::Texture::createInstance(Image(ImageFormat::RGB888, 32, 32), 0);
  if (!texture)
    return false;

  canvas = GL::TextureCanvas::createInstance(*context,
                                             texture->getPhysicalWidth(),
                                             texture->getPhysicalHeight());
  if (!canvas)
    return false;

  canvas->setColorBufferTexture(texture);

  Ref<GL::Program> program;
  
  program = GL::Program::readInstance("black");
  if (!program)
    return false;

  blackPass.setProgram(program);

  const Vec2 scale(1.f / texture->getPhysicalWidth(),
                   1.f / texture->getPhysicalHeight());

  program = GL::Program::readInstance("horzblur");
  if (!program)
    return false;

  horzPass.setDepthTesting(false);
  horzPass.setDepthWriting(false);
  horzPass.setProgram(program);
  horzPass.getSamplerState("image").setTexture(texture);
  horzPass.getUniformState("scale").setValue(scale);

  program = GL::Program::readInstance("vertblur");
  if (!program)
    return false;

  vertPass.setDepthTesting(false);
  vertPass.setDepthWriting(false);
  vertPass.setProgram(program);
  vertPass.getSamplerState("image").setTexture(texture);
  vertPass.getUniformState("scale").setValue(scale);

  program = GL::Program::readInstance("compose");
  if (!program)
    return false;

  composePass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE);
  composePass.setDepthTesting(false);
  composePass.setDepthWriting(false);
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
  camera.setAspectRatio(4.f / 3.f);

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
    graph.setTimeElapsed(timer.getTime());

    render::Sprite2 sprite;
    sprite.position.set(0.5f, 0.5f);

    render::Queue queue(camera);
    graph.enqueue(queue);

    GL::Renderer* renderer = GL::Renderer::get();

    renderer->setCurrentCanvas(*canvas);
    renderer->clearDepthBuffer();
    renderer->clearColorBuffer(ColorRGBA(0.f, 0.f, 0.f, 1.f));

    queue.render();

    renderer->setProjectionMatrix2D(1.f, 1.f);

    renderer->setCurrentCanvas(*canvas);

    horzPass.apply();
    sprite.render();

    renderer->setCurrentCanvas(*canvas);

    vertPass.apply();
    sprite.render();

    renderer->setCurrentCanvas(*canvas);

    horzPass.apply();
    sprite.render();

    renderer->setCurrentCanvas(*canvas);

    vertPass.apply();
    sprite.render();

    renderer->setScreenCanvasCurrent();
    renderer->clearDepthBuffer();
    renderer->clearColorBuffer(ColorRGBA(0.f, 0.f, 0.f, 1.f));

    queue.render();

    renderer->setProjectionMatrix2D(1.f, 1.f);

    composePass.apply();

    sprite.render();
  }
  while (GL::Context::get()->update());
}

void Demo::onButtonClicked(input::Button button, bool clicked)
{
  input::Context* context = input::Context::get();

  if (clicked)
  {
    context->captureCursor();
    oldCursorPosition = context->getCursorPosition();
  }
  else
    context->releaseCursor();
}

void Demo::onCursorMoved(const Vec2i& position)
{
  input::Context* context = input::Context::get();

  if (context->isCursorCaptured())
  {
    const Vec2i offset = position - oldCursorPosition;

    Quat rotation;

    if (offset.x)
    {
      rotation.setAxisRotation(Vec3::Y, offset.x / 50.f);
      Quat& parent = meshNode->getLocalTransform().rotation;
      parent = rotation * parent;
    }

    if (offset.y)
    {
      rotation.setAxisRotation(Vec3::X, offset.y / 50.f);
      Quat& parent = meshNode->getLocalTransform().rotation;
      parent = rotation * parent;
    }

    oldCursorPosition = position;
  }
}

void Demo::onWheelTurned(int offset)
{
  const float scale = meshNode->getMesh()->getBounds().radius / 10.f;

  meshNode->getLocalTransform().position.z += offset * scale;
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


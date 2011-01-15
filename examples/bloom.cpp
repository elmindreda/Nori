
#include <wendy/Wendy.h>

#include <cstdlib>

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
  ResourceIndex index;
  Ptr<render::GeometryPool> pool;
  Ref<GL::Texture> textures[2];
  Ref<GL::ImageCanvas> canvases[2];
  GL::RenderState horzPass;
  GL::RenderState vertPass;
  GL::RenderState composePass;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::ModelNode* modelNode;
  scene::CameraNode* cameraNode;
  Timer timer;
  Vec2i oldCursorPosition;
};

Demo::~Demo(void)
{
  for (int i = 0;  i < 2;  i++)
  {
    textures[i] = NULL;
    canvases[i] = NULL;
  }

  horzPass.setProgram(NULL);
  vertPass.setProgram(NULL);
  composePass.setProgram(NULL);

  graph.destroyRootNodes();

  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool Demo::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Bloom");

  if (!input::Context::createSingleton(*context))
    return false;

  input::Context::getSingleton()->getCursorMovedSignal().connect(*this, &Demo::onCursorMoved);
  input::Context::getSingleton()->getButtonClickedSignal().connect(*this, &Demo::onButtonClicked);
  input::Context::getSingleton()->getWheelTurnedSignal().connect(*this, &Demo::onWheelTurned);

  pool = new render::GeometryPool(*context);

  const int size = 32;

  Ref<GL::RenderBuffer> depthBuffer = GL::RenderBuffer::create(PixelFormat::DEPTH24, size, size);
  if (!depthBuffer)
  {
    logError("Failed to create depth render buffer");
    return false;
  }

  for (int i = 0;  i < 2;  i++)
  {
    Image data(index, PixelFormat::RGBA8, size, size);

    textures[i] = GL::Texture::create(index, *context, data, 0);
    if (!textures[i])
    {
      logError("Failed to create canvas texture");
      return false;
    }

    textures[i]->setAddressMode(GL::ADDRESS_CLAMP);

    GL::ImageRef colorBuffer = &(textures[i]->getImage(0));

    canvases[i] = GL::ImageCanvas::create(*context, size, size);
    if (!canvases[i])
    {
      logError("Failed to create canvas");
      return false;
    }

    canvases[i]->setColorBuffer(colorBuffer);
    canvases[i]->setDepthBuffer(depthBuffer);
  }

  const Vec2 scale(1.f / size, 1.f / size);

  // Load bloom post-processing programs
  {
    Ref<GL::Program> program;
    GL::ProgramReader reader(*context);

    program = reader.read(Path("horzblur.program"));
    if (!program)
      return false;

    horzPass.setDepthTesting(false);
    horzPass.setDepthWriting(false);
    horzPass.setProgram(program);
    horzPass.getSamplerState("image").setTexture(textures[0]);
    horzPass.getUniformState("scale").setValue(scale);

    program = reader.read(Path("vertblur.program"));
    if (!program)
      return false;

    vertPass.setDepthTesting(false);
    vertPass.setDepthWriting(false);
    vertPass.setProgram(program);
    vertPass.getSamplerState("image").setTexture(textures[1]);
    vertPass.getUniformState("scale").setValue(scale);

    program = reader.read(Path("compose.program"));
    if (!program)
    {
      logError("Failed to load compose program");
      return false;
    }

    composePass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ONE);
    composePass.setDepthTesting(false);
    composePass.setDepthWriting(false);
    composePass.setProgram(program);
    composePass.getSamplerState("image").setTexture(textures[0]);
  }

  // Set up scene
  {
    render::ModelReader reader(*context);
    Ref<render::Model> model = reader.read(Path("cube.model"));
    if (!model)
    {
      logError("Failed to load model");
      return false;
    }

    modelNode = new scene::ModelNode();
    modelNode->setModel(model);
    graph.addRootNode(*modelNode);

    camera = new render::Camera();
    camera->setFOV(60.f);
    camera->setAspectRatio(4.f / 3.f);

    cameraNode = new scene::CameraNode();
    cameraNode->setCamera(camera);
    cameraNode->getLocalTransform().position.z = model->getBounds().radius * 3.f;
    graph.addRootNode(*cameraNode);
  }

  timer.start();

  return true;
}

void Demo::run(void)
{
  render::Queue queue(*pool, *camera);
  GL::Context& context = pool->getContext();

  do
  {
    graph.update();

    render::Sprite2 sprite;
    sprite.position.set(0.5f, 0.5f);

    graph.enqueue(queue);

    context.setCurrentCanvas(*canvases[0]);
    context.clearDepthBuffer();
    context.clearColorBuffer(ColorRGBA(0.f, 0.f, 0.f, 1.f));

    queue.render("bloom");

    context.setProjectionMatrix2D(1.f, 1.f);

    for (unsigned int i = 0;  i < 2;  i++)
    {
      context.setCurrentCanvas(*canvases[1]);
      context.clearDepthBuffer();
      context.clearColorBuffer(ColorRGBA(0.f, 0.f, 0.f, 1.f));

      horzPass.apply();
      sprite.render(*pool);

      context.setCurrentCanvas(*canvases[0]);
      context.clearDepthBuffer();
      context.clearColorBuffer(ColorRGBA(0.f, 0.f, 0.f, 1.f));

      vertPass.apply();
      sprite.render(*pool);
    }

    context.setScreenCanvasCurrent();
    context.clearDepthBuffer();
    context.clearColorBuffer(ColorRGBA(0.f, 0.f, 0.f, 1.f));

    queue.render();

    context.setProjectionMatrix2D(1.f, 1.f);

    composePass.apply();
    sprite.render(*pool);

    queue.removeOperations();
  }
  while (context.update());
}

void Demo::onButtonClicked(input::Button button, bool clicked)
{
  input::Context* context = input::Context::getSingleton();

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
  input::Context* context = input::Context::getSingleton();

  if (context->isCursorCaptured())
  {
    const Vec2i offset = position - oldCursorPosition;

    Quat rotation;

    if (offset.x)
    {
      rotation.setAxisRotation(Vec3::Y, offset.x / 50.f);
      Quat& parent = modelNode->getLocalTransform().rotation;
      parent = rotation * parent;
    }

    if (offset.y)
    {
      rotation.setAxisRotation(Vec3::X, offset.y / 50.f);
      Quat& parent = modelNode->getLocalTransform().rotation;
      parent = rotation * parent;
    }

    oldCursorPosition = position;
  }
}

void Demo::onWheelTurned(int offset)
{
  const float scale = modelNode->getModel()->getBounds().radius / 10.f;

  modelNode->getLocalTransform().position.z += offset * scale;
}

int main(void)
{
  if (!wendy::initialize())
    std::exit(EXIT_FAILURE);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
    demo->run();

  demo = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}


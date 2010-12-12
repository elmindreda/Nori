
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
  Ref<GL::Texture> texture;
  Ptr<GL::ImageCanvas> canvas;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
  Vec2i oldCursorPosition;
  Timer timer;
};

Demo::~Demo(void)
{
  input::Context::destroy();
  render::GeometryPool::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Water");

  if (!render::GeometryPool::createSingleton(*context))
    return false;

  if (!input::Context::createSingleton(*context))
    return false;

  input::Context::get()->getCursorMovedSignal().connect(*this, &Demo::onCursorMoved);
  input::Context::get()->getButtonClickedSignal().connect(*this, &Demo::onButtonClicked);
  input::Context::get()->getWheelTurnedSignal().connect(*this, &Demo::onWheelTurned);

  texture = GL::Texture::create(index, *context, Image(index, PixelFormat::RGB8, 64, 64), 0);
  if (!texture)
    return false;

  canvas = GL::ImageCanvas::create(*context, 64, 64);
  if (!canvas)
    return false;

  canvas->setColorBuffer(&(texture->getImage(0)));

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setAspectRatio(4.f / 3.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  graph.addRootNode(*cameraNode);

  timer.start();

  return true;
}

void Demo::run(void)
{
  do
  {
    graph.update();

    render::Queue queue(*camera);
    graph.enqueue(queue);

    GL::Context* context = GL::Context::get();

    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    queue.render();
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
      //rotation.setAxisRotation(Vec3::Y, offset.x / 50.f);
      //Quat& parent = meshNode->getLocalTransform().rotation;
      //parent = rotation * parent;
    }

    if (offset.y)
    {
      //rotation.setAxisRotation(Vec3::X, offset.y / 50.f);
      //Quat& parent = meshNode->getLocalTransform().rotation;
      //parent = rotation * parent;
    }

    oldCursorPosition = position;
  }
}

void Demo::onWheelTurned(int offset)
{
  //const float scale = meshNode->getMesh()->getBounds().radius / 10.f;

  //meshNode->getLocalTransform().position.z += offset * scale;
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


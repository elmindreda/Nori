
#include <wendy/Wendy.h>

using namespace wendy;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  void onRequestedWL(GL::Uniform& uniform);
  void onRequestedLight(GL::Uniform& uniform);
  void onButtonClicked(input::Button button, bool clicked);
  void onCursorMoved(const Vec2i& position);
  void onWheelTurned(int position);
  Ref<GL::ImageCanvas> canvas;
  Ref<GL::Texture> depthmap;
  Ref<GL::Texture> colormap;
  Ref<render::Camera> lightCamera;
  Ref<render::Camera> viewCamera;
  scene::Graph graph;
  scene::MeshNode* meshNode;
  scene::CameraNode* lightCameraNode;
  scene::CameraNode* viewCameraNode;
  Timer timer;
  Mat4 WL;
  Vec2i oldCursorPosition;
};

Demo::~Demo(void)
{
  input::Context::destroy();
  render::GeometryPool::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  Image::addSearchPath(Path("media"));
  Mesh::addSearchPath(Path("media"));
  GL::Texture::addSearchPath(Path("media"));
  GL::VertexProgram::addSearchPath(Path("media"));
  GL::FragmentProgram::addSearchPath(Path("media"));
  GL::Program::addSearchPath(Path("media"));
  render::Material::addSearchPath(Path("media"));

  if (!GL::Context::create(GL::ContextMode()))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Shadow Map");

  if (!render::GeometryPool::create(*context))
    return false;

  context->reserveUniform("WL", GL::Uniform::FLOAT_MAT4).connect(*this, &Demo::onRequestedWL);
  context->reserveUniform("light", GL::Uniform::FLOAT_VEC3).connect(*this, &Demo::onRequestedLight);

  if (!input::Context::create(*context))
    return false;

  input::Context::get()->getCursorMovedSignal().connect(*this, &Demo::onCursorMoved);
  input::Context::get()->getButtonClickedSignal().connect(*this, &Demo::onButtonClicked);
  input::Context::get()->getWheelTurnedSignal().connect(*this, &Demo::onWheelTurned);

  const unsigned int size = 512;

  depthmap = GL::Texture::createInstance(*context, Image(PixelFormat::DEPTH32, size, size), 0, "depthmap");
  if (!depthmap)
    return false;

  colormap = GL::Texture::createInstance(*context, Image(PixelFormat::RGBA8, size, size), 0);
  if (!colormap)
    return false;

  canvas = GL::ImageCanvas::createInstance(*context, size, size);
  if (!canvas)
    return false;

  canvas->setDepthBuffer(&depthmap->getImage());
  canvas->setColorBuffer(&colormap->getImage());

  Ref<render::Mesh> mesh = render::Mesh::readInstance("hills");
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  const float radius = mesh->getBounds().radius;

  meshNode = new scene::MeshNode();
  meshNode->setMesh(mesh);
  graph.addRootNode(*meshNode);

  viewCamera = new render::Camera();
  viewCamera->setFOV(60.f);
  viewCamera->setAspectRatio(0.f);

  viewCameraNode = new scene::CameraNode();
  viewCameraNode->setCamera(viewCamera);
  viewCameraNode->getLocalTransform().position.z = radius * 2.f;
  graph.addRootNode(*viewCameraNode);

  lightCamera = new render::Camera();
  lightCamera->setFOV(60.f);
  lightCamera->setAspectRatio(1.f);
  lightCamera->setDepthRange(0.1f, radius * 6.f);

  lightCameraNode = new scene::CameraNode();
  lightCameraNode->setCamera(lightCamera);
  lightCameraNode->getLocalTransform().position.set(radius * 2.f, 0.f, radius * 2.f);
  lightCameraNode->getLocalTransform().rotation.setAxisRotation(Vec3::Y, (float) M_PI / 4.f);
  graph.addRootNode(*lightCameraNode);

  timer.start();

  return true;
}

void Demo::run(void)
{
  GL::Context* context = GL::Context::get();

  do
  {
    Time currentTime = timer.getTime();

    graph.update();

    // Calculate back-projection matrix for this frame
    {
      WL.x.set(0.5f, 0.f, 0.f, 0.f);
      WL.y.set(0.f, 0.5f, 0.f, 0.f);
      WL.z.set(0.f, 0.f, 0.5f, 0.f);
      WL.w.set(0.5f, 0.5f, 0.5f, 1.f);

      Mat4 LP;
      LP.setProjection3D(lightCamera->getFOV(),
                          lightCamera->getAspectRatio(),
                          lightCamera->getMinDepth(),
                          lightCamera->getMaxDepth());
      WL *= LP;

      Mat4 LV;
      LV = lightCamera->getViewTransform();
      WL *= LV;
    }

    // Render shadow map
    {
      context->setCurrentCanvas(*canvas);
      context->clearDepthBuffer();
      context->clearColorBuffer();

      render::Queue queue(*lightCamera);
      graph.enqueue(queue);
      queue.render("shadowmap");
    }

    // Render view
    {
      context->setScreenCanvasCurrent();
      context->clearDepthBuffer();
      context->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

      render::Queue queue(*viewCamera);
      graph.enqueue(queue);
      queue.render();
    }
  }
  while (context->update());
}

void Demo::onRequestedWL(GL::Uniform& uniform)
{
  uniform.setValue(WL);
}

void Demo::onRequestedLight(GL::Uniform& uniform)
{
  uniform.setValue(lightCamera->getTransform().position);
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


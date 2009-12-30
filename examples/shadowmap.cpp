
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
  bool render(void);
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
  Time currentTime;
};

Demo::~Demo(void)
{
  GL::Renderer::destroy();
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

  if (!GL::Renderer::create(*context))
    return false;

  const unsigned int size = 512;

  depthmap = GL::Texture::createInstance(*context, Image(PixelFormat::DEPTH16, size, size), 0, "depthmap");
  if (!depthmap)
    return false;

  colormap = GL::Texture::createInstance(*context, Image(PixelFormat::RGBA8, size, size), 0, "colormap");
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
  graph.addNode(*meshNode);

  viewCamera = new render::Camera();
  viewCamera->setFOV(60.f);
  viewCamera->setAspectRatio(0.f);

  viewCameraNode = new scene::CameraNode();
  viewCameraNode->setCamera(viewCamera);
  viewCameraNode->getLocalTransform().position.z = radius * 2.f;
  graph.addNode(*viewCameraNode);

  lightCamera = new render::Camera();
  lightCamera->setFOV(60.f);
  lightCamera->setAspectRatio(1.f);
  lightCamera->setDepthRange(0.1f, radius * 6.f);

  lightCameraNode = new scene::CameraNode();
  lightCameraNode->setCamera(lightCamera);
  lightCameraNode->getLocalTransform().position.x = radius * 2.f;
  lightCameraNode->getLocalTransform().rotation.setAxisRotation(Vec3::Y, (float) M_PI / 2.f);
  graph.addNode(*lightCameraNode);

  timer.start();

  return true;
}

void Demo::run(void)
{
  GL::Context* context = GL::Context::get();

  do
  {
    currentTime = timer.getTime();

    meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);

    graph.setTimeElapsed(currentTime);

    // Render shadow map
    {
      context->setCurrentCanvas(*canvas);
      context->clearDepthBuffer();
      context->clearColorBuffer();

      render::Queue queue(*lightCamera);
      graph.enqueue(queue);
      queue.render("shadowmap");
    }

    Mat4 WL;
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

    if (render::Material* material = render::Material::findInstance("hills"))
    {
      if (render::Technique* technique = material->getActiveTechnique())
      {
        for (unsigned int i = 0;  i < technique->getPassCount();  i++)
        {
          render::Pass& pass = technique->getPass(i);
          if (!pass.getName().empty())
            continue;

          if (GL::Program* program = pass.getProgram())
          {
            const char* uniformName = "LV";
            if (program->findUniform(uniformName))
              pass.getUniformState(uniformName).setValue(WL);
          }
        }
      }
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



#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

class Demo : public Trackable
{
public:
  bool init(void);
private:
  bool render(void);
  Ptr<GL::VertexProgram> program;
  Ptr<GL::Shader> style;
  Ptr<GL::Camera> camera;
  Ptr<GL::Mesh> mesh;
};

bool Demo::init(void)
{
  GL::ContextMode mode(640, 480, 32, 32, 0, GL::ContextMode::WINDOWED);
  if (!GL::Context::create(mode))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Program");
  context->getRenderSignal().connect(*this, &Demo::render);

  program = GL::VertexProgram::createInstance(Path("program.vp"));
  if (!program)
    return false;

  camera = new GL::Camera();
  camera->getTransform().position.z = 5.f;

  style = new GL::Shader();
  GL::ShaderPass& pass = style->createPass();
  pass.setDefaultColor(ColorRGBA(0.f, 0.f, 1.f, 1.f));
  pass.setVertexProgramName(program->getName());

  Mesh* meshData = Mesh::createInstance(Path("cube.mesh"));
  if (!meshData)
    return false;

  meshData->collapseGeometries(style->getName());

  mesh = GL::Mesh::createInstance(*meshData);
  if (!mesh)
    return false;

  return true;
}

bool Demo::render(void)
{
  GL::ContextCanvas canvas;
  canvas.push();
  canvas.clearDepthBuffer();
  canvas.clearColorBuffer();

  camera->begin();
  mesh->render();
  camera->end();

  canvas.pop();
  return true;
}

int main()
{
  if (!initializeSystem())
    exit(1);

  Ptr<Demo> demo = new Demo();
  if (demo->init())
  {
    while (GL::Context::get()->update())
      ;
  }
  else
  {
    Log::writeError("Punt");
    exit(1);
  }

  demo = NULL;

  shutdownSystem();
  exit(0);
}


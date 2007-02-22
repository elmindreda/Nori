
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

#include "editor.h"

namespace
{

bool setup(void)
{
  new demo::EffectTemplate<CubeEffect>("cube");

  if (!GL::Renderer::create())
    return false;

  if (!UI::Renderer::create())
    return false;

  if (!demo::Show::create())
    return false;

  if (!demo::Editor::create())
    return false;

  return true;
}

}

CubeEffect::CubeEffect(demo::EffectType& type, const String& name):
  demo::Effect(type, name)
{
}

bool CubeEffect::init(void)
{
  cube = render::Mesh::readInstance("cube");
  if (!cube)
    return false;

  camera.setFOV(60.f);

  cameraNode = new render::CameraNode();
  cameraNode->setCameraName(camera.getName());
  cameraNode->getLocalTransform().position.z = cube->getBounds().radius * 1.5f;
  scene.addNode(*cameraNode);

  meshNode = new render::MeshNode();
  meshNode->setMeshName(cube->getName());
  scene.addNode(*meshNode);

  return true;
}

void CubeEffect::render(void) const
{
  GL::ScreenCanvas canvas;
  canvas.begin();

  render::Queue queue(camera);
  scene.enqueue(queue);

  camera.begin();
  queue.render();
  camera.end();

  canvas.end();
}

void CubeEffect::update(Time deltaTime)
{
  scene.setTimeElapsed(getTimeElapsed());
}

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    exit(1);

  GL::ContextMode mode(640, 480, 32, 32, 0, true);
  if (GL::Context::create(mode))
  {
    if (setup())
    {
      while (GL::Context::get()->update())
	;
    }

    demo::Editor::destroy();
    demo::Show::destroy();
  }
  else
    Log::writeError("Punt");

  wendy::shutdown();
  exit(0);
}


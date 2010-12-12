
#include <wendy/Wendy.h>

#include <cstdlib>

namespace
{

using namespace wendy;

class Test
{
public:
  ~Test(void);
  bool init(void);
  void run(void);
private:
  ResourceIndex index;
  Ref<render::Material> material;
  render::Camera camera;
  render::ParticleSystem system;
  Timer timer;
};

Test::~Test(void)
{
  system.setMaterial(NULL);
  material = NULL;

  render::GeometryPool::destroySingleton();
  GL::Context::destroySingleton();
}

bool Test::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
  {
    Log::writeError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Particles");

  if (!render::GeometryPool::createSingleton(*context))
  {
    Log::writeError("Failed to create OpenGL renderer");
    return false;
  }

  material = render::Material::read(*context, Path("particle.material"));
  if (!material)
  {
    Log::writeError("Failed to load material");
    return false;
  }

  render::DefaultParticleEmitter* emitter = new render::DefaultParticleEmitter();
  emitter->setEmissionRate(50.f);
  emitter->setColorRange(RandomRGBA(ColorRGBA(1.f, 1.f, 1.f, 1.f),
                                    ColorRGBA(0.2f, 0.2f, 0.2f, 1.f)));
  emitter->setVelocityRange(RandomRange(4.f, 6.f));
  emitter->setDurationRange(RandomRange(4.f, 6.f));
  emitter->setAngleRange(RandomRange(0.f, M_PI));
  emitter->setOriginVolume(RandomVolume(Vec3(0.f, 0.f, 0.f),
                                        Vec3(0.f, 0.f, 0.f)));
  emitter->setSizeRange(RandomRange(0.1f, 0.3f));

  render::PlanarGravityParticleAffector* affector = new render::PlanarGravityParticleAffector();
  affector->setGravity(Vec3(0.f, -1.f, 0.f));

  system.setMaterial(material);
  system.addEmitter(*emitter);
  system.addAffector(*affector);
  system.setParticleCount(1000);

  Transform3 transform;
  transform.position.z = 5.f;
  camera.setTransform(transform);
  camera.setAspectRatio(0.f);

  timer.start();
  return true;
}

void Test::run(void)
{
  GL::Context* context = GL::Context::getSingleton();

  render::Queue queue(camera);

  do
  {
    system.setTimeElapsed(timer.getTime());

    context->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));
    context->clearDepthBuffer();

    system.enqueue(queue, Transform3());
    queue.render();
    queue.removeOperations();
  }
  while (context->update());
}

} /*namespace*/

int main(void)
{
  if (!wendy::initialize())
  {
    Log::writeError("Failed to initialize Wendy");
    std::exit(EXIT_FAILURE);
  }

  Ptr<Test> test(new Test());
  if (!test->init())
  {
    Log::writeError("Failed to initialize test");
    std::exit(EXIT_FAILURE);
  }

  test->run();
  test = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}


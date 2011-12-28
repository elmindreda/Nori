
#include <wendy/WendyCore.h>
#include <wendy/WendyGL.h>
#include <wendy/WendyRender.h>

#include <cstdlib>

using namespace wendy;

namespace
{

const char* lines[] =
{
  "In A.D. 2101, war was beginning",
  "What happen?",
  "Somebody set up us the bomb.",
  "We get signal.",
  "What?",
  "Main screen turn on.",
  "It's you.",
  "How are you gentlemen?",
  "All your base are belong to us.",
  "You are on the way to destruction.",
  "What you say?",
  "You have no chance to survive make your time.",
  "Ha ha ha ....",
  NULL
};

class Test : public Trackable
{
public:
  ~Test();
  bool init();
  void run();
private:
  void onContextResized(unsigned int width, unsigned int height);
  ResourceCache cache;
  Ref<render::SharedProgramState> state;
  Ref<render::GeometryPool> pool;
  Ref<render::Font> font;
};

Test::~Test()
{
  font = NULL;
  pool = NULL;

  GL::Context::destroySingleton();
}

bool Test::init()
{
  const char* mediaPath = std::getenv("WENDY_MEDIA_DIR");
  if (!mediaPath)
    mediaPath = WENDY_MEDIA_DIR;

  if (!cache.addSearchPath(Path(mediaPath)))
    return false;

  if (!GL::Context::createSingleton(cache, GL::WindowConfig("Text Rendering Test")))
  {
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setRefreshMode(GL::Context::MANUAL_REFRESH);
  context->getResizedSignal().connect(*this, &Test::onContextResized);

  state = new render::SharedProgramState();
  state->reserveSupported(*context);
  context->setCurrentSharedProgramState(state);

  pool = render::GeometryPool::create(*context, 2048);

  font = render::Font::read(*pool, "wendy/UIDefault.font");
  if (!font)
  {
    logError("Failed to load font");
    return false;
  }

  return true;
}

void Test::run()
{
  GL::Context* context = GL::Context::getSingleton();

  do
  {
    context->clearColorBuffer();

    state->setOrthoProjectionMatrix(640.f, 480.f);

    const float em = font->getHeight();

    vec2 pen(100.f, 400.f);

    for (size_t i = 0;  lines[i];  i++)
    {
      font->drawText(pen, vec4(1.f), lines[i]);
      pen.y -= em * 1.5f;
    }
  }
  while (context->update());
}

void Test::onContextResized(unsigned int width, unsigned int height)
{
  GL::Context* context = GL::Context::getSingleton();
  context->setViewportArea(Recti(0, 0, width, height));
}

} /*namespace*/

int main()
{
  Ptr<Test> test(new Test());
  if (!test->init())
  {
    logError("Failed to initialize test");
    std::exit(EXIT_FAILURE);
  }

  test->run();
  test = NULL;

  std::exit(EXIT_SUCCESS);
}


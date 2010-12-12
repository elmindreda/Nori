
#include <wendy/Wendy.h>

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

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  bool render(void);
private:
  ResourceIndex index;
  Ref<render::Font> font;
};

Demo::~Demo(void)
{
  font = NULL;

  render::GeometryPool::destroySingleton();
  GL::Context::destroySingleton();
}

bool Demo::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
  {
    Log::writeError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Font Test");
  context->setRefreshMode(GL::Context::MANUAL_REFRESH);

  if (!render::GeometryPool::createSingleton(*context))
    return false;

  font = render::Font::read(*render::GeometryPool::getSingleton(),
                            Path("wendy/default.font"));
  if (!font)
  {
    Log::writeError("Failed to load font");
    return false;
  }

  return true;
}

bool Demo::render(void)
{
  GL::Context* context = GL::Context::getSingleton();
  context->clearColorBuffer();
  context->setProjectionMatrix2D(640.f, 480.f);

  const float em = font->getHeight();

  Vec2 pen(100.f, 400.f);

  for (size_t i = 0;  lines[i];  i++)
  {
    font->drawText(pen, ColorRGBA::WHITE, lines[i]);
    pen.y -= em * 1.5;
  }

  return true;
}

} /*namespace*/

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    std::exit(EXIT_FAILURE);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
  {
    do
    {
      demo->render();
    }
    while (GL::Context::getSingleton()->update());
  }

  demo = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}


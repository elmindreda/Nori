
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

namespace
{

class Demo : public Trackable
{
public:
  bool init(void);
private:
  bool render(void);
  Ptr<GL::Font> font;
};

bool Demo::init(void)
{
  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, GL::ContextMode::WINDOWED);

  if (!GL::Context::create(mode))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Fonts");
  context->getRenderSignal().connect(*this, &Demo::render);

  font = GL::Font::createInstance(Path("font.png"), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
  if (!font)
    return false;

  return true;
}

bool Demo::render(void)
{
  GL::ContextCanvas canvas;
  canvas.push();
  canvas.clearColorBuffer();
  canvas.begin2D(Vector2((float) canvas.getPhysicalWidth(),
                         (float) canvas.getPhysicalHeight()));

  String text = "All your base are belong to us";

  Vector2 pen(100.f, 100.f);

  font->setPenPosition(pen);
  font->setColor(ColorRGBA::WHITE);
  font->render(text);

  /*
  Rectangle area = font->getTextMetrics(text);

  GL::ShaderPass pass;
  pass.setPolygonMode(GL_LINE);
  pass.setDefaultColor(ColorRGBA::BLACK);
  pass.apply();

  glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);

  glBegin(GL_LINES);
  glVertex2f(0.f, pen.y);
  glVertex2f(canvas.getPhysicalWidth(), pen.y);
  glEnd();
  */

  canvas.end();
  canvas.pop();

  return true;
}

}

int main(int argc, char** argv)
{
  if (!initializeSystem())
    exit(1);

  Ptr<Demo> demo = new Demo();
  if (demo->init())
  {
    while (GL::Context::get()->update())
      ;
  }

  demo = NULL;

  shutdownSystem();
  exit(0);
}


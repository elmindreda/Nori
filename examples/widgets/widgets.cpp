
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

class Widgets : public Trackable
{
public:
  bool init(void);
  void render(void);
private:
  void onButtonPush(GL::Button& button);
  void onValueChange(GL::Slider& slider, float newValue);
  Ptr<GL::Window> window;
};

bool Widgets::init(void)
{
  window = new GL::Window("", "Hello");
  window->setArea(Rectangle(10, 10, 500, 500));

  GL::Button* button = new GL::Button("", "OK");
  button->setArea(Rectangle(10, 10, 300, 300));
  button->getPushedSignal().connect(*this, &Widgets::onButtonPush);
  window->addChildFirst(*button);

  GL::Slider* slider = new GL::Slider();
  slider->setArea(Rectangle(10, 400, 400, 20));
  slider->setOrientation(GL::Slider::HORIZONTAL);
  slider->getChangeValueSignal().connect(*this, &Widgets::onValueChange);
  window->addChildFirst(*slider);

  window->activate();

  return true;
}

void Widgets::render(void)
{
  GL::Widget::renderRoots();
}

void Widgets::onButtonPush(GL::Button& button)
{
  Log::writeInformation("Hallelujah");
}

void Widgets::onValueChange(GL::Slider& slider, float newValue)
{
  Log::writeInformation("New value: %0.2f", newValue);
}

int main()
{
  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, GL::ContextMode::WINDOWED);

  if (!initializeSystem())
    return 1;

  if (GL::Context::create(mode))
  {
    GL::Context::get()->setTitle("Widgets");

    Ptr<Widgets> widgets = new Widgets();
    if (widgets->init())
    { 
      while (GL::Context::get()->update())
      {
	glClearColor(1.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	widgets->render();
      }
    }

    GL::Context::destroy();
  }

  shutdownSystem();
  return 0;
}


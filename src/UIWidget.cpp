///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>
#include <wendy/GLFont.h>

#include <wendy/UIWidget.h>

#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Alignment::Alignment(HorzAlignment initHorizontal,
                     VertAlignment initVertical):
  horizontal(initHorizontal),
  vertical(initVertical)
{
}

void Alignment::set(HorzAlignment newHorizontal, VertAlignment newVertical)
{
  horizontal = newHorizontal;
  vertical = newVertical;
}

///////////////////////////////////////////////////////////////////////

void WidgetRenderer::drawText(const Rectangle& area,
                              const String& text,
		              const Alignment& alignment,
			      bool selected)
{
  Rectangle metrics = currentFont->getTextMetrics(text);

  Vector2 penPosition = currentFont->getPenPosition();

  switch (alignment.horizontal)
  {
    case LEFT_ALIGNED:
      penPosition.x += area.position.x - metrics.position.x;
      break;
    case CENTERED_ON_X:
      penPosition.x += area.getCenter().x - metrics.getCenter().x;
      break;
    case RIGHT_ALIGNED:
      penPosition.x += (area.position.x + area.size.x) -
                       (metrics.position.x + metrics.size.x);
      break;
    default:
      Log::writeError("Invalid horizontal alignment");
      return;
  }

  switch (alignment.vertical)
  {
    case BOTTOM_ALIGNED:
      penPosition.y += area.position.y - metrics.position.y;
      break;
    case CENTERED_ON_Y:
      penPosition.y += area.getCenter().y - metrics.getCenter().y;
      break;
    case TOP_ALIGNED:
      penPosition.y += (area.position.y + area.size.y) -
                 (metrics.position.y + metrics.size.y);
      break;
    default:
      Log::writeError("Invalid vertical alignment");
      return;
  }

  if (selected)
    currentFont->setColor(ColorRGBA(selectedTextColor, 1.f));
  else
    currentFont->setColor(ColorRGBA(textColor, 1.f));

  currentFont->setPenPosition(penPosition);
  currentFont->drawText(text);
}

void WidgetRenderer::drawFrame(const Rectangle& area, WidgetState state)
{
  GL::RenderPass pass;
  pass.setCullMode(GL::CULL_NONE);
  pass.setDepthTesting(false);
  pass.setDepthWriting(false);

  switch (state)
  {
    case STATE_ACTIVE:
      pass.setDefaultColor(ColorRGBA(widgetColor * 1.2f, 1.f));
      break;
    case STATE_DISABLED:
      pass.setDefaultColor(ColorRGBA(widgetColor * 0.8f, 1.f));
      break;
    default:
      pass.setDefaultColor(ColorRGBA(widgetColor, 1.f));
      break;
  }
      
  pass.apply();

  glRectf(area.position.x,
          area.position.y,
	  area.position.x + area.size.x,
	  area.position.y + area.size.y);
      
  pass.setPolygonMode(GL_LINE);
  pass.setDefaultColor(ColorRGBA::BLACK);
  pass.apply();

  glRectf(area.position.x,
          area.position.y,
	  area.position.x + area.size.x,
	  area.position.y + area.size.y);
}

void WidgetRenderer::drawTextFrame(const Rectangle& area, WidgetState state)
{
  GL::RenderPass pass;
  pass.setCullMode(GL::CULL_NONE);
  pass.setDepthTesting(false);
  pass.setDepthWriting(false);

  switch (state)
  {
    case STATE_ACTIVE:
      pass.setDefaultColor(ColorRGBA(textFrameColor * 1.2f, 1.f));
      break;
    case STATE_DISABLED:
      pass.setDefaultColor(ColorRGBA(textFrameColor * 0.8f, 1.f));
      break;
    default:
      pass.setDefaultColor(ColorRGBA(textFrameColor, 1.f));
      break;
  }
      
  pass.apply();

  glRectf(area.position.x,
          area.position.y,
	  area.position.x + area.size.x,
	  area.position.y + area.size.y);
      
  pass.setPolygonMode(GL_LINE);
  pass.setDefaultColor(ColorRGBA::BLACK);
  pass.apply();

  glRectf(area.position.x,
          area.position.y,
	  area.position.x + area.size.x,
	  area.position.y + area.size.y);
}

void WidgetRenderer::drawHandle(const Rectangle& area, WidgetState state)
{
  drawFrame(area, state);
}

void WidgetRenderer::drawButton(const Rectangle& area, WidgetState state, const String& text)
{
  drawFrame(area, state);

  if (text.length())
    drawText(area, text);
}

const ColorRGB& WidgetRenderer::getWidgetColor(void)
{
  return widgetColor;
}

const ColorRGB& WidgetRenderer::getTextColor(void)
{
  return textColor;
}

const ColorRGB& WidgetRenderer::getTextFrameColor(void)
{
  return textFrameColor;
}

const ColorRGB& WidgetRenderer::getSelectedTextColor(void)
{
  return selectedTextColor;
}

const ColorRGB& WidgetRenderer::getSelectionColor(void)
{
  return selectionColor;
}

GL::Font* WidgetRenderer::getCurrentFont(void)
{
  return currentFont;
}

GL::Font* WidgetRenderer::getDefaultFont(void)
{
  return defaultFont;
}

bool WidgetRenderer::create(void)
{
  Ptr<WidgetRenderer> renderer = new WidgetRenderer();
  if (!renderer->init())
    return false;

  set(renderer.detachObject());
  return true;
}

WidgetRenderer::WidgetRenderer(void):
  currentFont(NULL)
{
}

bool WidgetRenderer::init(void)
{
  if (!GL::Context::get())
  {
    Log::writeError("Cannot create the widget renderer without an OpenGL context");
    return false;
  }

  Font* fontData = Font::findInstance("default");
  if (!fontData)
    return false;

  defaultFont = GL::Font::createInstance(*fontData);
  if (!defaultFont)
    return false;

  currentFont = defaultFont;

  widgetColor.set(0.7f, 0.7f, 0.7f);
  textColor = ColorRGB::BLACK;
  textFrameColor = ColorRGB::WHITE;
  selectionColor.set(0.3f, 0.3f, 0.3f);
  selectedTextColor = ColorRGB::WHITE;

  return true;
}

///////////////////////////////////////////////////////////////////////

Widget::Widget(const String& name):
  Managed<Widget>(name),
  enabled(true),
  visible(true),
  underCursor(false)
{
  static bool initialized = false;

  if (!initialized)
  {
    GL::Context* context = GL::Context::get();
    if (!context)
      throw Exception("Cannot create UI widgets without an OpenGL context");

    context->getKeyPressSignal().connect(&Widget::onKeyPress);
    context->getButtonClickSignal().connect(&Widget::onButtonClick);
    context->getCursorMoveSignal().connect(&Widget::onCursorMove);

    initialized = true;
  }

  area.set(0.f, 0.f, 1.f, 1.f);

  roots.push_back(this);
}

Widget::~Widget(void)
{
  if (activeWidget == this)
  {
    if (Widget* parent = getParent())
      parent->activate();
    else if (!roots.empty())
      roots.back()->activate();
    else
    {
      changeFocusSignal.emit(*this, false);
      activeWidget = NULL;
    }
  }

  destroySignal.emit(*this);
}

Widget* Widget::findByPoint(const Vector2& point)
{
  if (!area.contains(point))
    return NULL;

  const Vector2 localPoint = point - area.position;

  for (Widget* child = getFirstChild();  child;  child = child->getNextSibling())
  {
    if (child->isVisible())
      if (Widget* result = child->findByPoint(localPoint))
	return result;
  }

  return this;
}

Vector2 Widget::transformToLocal(const Vector2& globalPoint) const
{
  return globalPoint - getGlobalArea().position;
}

Vector2 Widget::transformToGlobal(const Vector2& localPoint) const
{
  return localPoint + getGlobalArea().position;
}

void Widget::removeFromParent(void)
{
  Node<Widget>::removeFromParent();

  roots.push_back(this);
}

void Widget::enable(void)
{
  enabled = true;
}

void Widget::disable(void)
{
  enabled = false;
}

void Widget::show(void)
{
  visible = true;
}

void Widget::hide(void)
{
  visible = false;
}

void Widget::activate(void)
{
  if (activeWidget == this)
    return;

  if (activeWidget)
    activeWidget->changeFocusSignal.emit(*activeWidget, false);

  changeFocusSignal.emit(*this, true);
  activeWidget = this;
}

bool Widget::isEnabled(void) const
{
  return enabled;
}

bool Widget::isVisible(void) const
{
  return visible;
}

bool Widget::isActive(void) const
{
  return activeWidget == this;
}

bool Widget::isUnderCursor(void) const
{
  return underCursor;
}

bool Widget::isBeingDragged(void) const
{
  return draggedWidget == this;
}

WidgetState Widget::getState(void) const
{
  if (isEnabled())
  {
    if (isActive())
      return STATE_ACTIVE;
    else
      return STATE_NORMAL;
  }
  else
    return STATE_DISABLED;
}

const Rectangle& Widget::getArea(void) const
{
  return area;
}

const Rectangle& Widget::getGlobalArea(void) const
{
  globalArea = area;

  if (const Widget* parent = getParent())
    globalArea.position += parent->getGlobalArea().position;

  return globalArea;
}

void Widget::setArea(const Rectangle& newArea)
{
  changeAreaSignal.emit(*this, newArea);
  area = newArea;
}

void Widget::setSize(const Vector2& newSize)
{
  Rectangle newArea(area.position, newSize);
  changeAreaSignal.emit(*this, newArea);
  area.size = newSize;
}

void Widget::setPosition(const Vector2& newPosition)
{
  Rectangle newArea(newPosition, area.size);
  changeAreaSignal.emit(*this, newArea);
  area.position = newPosition;
}

SignalProxy1<void, Widget&> Widget::getDestroySignal(void)
{
  return destroySignal;
}

SignalProxy2<void, Widget&, const Rectangle&> Widget::getChangeAreaSignal(void)
{
  return changeAreaSignal;
}

SignalProxy2<void, Widget&, bool> Widget::getChangeFocusSignal(void)
{
  return changeFocusSignal;
}

SignalProxy3<void, Widget&, GL::Key, bool> Widget::getKeyPressSignal(void)
{
  return keyPressSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getCursorMoveSignal(void)
{
  return cursorMoveSignal;
}

SignalProxy4<void, Widget&, const Vector2&, unsigned int, bool> Widget::getButtonClickSignal(void)
{
  return buttonClickSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorEnterSignal(void)
{
  return cursorEnterSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorLeaveSignal(void)
{
  return cursorLeaveSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragBeginSignal(void)
{
  return dragBeginSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragMoveSignal(void)
{
  return dragMoveSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragEndSignal(void)
{
  return dragEndSignal;
}

Widget* Widget::getActive(void)
{
  return activeWidget;
}

void Widget::renderRoots(void)
{
  if (!WidgetRenderer::get())
  {
    Log::writeError("Cannot render widgets without a widget renderer");
    return;
  }

  GL::Canvas* canvas = GL::Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot render widgets without a current canvas");
    return;
  }

  canvas->begin2D(Vector2((float) canvas->getPhysicalWidth(),
                          (float) canvas->getPhysicalHeight()));

  for (WidgetList::iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->render();
  }

  canvas->end();
}

void Widget::render(void) const
{
  for (const Widget* child = getLastChild();  child;  child = child->getPrevSibling())
  {
    if (child->isVisible())
      child->render();
  }
}

void Widget::addedToParent(Widget& parent)
{
  roots.remove(this);
}

void Widget::onKeyPress(GL::Key key, bool pressed)
{
  switch (key)
  {
    default:
    {
      if (activeWidget)
        activeWidget->keyPressSignal.emit(*activeWidget, key, pressed);

      break;
    }
  }
}

void Widget::onCursorMove(const Vector2& position)
{
  if (draggedWidget)
  {
    if (dragging)
      draggedWidget->dragMoveSignal.emit(*draggedWidget, position);
    else
      draggedWidget->dragBeginSignal.emit(*draggedWidget, position);
  }
}

void Widget::onButtonClick(unsigned int button, bool clicked)
{
  GL::Context* context = GL::Context::get();

  Vector2 cursorPosition = context->getCursorPosition();
  cursorPosition.y = context->getHeight() - cursorPosition.y;

  if (clicked)
  {
    Widget* clickedWidget = NULL;

    for (WidgetList::iterator i = roots.begin();  i != roots.end();  i++)
    {
      if (clickedWidget = (*i)->findByPoint(cursorPosition))
	break;
    }

    while (clickedWidget && !clickedWidget->isEnabled())
      clickedWidget = clickedWidget->getParent();

    if (clickedWidget)
    {
      clickedWidget->activate();
      clickedWidget->buttonClickSignal.emit(*clickedWidget, cursorPosition, button, clicked);
    }
  }
  else
  {
    if (draggedWidget)
    {
      if (dragging)
      {
	draggedWidget->dragEndSignal.emit(*draggedWidget, cursorPosition);
	dragging = false;
      }

      draggedWidget = NULL;
    }

    if (activeWidget)
      activeWidget->buttonClickSignal.emit(*activeWidget, cursorPosition, button, clicked);
  }
}

bool Widget::dragging = false;

Widget::WidgetList Widget::roots;

Widget* Widget::activeWidget = NULL;
Widget* Widget::draggedWidget = NULL;

///////////////////////////////////////////////////////////////////////

Label::Label(const String& initText, const String& name):
  Widget(name),
  text(initText)
{
}

const String& Label::getText(void) const
{
  return text;
}

void Label::setText(const String& newText)
{
  changeTextSignal.emit(*this, newText);
  text = newText;
}

SignalProxy2<void, Label&, const String&> Label::getChangeTextSignal(void)
{
  return changeTextSignal;
}

void Label::render(void) const
{
  const Rectangle& area = getGlobalArea();

  WidgetRenderer::get()->drawText(area, text);
}

///////////////////////////////////////////////////////////////////////

Button::Button(const String& initText, const String& name):
  Widget(name),
  text(initText)
{
  GL::Font* font = WidgetRenderer::get()->getCurrentFont();

  float textWidth;

  if (text.empty())
    textWidth = font->getWidth() * 3.f;
  else
    textWidth = font->getTextSize(text).x;

  setSize(Vector2(font->getWidth() * 2.f + textWidth,
                  font->getHeight() * 2.f));

  getButtonClickSignal().connect(*this, &Button::onButtonClick);
  getKeyPressSignal().connect(*this, &Button::onKeyPress);
}

const String& Button::getText(void) const
{
  return text;
}

void Button::setText(const String& newText)
{
  changeTextSignal.emit(*this, newText);
  text = newText;
}

SignalProxy2<void, Button&, const String&> Button::getChangeTextSignal(void)
{
  return changeTextSignal;
}

SignalProxy1<void, Button&> Button::getPushedSignal(void)
{
  return pushedSignal;
}

void Button::render(void) const
{
  const Rectangle& area = getGlobalArea();

  WidgetRenderer::get()->drawButton(area, getState(), text);

  Widget::render();
}

void Button::onButtonClick(Widget& widget, const Vector2& position, unsigned int button, bool clicked)
{
  if (button == 0 && clicked == false && getGlobalArea().contains(position))
    pushedSignal.emit(*this);
}

void Button::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
  if (key == GL::Key::ENTER && pressed == true)
    pushedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

Entry::Entry(const String& initText, const String& name):
  Widget(name),
  text(initText)
{
  GL::Font* font = WidgetRenderer::get()->getCurrentFont();

  setSize(Vector2(font->getWidth() * 10.f,
                  font->getHeight() * 1.5f));

  getButtonClickSignal().connect(*this, &Entry::onButtonClick);
  getKeyPressSignal().connect(*this, &Entry::onKeyPress);
}

const String& Entry::getText(void) const
{
  return text;
}

void Entry::setText(const String& newText)
{
  changeTextSignal.emit(*this, newText);
  text = newText;
}

unsigned int Entry::getCaretPosition(void) const
{
  return caretPosition;
}

void Entry::setCaretPosition(unsigned int newPosition)
{
  caretMoveSignal.emit(*this, newPosition);
  caretPosition = newPosition;
}

SignalProxy2<void, Entry&, const String&> Entry::getChangeTextSignal(void)
{
  return changeTextSignal;
}

void Entry::render(void) const
{
  const Rectangle& area = getGlobalArea();

  WidgetRenderer::get()->drawTextFrame(area, getState());

  GL::Font* font = WidgetRenderer::get()->getCurrentFont();

  Rectangle textArea = area;
  textArea.position.x += font->getWidth() / 2.f;
  textArea.size.x -= font->getWidth();

  WidgetRenderer::get()->drawText(textArea, text, LEFT_ALIGNED);
}

void Entry::onButtonClick(Widget& widget,
		          const Vector2& position,
		          unsigned int button,
		          bool clicked)
{
}

void Entry::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
}

///////////////////////////////////////////////////////////////////////

Slider::Slider(Orientation initOrientation, const String& name):
  Widget(name),
  minValue(0.f),
  maxValue(1.f),
  value(0.f),
  orientation(initOrientation)
{
  getKeyPressSignal().connect(*this, &Slider::onKeyPress);
  getButtonClickSignal().connect(*this, &Slider::onButtonClick);
}

float Slider::getMinValue(void) const
{
  return minValue;
}

float Slider::getMaxValue(void) const
{
  return maxValue;
}

void Slider::setValueRange(float newMinValue, float newMaxValue)
{
  minValue = newMinValue;
  maxValue = newMaxValue;
}

float Slider::getValue(void) const
{
  return value;
}

void Slider::setValue(float newValue)
{
  changeValueSignal.emit(*this, newValue);
  value = newValue;
}

Slider::Orientation Slider::getOrientation(void) const
{
  return orientation;
}

void Slider::setOrientation(Orientation newOrientation)
{
  orientation = newOrientation;
}

SignalProxy2<void, Slider&, float> Slider::getChangeValueSignal(void)
{
  return changeValueSignal;
}

void Slider::render(void) const
{
  const Rectangle& area = getGlobalArea();

  WidgetRenderer::get()->drawFrame(area, getState());

  const float position = (value - minValue) / (maxValue - minValue);

  Rectangle handleArea;

  if (orientation == HORIZONTAL)
  {
    handleArea.set(area.position.x + position * area.size.x - 5.f,
                   area.position.y,
	           10.f,
	           area.size.y);
  }
  else
  {
    handleArea.set(area.position.x,
                   area.position.y + position * area.size.y - 5.f,
	           area.size.x,
	           10.f);
  }

  WidgetRenderer::get()->drawHandle(handleArea, getState());

  Widget::render();
}

void Slider::onButtonClick(Widget& widget,
	                   const Vector2& position,
	                   unsigned int button,
		           bool clicked)
{
  if (clicked)
  {
    Vector2 localPosition = transformToLocal(position);

    if (orientation == HORIZONTAL)
      setValue(minValue + (maxValue - minValue) * (localPosition.x / getArea().size.x));
    else
      setValue(minValue + (maxValue - minValue) * (localPosition.y / getArea().size.y));
  }
}

void Slider::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
  if (pressed)
  {
    switch (key)
    {
      case GL::Key::UP:
	setValue(value + 1.f);
	break;
      case GL::Key::DOWN:
	setValue(value - 1.f);
	break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

Item::Item(const String& initValue):
  value(initValue)
{
}

float Item::getHeight(void) const
{
  return WidgetRenderer::get()->getCurrentFont()->getHeight() * 1.5f;
}

const String& Item::getValue(void) const
{
  return value;
}

void Item::setValue(const String& newValue)
{
  value = newValue;
}

void Item::render(const Rectangle& area, bool selected)
{
  GL::Font* font = WidgetRenderer::get()->getCurrentFont();

  Rectangle textArea = area;
  textArea.position.x += font->getWidth() / 2.f;
  textArea.size.x -= font->getWidth();

  if (selected)
  {
    GL::RenderPass pass;
    pass.setDefaultColor(ColorRGBA(WidgetRenderer::get()->getSelectionColor(), 1.f));
    pass.setDepthTesting(false);
    pass.setDepthWriting(false);
    pass.apply();

    glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);
  }

  WidgetRenderer::get()->drawText(textArea, value, LEFT_ALIGNED, selected);
}

///////////////////////////////////////////////////////////////////////

List::List(const String& name):
  Widget(name),
  selection(0)
{
  getButtonClickSignal().connect(*this, &List::onButtonClick);
  getKeyPressSignal().connect(*this, &List::onKeyPress);
}

void List::insertItem(Item* item, unsigned int index)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), item);
  if (i != items.end())
    return;

  addItemSignal.emit(*this, *item);

  ItemList::iterator p = items.begin();
  std::advance(p, index);
  items.insert(p, item);
}

void List::removeItem(Item* item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), item);
  if (i != items.end())
  {
    removeItemSignal.emit(*this, **i);

    delete *i;
    items.erase(i);
  }
}

void List::removeItems(void)
{
  while (!items.empty())
  {
    removeItemSignal.emit(*this, *items.front());

    delete items.front();
    items.pop_front();
  }
}

void List::sortItems(void)
{
  // TODO: The code.
}

unsigned int List::getSelection(void) const
{
  return selection;
}

void List::setSelection(unsigned int newIndex)
{
  if (items.empty())
    return;

  newIndex = std::min(newIndex, (unsigned int) items.size() - 1);
  changeSelectionSignal.emit(*this, newIndex);
  selection = newIndex;
}

unsigned int List::getItemCount(void) const
{
  return (unsigned int) items.size();
}

Item* List::getItem(unsigned int index)
{
  if (index < items.size())
  {
    ItemList::iterator i = items.begin();
    std::advance(i, index);
    return *i;
  }

  return NULL;
}

const Item* List::getItem(unsigned int index) const
{
  if (index < items.size())
  {
    ItemList::const_iterator i = items.begin();
    std::advance(i, index);
    return *i;
  }

  return NULL;
}

SignalProxy2<void, List&, Item&> List::getAddItemSignal(void)
{
  return addItemSignal;
}

SignalProxy2<void, List&, Item&> List::getRemoveItemSignal(void)
{
  return removeItemSignal;
}

SignalProxy2<void, List&, unsigned int> List::getChangeSelectionSignal(void)
{
  return changeSelectionSignal;
}

void List::render(void) const
{
  const Rectangle& area = getGlobalArea();

  WidgetRenderer::get()->drawFrame(area, getState());

  float start = area.size.y;

  unsigned int index = 0;

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    float height = (*i)->getHeight();
    if (height + start < 0.f)
      break;

    Rectangle itemArea = area;
    itemArea.position.y += start - height;
    itemArea.size.y = height;

    (*i)->render(itemArea, index == selection);

    start -= height;
    index++;
  }

  Widget::render();
}

void List::onButtonClick(Widget& widget,
                         const Vector2& position,
		         unsigned int button,
		         bool clicked)
{
  if (!clicked || button != 0)
    return;

  Vector2 localPosition = transformToLocal(position);

  unsigned int index = 0;

  const float height = getArea().size.y;
  float itemTop = height;

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    const float itemHeight = (*i)->getHeight();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= localPosition.y)
    {
      setSelection(index);
      return;
    }

    itemTop -= itemHeight;
    index++;
  }
}

void List::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case GL::Key::UP:
    {
      if (selection > 0)
	setSelection(selection - 1);
      break;
    }

    case GL::Key::DOWN:
    {
      setSelection(selection + 1);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

Window::Window(const String& initTitle, const String& name):
  Widget(name),
  title(initTitle)
{
}

void Window::render(void) const
{
  GL::RenderPass pass;
  pass.setDepthTesting(false);
  pass.setDefaultColor(ColorRGBA::WHITE);
  pass.apply();

  const Rectangle& area = getGlobalArea();
  glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);

  Widget::render();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

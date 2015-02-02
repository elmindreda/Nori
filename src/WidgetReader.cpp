//////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
// Copyright (c) 2014 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <nori/Config.hpp>

#include <nori/Core.hpp>

#include <nori/Theme.hpp>
#include <nori/Layer.hpp>
#include <nori/Widget.hpp>
#include <nori/Label.hpp>
#include <nori/Button.hpp>
#include <nori/Progress.hpp>
#include <nori/Canvas.hpp>
#include <nori/Entry.hpp>
#include <nori/Book.hpp>
#include <nori/Scroller.hpp>
#include <nori/Slider.hpp>
#include <nori/Item.hpp>
#include <nori/List.hpp>
#include <nori/Menu.hpp>
#include <nori/Popup.hpp>
#include <nori/Layout.hpp>

#include <nori/WidgetReader.hpp>

#include <pugixml.hpp>

namespace nori
{

namespace
{

const uint WIDGETS_XML_VERSION = 1;

int alignmentCast(const std::string& name)
{
  static const std::regex regex("(left|center|right) (top|center|bottom)");

  int alignment = 0;
  std::smatch match;

  if (std::regex_match(name, match, regex))
  {
    if (match[1] == "left")
      alignment |= ALIGN_LEFT;
    else if (match[1] == "center")
      alignment |= ALIGN_CENTER;
    else if (match[1] == "right")
      alignment |= ALIGN_RIGHT;

    if (match[2] == "top")
      alignment |= ALIGN_TOP;
    else if (match[2] == "center")
      alignment |= ALIGN_MIDDLE;
    else if (match[2] == "bottom")
      alignment |= ALIGN_BOTTOM;
  }

  return alignment;
}

Orientation orientationCast(const std::string& name)
{
  if (name == "horizontal")
    return HORIZONTAL;
  else if (name == "vertical")
    return VERTICAL;
}

LayoutMode layoutModeCast(const std::string& name)
{
  if (name == "cover")
    return COVER_PARENT;
  else if (name == "wrap")
    return WRAP_CHILDREN;
  else if (name == "static")
    return STATIC_SIZE;
}

Widget* createBook(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  return new Book(layer, parent);
}

Widget* createCanvas(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  return new Canvas(layer, parent);
}

Widget* createCheckButton(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  CheckButton* check = new CheckButton(layer, parent, wn.attribute("text").value());

  if (pugi::xml_attribute a = wn.attribute("checked"))
    check->setChecked(a.as_bool());

  return check;
}

Widget* createEntry(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  return new Entry(layer, parent, wn.attribute("text").value());
}

Widget* createLabel(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  Label* label = new Label(layer, parent, wn.attribute("text").value());

  if (pugi::xml_attribute a = wn.attribute("alignment"))
    label->setTextAlignment(alignmentCast(a.value()));

  return label;
}

Widget* createLayout(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  Layout* layout = new Layout(layer, parent,
                              orientationCast(wn.attribute("orientation").value()),
                              layoutModeCast(wn.attribute("mode").value()));

  if (pugi::xml_attribute a = wn.attribute("border"))
    layout->setBorderSize(a.as_float());

  return layout;
}

Widget* createList(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  List* list = new List(layer, parent);

  if (pugi::xml_attribute a = wn.attribute("editable"))
    list->setEditable(a.as_bool());

  return list;
}

Widget* createMenu(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  return new Menu(layer);
}

Widget* createPage(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  if (Book* book = dynamic_cast<Book*>(parent))
    return new Page(layer, *book, wn.attribute("text").value());

  return nullptr;
}

Widget* createPopup(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  return new Popup(layer, parent);
}

Widget* createProgress(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  Progress* progress = new Progress(layer, parent,
                                    orientationCast(wn.attribute("orientation").value()));

  if (pugi::xml_attribute a = wn.attribute("min"))
    progress->setValueRange(a.as_float(), progress->maxValue());
  if (pugi::xml_attribute a = wn.attribute("max"))
    progress->setValueRange(progress->minValue(), a.as_float());
  if (pugi::xml_attribute a = wn.attribute("value"))
    progress->setValue(a.as_float());

  return progress;
}

Widget* createPushButton(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  return new PushButton(layer, parent, wn.attribute("text").value());
}

Widget* createScroller(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  Scroller* scroller = new Scroller(layer, parent,
                                    orientationCast(wn.attribute("orientation").value()));

  if (pugi::xml_attribute a = wn.attribute("min"))
    scroller->setValueRange(a.as_float(), scroller->maxValue());
  if (pugi::xml_attribute a = wn.attribute("max"))
    scroller->setValueRange(scroller->minValue(), a.as_float());
  if (pugi::xml_attribute a = wn.attribute("value"))
    scroller->setValue(a.as_float());

  return scroller;
}

Widget* createSlider(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  Slider* slider = new Slider(layer, parent,
                              orientationCast(wn.attribute("orientation").value()));

  if (pugi::xml_attribute a = wn.attribute("min"))
    slider->setValueRange(a.as_float(), slider->maxValue());
  if (pugi::xml_attribute a = wn.attribute("max"))
    slider->setValueRange(slider->minValue(), a.as_float());
  if (pugi::xml_attribute a = wn.attribute("value"))
    slider->setValue(a.as_float());
  if (pugi::xml_attribute a = wn.attribute("step"))
    slider->setStepSize(a.as_float());

  return slider;
}

Widget* createWidget(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  return new Widget(layer, parent);
}

Widget* createItem(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  if (ItemContainer* container = dynamic_cast<ItemContainer*>(parent))
  {
    container->addItem(*new Item(layer,
                                 wn.attribute("text").value(),
                                 wn.attribute("id").as_int()));
  }

  return nullptr;
}

Widget* createSeparatorItem(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  if (ItemContainer* container = dynamic_cast<ItemContainer*>(parent))
    container->addItem(*new SeparatorItem(layer));

  return nullptr;
}

} /*namespace*/

WidgetReader::WidgetReader(ResourceCache& cache):
  m_cache(cache)
{
  m_factories["book"] = createBook;
  m_factories["canvas"] = createCanvas;
  m_factories["check"] = createCheckButton;
  m_factories["entry"] = createEntry;
  m_factories["item"] = createItem;
  m_factories["label"] = createLabel;
  m_factories["layout"] = createLayout;
  m_factories["list"] = createList;
  m_factories["menu"] = createMenu;
  m_factories["page"] = createPage;
  m_factories["popup"] = createPopup;
  m_factories["progress"] = createProgress;
  m_factories["push"] = createPushButton;
  m_factories["scroller"] = createScroller;
  m_factories["separator"] = createSeparatorItem;
  m_factories["slider"] = createSlider;
  m_factories["widget"] = createWidget;
}

void WidgetReader::addFactory(const std::string& name, WidgetFactory factory)
{
  m_factories[name] = factory;
}

bool WidgetReader::read(Widget& parent, const std::string& name)
{
  return read(parent.layer(), &parent, name);
}

bool WidgetReader::read(Layer& layer, const std::string& name)
{
  return read(layer, nullptr, name);
}

bool WidgetReader::read(Layer& layer, Widget* parent, const std::string& name)
{
  const Path path = m_cache.findFile(name);

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load_file(path.name().c_str());
  if (!result)
  {
    logError("Failed to load widget tree %s: %s",
             name.c_str(),
             result.description());
    return false;
  }

  pugi::xml_node root = document.child("widgets");
  if (root.attribute("version").as_uint() != WIDGETS_XML_VERSION)
  {
    logError("Widget tree file format mismatch in %s", name.c_str());
    return false;
  }

  for (auto wn : root.children())
    read(layer, parent, wn);

  return true;
}

void WidgetReader::read(Layer& layer, Widget* parent, pugi::xml_node wn)
{
  auto entry = m_factories.find(wn.name());
  if (entry == m_factories.end())
  {
    logError("Cannot create unknown widget type %s", wn.name());
    return;
  }

  Widget* widget = entry->second(layer, parent, wn);
  if (!widget)
    return;

  if (pugi::xml_attribute a = wn.attribute("area"))
    widget->setArea(rectCast(a.value()));
  if (pugi::xml_attribute a = wn.attribute("desired"))
    widget->setDesiredSize(vec2Cast(a.value()));
  if (pugi::xml_attribute a = wn.attribute("focusable"))
    widget->setFocusable(a.as_bool());
  if (pugi::xml_attribute a = wn.attribute("draggable"))
    widget->setDraggable(a.as_bool());

  if (pugi::xml_attribute a = wn.attribute("enabled"))
  {
    if (a.as_bool())
      widget->enable();
    else
      widget->disable();
  }

  if (pugi::xml_attribute a = wn.attribute("visible"))
  {
    if (a.as_bool())
      widget->show();
    else
      widget->hide();
  }

  if (pugi::xml_attribute a = wn.attribute("name"))
    m_named[a.value()] = widget;

  for (auto cn : wn.children())
    read(layer, widget, cn);
}

} /*namespace nori*/


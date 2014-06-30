///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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
#ifndef WENDY_WIDGETREADER_HPP
#define WENDY_WIDGETREADER_HPP
///////////////////////////////////////////////////////////////////////

#include <map>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

typedef Widget* (*WidgetFactory)(Layer&,Widget*,pugi::xml_node);

///////////////////////////////////////////////////////////////////////

class WidgetReader
{
public:
  WidgetReader(ResourceCache& cache);
  void addFactory(const String& name, WidgetFactory factory);
  bool read(Layer& layer, const String& name);
  template <typename T>
  T* find(const String& name)
  {
    auto entry = m_named.find(name);
    if (entry == m_named.end())
      return nullptr;

    return dynamic_cast<T*>(entry->second);
  }
private:
  void read(Layer& layer, Widget* parent, pugi::xml_node wn);
  ResourceCache& m_cache;
  std::map<String,WidgetFactory> m_factories;
  std::map<String,Widget*> m_named;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WIDGETREADER_HPP*/
///////////////////////////////////////////////////////////////////////

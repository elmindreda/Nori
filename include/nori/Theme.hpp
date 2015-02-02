///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#pragma once

#include <nori/Core.hpp>

#include <nori/VectorContext.hpp>

namespace nori
{

/*! @ingroup ui
 */
enum WidgetState
{
  STATE_DISABLED,
  STATE_NORMAL,
  STATE_ACTIVE,
  STATE_SELECTED
};

/*! @ingroup ui
 */
enum Orientation
{
  HORIZONTAL,
  VERTICAL
};

/*! @ingroup ui
 */
class Theme
{
public:
  ~Theme();
  void beginLayer();
  void endLayer();
  bool pushClipArea(Rect area);
  void popClipArea();
  void drawText(Rect area, WidgetState state, int alignment, const char* text);
  void drawWell(Rect area, WidgetState state);
  void drawFrame(Rect area, WidgetState state);
  void drawHandle(Rect area, WidgetState state);
  void drawPushButton(Rect area, WidgetState state, const char* text);
  void drawCheckButton(Rect area, WidgetState state, bool checked, const char* text);
  void drawPopup(Rect area, WidgetState state, const char* text);
  void drawTab(Rect area, WidgetState state, const char* text);
  void drawSelection(Rect area, WidgetState state);
  float em() const { return m_em; }
  VectorContext& context() const { return m_vc; }
  static std::unique_ptr<Theme> create(VectorContext& context);
private:
  Theme(VectorContext& context);
  bool init();
  VectorContext& m_vc;
  RectClipStackf m_stack;
  int m_font;
  float m_em;
};

} /*namespace nori*/


///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Core.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>
#include <wendy/Pass.hpp>
#include <wendy/Font.hpp>

namespace wendy
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
enum HorzAlignment
{
  LEFT_ALIGNED,
  RIGHT_ALIGNED,
  CENTERED_ON_X
};

/*! @ingroup ui
 */
enum VertAlignment
{
  TOP_ALIGNED,
  BOTTOM_ALIGNED,
  CENTERED_ON_Y
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
class Alignment
{
public:
  Alignment(HorzAlignment horizontal = CENTERED_ON_X,
            VertAlignment vertical = CENTERED_ON_Y);
  void set(HorzAlignment newHorizontal, VertAlignment newVertical);
  HorzAlignment horizontal;
  VertAlignment vertical;
};

/*! @ingroup ui
 */
class Theme : public Resource, public RefObject
{
  friend class Drawer;
  friend class ThemeReader;
public:
  Theme(const ResourceInfo& info);
  float em() const { return m_font->height(); }
  Font& font() const { return *m_font; }
  vec3 textColor(WidgetState state) const { return m_textColors[state]; }
  vec3 backgroundColor(WidgetState state) const { return m_backColors[state]; }
  vec3 caretColor(WidgetState state) const { return m_caretColors[state]; }
  static Ref<Theme> read(RenderContext& context, const String& name);
private:
  Rect m_buttonElements[4];
  Rect m_handleElements[4];
  Rect m_frameElements[4];
  Rect m_wellElements[4];
  Rect m_tabElements[4];
  Rect m_checkElements[4];
  Rect m_clearElements[4];
  vec3 m_textColors[4];
  vec3 m_backColors[4];
  vec3 m_caretColors[4];
  Ref<Texture> m_texture;
  Ref<Font> m_font;
};

/*! @ingroup ui
 */
class ThemeReader : public ResourceReader<Theme>
{
public:
  ThemeReader(RenderContext& context);
  using ResourceReader<Theme>::read;
  Ref<Theme> read(const String& name, const Path& path);
private:
  RenderContext& m_context;
};

/*! @brief User interface renderer.
 *  @ingroup ui
 *
 *  This class provides drawing for widgets.
 */
class Drawer
{
public:
  void begin();
  void end();
  /*! Pushes a clipping area onto the clip stack. The current
   *  clipping area then becomes the specified area as clipped by the
   *  previously current clipping area.
   *  @param area The desired clipping area.
   *  @return @c true if successful, or @c false if the specified area
   *  would result in a null total clipping area.
   *  @remarks If the resulting clipping area is empty, it is not pushed
   *  onto the stack, so you do not need to (and should not) pop it. The
   *  recommended pattern is:
   *  @code
   *  if (drawer.pushClipArea(childArea))
   *  {
   *    drawStuff();
   *    drawer.popClipArea();
   *  }
   *  @endcode
   */
  bool pushClipArea(const Rect& area);
  /*! Pops the top clipping area from the clip stack, restoring the
   *  previously current clipping area.
   */
  void popClipArea();
  void drawPoint(vec2 point, vec4 color);
  void drawLine(vec2 start, vec2 end, vec4 color);
  void drawRectangle(const Rect& rectangle, vec4 color);
  void fillRectangle(const Rect& rectangle, vec4 color);
  void blitTexture(const Rect& area, Texture& texture, vec4 color);
  void drawText(const Rect& area,
                const char* text,
                Alignment alignment,
                vec3 color);
  void drawText(const Rect& area,
                const char* text,
                Alignment alignment,
                WidgetState state);
  void drawWell(const Rect& area, WidgetState state);
  void drawFrame(const Rect& area, WidgetState state);
  void drawHandle(const Rect& area, WidgetState state);
  void drawButton(const Rect& area, WidgetState state, const char* text = "");
  void drawCheck(const Rect& area, WidgetState state, bool checked, const char* text = "");
  void drawTab(const Rect& area, WidgetState state, const char* text = "");
  const Theme& theme() const;
  RenderContext& context();
  Font& currentFont();
  void setCurrentFont(Font* newFont);
  static Drawer* create(RenderContext& context);
private:
  Drawer(RenderContext& context);
  bool init();
  void drawElement(const Rect& area, const Rect& mapping);
  void setDrawingState(vec4 color, bool wireframe);
  RectClipStackf m_clipAreaStack;
  Ref<VertexBuffer> m_vertexBuffer;
  Ref<IndexBuffer> m_indexBuffer;
  PrimitiveRange m_range;
  Ref<Theme> m_theme;
  RenderContext& m_context;
  Ref<Font> m_font;
  Pass m_drawPass;
  Pass m_blitPass;
  Pass m_elementPass;
  UniformStateIndex m_elementPosIndex;
  UniformStateIndex m_elementSizeIndex;
  UniformStateIndex m_texPosIndex;
  UniformStateIndex m_texSizeIndex;
  Ref<SharedProgramState> m_state;
};

} /*namespace wendy*/


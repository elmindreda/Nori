///////////////////////////////////////////////////////////////////////
// Wendy user interface library
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
#ifndef WENDY_UIDRAWER_H
#define WENDY_UIDRAWER_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Bezier.h>
#include <wendy/Segment.h>
#include <wendy/Triangle.h>

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderState.h>
#include <wendy/RenderSystem.h>
#include <wendy/RenderFont.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum WidgetState
{
  STATE_DISABLED,
  STATE_NORMAL,
  STATE_ACTIVE,
  STATE_SELECTED
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum HorzAlignment
{
  LEFT_ALIGNED,
  RIGHT_ALIGNED,
  CENTERED_ON_X
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum VertAlignment
{
  TOP_ALIGNED,
  BOTTOM_ALIGNED,
  CENTERED_ON_Y
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum Orientation
{
  HORIZONTAL,
  VERTICAL
};

///////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Theme : public Resource
{
public:
  Theme(const ResourceInfo& info);
  static Ref<Theme> read(render::GeometryPool& pool, const String& name);
  Rect buttonElements[4];
  Rect handleElements[4];
  Rect frameElements[4];
  Rect wellElements[4];
  Rect tabElements[4];
  vec3 textColors[4];
  vec3 backColors[4];
  vec3 caretColors[4];
  Ref<GL::Texture> texture;
  Ref<render::Font> font;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class ThemeReader : public ResourceReader<Theme>
{
public:
  ThemeReader(render::GeometryPool& pool);
  using ResourceReader<Theme>::read;
  Ref<Theme> read(const String& name, const Path& path);
private:
  Ref<render::GeometryPool> pool;
};

///////////////////////////////////////////////////////////////////////

/*! @brief User interface renderer.
 *  @ingroup ui
 *
 *  This class provides drawing for widgets.
 */
class Drawer : public RefObject
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
  void drawPoint(const vec2& point, const vec4& color);
  void drawLine(const Segment2& segment, const vec4& color);
  void drawTriangle(const Triangle2& triangle, const vec4& color);
  void drawBezier(const BezierCurve2& spline, const vec4& color);
  void drawRectangle(const Rect& rectangle, const vec4& color);
  void fillRectangle(const Rect& rectangle, const vec4& color);
  void fillTriangle(const Triangle2& triangle, const vec4& color);
  void blitTexture(const Rect& area, GL::Texture& texture);
  void drawText(const Rect& area,
                const char* text,
                const Alignment& alignment,
                const vec3& color);
  void drawText(const Rect& area,
                const char* text,
                const Alignment& alignment,
                WidgetState state);
  void drawWell(const Rect& area, WidgetState state);
  void drawFrame(const Rect& area, WidgetState state);
  void drawHandle(const Rect& area, WidgetState state);
  void drawButton(const Rect& area, WidgetState state, const char* text = "");
  void drawTab(const Rect& area, WidgetState state, const char* text = "");
  const Theme& getTheme() const;
  GL::Context& getContext();
  render::GeometryPool& getGeometryPool();
  render::Font& getCurrentFont();
  void setCurrentFont(render::Font* newFont);
  float getCurrentEM() const;
  static Ref<Drawer> create(render::GeometryPool& pool);
private:
  Drawer(render::GeometryPool& pool);
  bool init();
  void drawElement(const Rect& area, const Rect& mapping);
  void setDrawingState(const vec4& color, bool wireframe);
  RectClipStackf clipAreaStack;
  Ref<GL::VertexBuffer> vertexBuffer;
  Ref<GL::IndexBuffer> indexBuffer;
  GL::PrimitiveRange range;
  Ref<Theme> theme;
  Ref<render::GeometryPool> pool;
  Ref<render::Font> currentFont;
  render::Pass drawPass;
  render::Pass blitPass;
  render::Pass elementPass;
  render::UniformStateIndex elementPosIndex;
  render::UniformStateIndex elementSizeIndex;
  render::UniformStateIndex texPosIndex;
  render::UniformStateIndex texSizeIndex;
  Ref<render::SharedProgramState> state;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIDRAWER_H*/
///////////////////////////////////////////////////////////////////////

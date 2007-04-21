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
#ifndef WENDY_UIRENDERER_H
#define WENDY_UIRENDERER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum WidgetState
{
  STATE_DISABLED,
  STATE_NORMAL,
  STATE_ACTIVE,
  STATE_SELECTED,
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum HorzAlignment
{
  LEFT_ALIGNED,
  RIGHT_ALIGNED,
  CENTERED_ON_X,
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum VertAlignment
{
  TOP_ALIGNED,
  BOTTOM_ALIGNED,
  CENTERED_ON_Y,
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
enum Orientation
{
  HORIZONTAL,
  VERTICAL,
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

/*! @brief Widget renderer singleton.
 *  @ingroup ui
 *
 *  This class implements the default rendering behavior for widgets.
 * 
 *  @remarks This should probable be made overridable at some point.
 */
class Renderer : public Singleton<Renderer>
{
public:
  /*! Pushes a clipping area onto the clip stack. The current
   *  clipping area then becomes the specified area as clipped by the
   *  previously current clipping area.
   *  @param area The desired clipping area.
   *  @return @c true if successful, or @c false if the specified area
   *  would result in a null total clipping area.
   */
  bool pushClipArea(const Rectangle& area);
  /*! Pops the top clipping area from the clip stack, restoring the
   *  previously current clipping area.
   */
  void popClipArea(void);
  void drawText(const Rectangle& area,
                const String& text,
		const Alignment& alignment = Alignment(),
		bool selected = false);
  void drawWell(const Rectangle& area, WidgetState state);
  void drawFrame(const Rectangle& area, WidgetState state);
  void drawHandle(const Rectangle& area, WidgetState state);
  void drawButton(const Rectangle& area, WidgetState state, const String& text = "");
  const ColorRGB& getWidgetColor(void);
  const ColorRGB& getTextColor(void);
  const ColorRGB& getWellColor(void);
  const ColorRGB& getSelectionColor(void);
  const ColorRGB& getSelectedTextColor(void);
  render::Font* getDefaultFont(void);
  render::Font* getCurrentFont(void);
  float getDefaultEM(void) const;
  float getCurrentEM(void) const;
  static bool create(void);
private:
  Renderer(void);
  bool init(void);
  static void onContextDestroy(void);
  ColorRGB widgetColor;
  ColorRGB textColor;
  ColorRGB wellColor;
  ColorRGB selectionColor;
  ColorRGB selectedTextColor;
  Ref<render::Font> defaultFont;
  render::Font* currentFont;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIRENDERER_H*/
///////////////////////////////////////////////////////////////////////

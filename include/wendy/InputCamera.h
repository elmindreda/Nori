///////////////////////////////////////////////////////////////////////
// Wendy input library
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_INPUTCAMERA_H
#define WENDY_INPUTCAMERA_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace input
  {

///////////////////////////////////////////////////////////////////////

class MayaCamera : public Focus
{
public:
  MayaCamera(void);
  void onKeyPressed(Key key, bool pressed);
  void onButtonClicked(Button button, bool clicked);
  void onCursorMoved(const Vec2i& position);
  void onWheelTurned(int offset);
  void onFocusChanged(bool activated);
  const Transform3& getTransform(void) const;
private:
  enum Mode
  {
    NONE,
    TUMBLE,
    TRACK,
    DOLLY,
  };
  void updateTransform(void);
  Transform3 transform;
  Vec2i lastPosition;
  Vec3 target;
  float angleX;
  float angleY;
  float distance;
  Mode mode;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace input*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_INPUTCAMERA_H*/
///////////////////////////////////////////////////////////////////////

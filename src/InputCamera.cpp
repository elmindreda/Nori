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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Vector.h>
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>

#include <wendy/GLContext.h>

#include <wendy/Input.h>
#include <wendy/InputCamera.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace input
  {

///////////////////////////////////////////////////////////////////////

MayaCamera::MayaCamera(void):
  target(Vec3::ZERO),
  angleX(0.f),
  angleY(0.f),
  distance(5.f),
  mode(NONE)
{
  updateTransform();
}

void MayaCamera::onKeyPressed(Key key, bool pressed)
{
}

void MayaCamera::onButtonClicked(Button button, bool clicked)
{
  Context* context = Context::get();

  if (clicked)
  {
    switch (button)
    {
      case Button::LEFT:
        mode = TUMBLE;
        break;
      case Button::MIDDLE:
        mode = TRACK;
        break;
      case Button::RIGHT:
        mode = DOLLY;
        break;
    }

    if (mode != NONE)
      context->captureCursor();
  }
  else
  {
    mode = NONE;
    context->releaseCursor();
  }
}

void MayaCamera::onCursorMoved(const Vec2i& position)
{
  Vec2i offset = position - lastPosition;

  if (mode == TUMBLE)
  {
    angleY += offset.x / 250.f;
    angleX += offset.y / 250.f;
    updateTransform();
  }
  else if (mode == TRACK)
  {
    Vec3 axisX = Vec3::X;
    Vec3 axisY = Vec3::Y;

    transform.rotation.rotateVector(axisX);
    transform.rotation.rotateVector(axisY);

    target -= axisX * offset.x / 50.f;
    target += axisY * offset.y / 50.f;
    updateTransform();
  }

  lastPosition = position;
}

void MayaCamera::onWheelTurned(int offset)
{
  distance += (float) offset;
  updateTransform();
}

void MayaCamera::onFocusChanged(bool activated)
{
}

const Transform3& MayaCamera::getTransform(void) const
{
  return transform;
}

void MayaCamera::updateTransform(void)
{
  Vec3 offset = Vec3::Z * distance;

  transform.rotation.setEulerRotation(Vec3(angleX, angleY, 0.f));
  transform.rotation.rotateVector(offset);

  transform.position = target + offset;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace input*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#include <wendy/Block.h>
#include <wendy/Vector.h>
#include <wendy/Bezier.h>
#include <wendy/Quaternion.h>
#include <wendy/Matrix.h>
#include <wendy/Transform.h>
#include <wendy/Managed.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/Resource.h>
#include <wendy/Animation.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

KeyFrame3::KeyFrame3(Anim3& initAnimation):
  animation(&initAnimation)
{
}

bool KeyFrame3::operator < (const KeyFrame3& other) const
{
  return moment < other.moment;
}

Time KeyFrame3::getMoment(void) const
{
  return moment;
}

void KeyFrame3::setMoment(Time newMoment)
{
  moment = newMoment;
  animation->sortKeyFrames();
}

const Transform3& KeyFrame3::getTransform(void) const
{
  return transform;
}

void KeyFrame3::setTransform(const Transform3& newTransform)
{
  transform = newTransform;
}

void KeyFrame3::setPosition(const Vec3& newPosition)
{
  transform.position = newPosition;
}

void KeyFrame3::setRotation(const Quat& newRotation)
{
  transform.rotation = newRotation;
}

///////////////////////////////////////////////////////////////////////

Anim3::Anim3(const String& name):
  Resource<Anim3>(name)
{
}

void Anim3::createKeyFrame(Time moment,
                           const Transform3& transform,
                           const Vec3& direction)
{
  moment = std::max(moment, 0.0);

  keyframes.push_back(KeyFrame3(*this));
  keyframes.back().setTransform(transform);
  keyframes.back().setDirection(direction);
  keyframes.back().setMoment(moment);
}

void Anim3::destroyKeyFrame(KeyFrame3& frame)
{
  for (KeyFrameList::iterator f = keyframes.begin();  f != keyframes.end();  f++)
  {
    if (&(*f) == &frame)
    {
      keyframes.erase(f);
      break;
    }
  }
}

void Anim3::destroyKeyFrames(void)
{
  keyframes.clear();
}

void Anim3::evaluate(Time moment, Transform3& result) const
{
  if (keyframes.empty())
  {
    result.setIdentity();
    return;
  }

  moment = std::max(moment, 0.0);

  size_t index;

  for (index = 0;  index < keyframes.size();  index++)
  {
    if (keyframes[index].getMoment() > moment)
      break;
  }

  if (index == 0)
  {
    result = keyframes.front().getTransform();
    return;
  }

  if (index == keyframes.size())
  {
    result = keyframes.back().getTransform();
    return;
  }

  const KeyFrame3& startFrame = keyframes[index - 1];
  const KeyFrame3& endFrame = keyframes[index];

  const float t = (moment - startFrame.getMoment()) /
                  (endFrame.getMoment() - startFrame.getMoment());

  const Quat& startRot = startFrame.getTransform().rotation;
  const Quat& endRot = endFrame.getTransform().rotation;

  result.rotation = startRot.interpolateTo(t, endRot);

  BezierCurve3 curve;
  curve.P[0] = startFrame.getTransform().position;
  curve.P[1] = startFrame.getDirection();
  curve.P[2] = -endFrame.getDirection();
  curve.P[3] = endFrame.getTransform().position;

  result.position = curve(t);
}

size_t Anim3::getKeyFrameCount(void) const
{
  return keyframes.size();
}

KeyFrame3& Anim3::getKeyFrame(size_t index)
{
  return keyframes[index];
}

const KeyFrame3& Anim3::getKeyFrame(size_t index) const
{
  return keyframes[index];
}

void Anim3::sortKeyFrames(void)
{
  std::stable_sort(keyframes.begin(), keyframes.end());
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

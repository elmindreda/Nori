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
#ifndef WENDY_ANIMATION_H
#define WENDY_ANIMATION_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Anim3;

///////////////////////////////////////////////////////////////////////

class KeyFrame3
{
  friend class Anim3;
public:
  KeyFrame3(Anim3& animation);
  bool operator < (const KeyFrame3& other) const;
  Time getMoment(void) const;
  void setMoment(Time newMoment);
  const Transform3& getTransform(void) const;
  const Vec3& getDirection(void) const;
  void setTransform(const Transform3& newTransform);
  void setPosition(const Vec3& newPosition);
  void setRotation(const Quat& newRotation);
  void setDirection(const Vec3& newDirection);
private:
  Anim3* animation;
  Transform3 transform;
  Vec3 direction;
  Time moment;
};

///////////////////////////////////////////////////////////////////////

class Anim3 : public Resource<Anim3>
{
  friend class KeyFrame3;
public:
  Anim3(const String& name = "");
  Anim3(const Anim3& source);
  void createKeyFrame(Time moment, const Transform3& transform, const Vec3& direction);
  void destroyKeyFrame(KeyFrame3& frame);
  void destroyKeyFrames(void);
  void evaluate(Time moment, Transform3& result) const;
  Anim3& operator = (const Anim3& source);
  size_t getKeyFrameCount(void) const;
  KeyFrame3& getKeyFrame(size_t index);
  const KeyFrame3& getKeyFrame(size_t index) const;
  Time getDuration(void) const;
private:
  typedef std::vector<KeyFrame3> KeyFrameList;
  void sortKeyFrames(void);
  KeyFrameList keyframes;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ANIMATION_H*/
///////////////////////////////////////////////////////////////////////

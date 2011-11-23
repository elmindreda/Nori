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
#include <wendy/Bezier.h>
#include <wendy/Transform.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>
#include <wendy/Animation.h>

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int ANIM3_XML_VERSION = 1;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

KeyFrame3::KeyFrame3(AnimTrack3& initTrack):
  track(&initTrack)
{
}

bool KeyFrame3::operator < (const KeyFrame3& other) const
{
  return moment < other.moment;
}

Time KeyFrame3::getMoment() const
{
  return moment;
}

void KeyFrame3::setMoment(Time newMoment)
{
  moment = newMoment;
  track->sortKeyFrames();
  track->flipRotations();
}

const Transform3& KeyFrame3::getTransform() const
{
  return transform;
}

const vec3& KeyFrame3::getDirection() const
{
  return direction;
}

void KeyFrame3::setTransform(const Transform3& newTransform)
{
  transform = newTransform;
}

void KeyFrame3::setPosition(const vec3& newPosition)
{
  transform.position = newPosition;
}

void KeyFrame3::setRotation(const quat& newRotation)
{
  transform.rotation = newRotation;
}

void KeyFrame3::setDirection(const vec3& newDirection)
{
  direction = newDirection;
}

///////////////////////////////////////////////////////////////////////

AnimTrack3::AnimTrack3(const char* initName):
  name(initName)
{
}

AnimTrack3::AnimTrack3(const AnimTrack3& source)
{
  operator = (source);
}

void AnimTrack3::createKeyFrame(Time moment,
                                const Transform3& transform,
                                const vec3& direction)
{
  moment = max(moment, 0.0);

  keyframes.push_back(KeyFrame3(*this));
  keyframes.back().transform = transform;
  keyframes.back().direction = direction;
  keyframes.back().setMoment(moment);
}

void AnimTrack3::destroyKeyFrame(KeyFrame3& frame)
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

void AnimTrack3::destroyKeyFrames()
{
  keyframes.clear();
}

void AnimTrack3::evaluate(Time moment, Transform3& result) const
{
  if (keyframes.empty())
  {
    result.setIdentity();
    return;
  }

  moment = max(moment, 0.0);

  size_t index;

  for (index = 0;  index < keyframes.size();  index++)
  {
    if (keyframes[index].moment > moment)
      break;
  }

  if (index == 0)
  {
    result = keyframes.front().transform;
    return;
  }

  if (index == keyframes.size())
  {
    result = keyframes.back().transform;
    return;
  }

  const KeyFrame3& startFrame = keyframes[index - 1];
  const KeyFrame3& endFrame = keyframes[index];

  const float t = (float) ((moment - startFrame.moment) /
                           (endFrame.moment - startFrame.moment));

  const quat& startRot = startFrame.transform.rotation;
  const quat& endRot = endFrame.transform.rotation;

  result.rotation = mix(startRot, endRot, t);

  BezierCurve3 curve;
  curve.P[0] = startFrame.transform.position;
  curve.P[1] = startFrame.direction;
  curve.P[2] = -endFrame.direction;
  curve.P[3] = endFrame.transform.position;

  result.position = curve(t);
}

AnimTrack3& AnimTrack3::operator = (const AnimTrack3& source)
{
  keyframes.reserve(source.keyframes.size());

  for (KeyFrameList::const_iterator f = source.keyframes.begin();  f != source.keyframes.end();  f++)
  {
    keyframes.push_back(KeyFrame3(*this));
    keyframes.back().transform = f->transform;
    keyframes.back().direction = f->direction;
    keyframes.back().moment = f->moment;
  }

  name = source.name;
  return *this;
}

size_t AnimTrack3::getKeyFrameCount() const
{
  return keyframes.size();
}

KeyFrame3& AnimTrack3::getKeyFrame(size_t index)
{
  return keyframes[index];
}

const KeyFrame3& AnimTrack3::getKeyFrame(size_t index) const
{
  return keyframes[index];
}

const String& AnimTrack3::getName() const
{
  return name;
}

Time AnimTrack3::getDuration() const
{
  if (keyframes.empty())
    return 0.0;

  return keyframes.back().moment;
}

float AnimTrack3::getLength(float tolerance) const
{
  if (keyframes.size() < 2)
    return 0.f;

  float length = 0.f;
  BezierCurve3 curve;

  for (size_t i = 1;  i < keyframes.size();  i++)
  {
    curve.P[0] = keyframes[i - 1].transform.position;
    curve.P[1] = keyframes[i - 1].direction;
    curve.P[2] = -keyframes[i].direction;
    curve.P[3] = keyframes[i].transform.position;

    length += curve.length(tolerance);
  }

  return length;
}

void AnimTrack3::sortKeyFrames()
{
  std::stable_sort(keyframes.begin(), keyframes.end());
}

void AnimTrack3::flipRotations()
{
  for (size_t i = 1;  i < keyframes.size();  i++)
  {
    quat& R0 = keyframes[i - 1].transform.rotation;
    quat& R1 = keyframes[i].transform.rotation;

    if (dot(R0, R1) < 0.f)
      R1 = -R1;
  }
}

///////////////////////////////////////////////////////////////////////

Anim3::Anim3(const ResourceInfo& info):
  Resource(info)
{
}

Anim3::Anim3(const Anim3& source):
  Resource(source)
{
  operator = (source);
}

Anim3& Anim3::operator = (const Anim3& source)
{
  destroyTracks();
  tracks.reserve(source.tracks.size());

  for (TrackList::const_iterator t = tracks.begin();  t != tracks.end();  t++)
    tracks.push_back(AnimTrack3(*t));

  return *this;
}

AnimTrack3& Anim3::createTrack(const char* name)
{
  tracks.push_back(AnimTrack3(name));
  return tracks.back();
}

void Anim3::destroyTrack(AnimTrack3& track)
{
  for (TrackList::iterator t = tracks.begin();  t != tracks.end();  t++)
  {
    if (&(*t) == &track)
    {
      tracks.erase(t);
      break;
    }
  }
}

void Anim3::destroyTracks()
{
  tracks.clear();
}

AnimTrack3* Anim3::findTrack(const char* name)
{
  for (TrackList::iterator t = tracks.begin();  t != tracks.end();  t++)
  {
    if (t->getName() == name)
      return &(*t);
  }

  return NULL;
}

const AnimTrack3* Anim3::findTrack(const char* name) const
{
  for (TrackList::const_iterator t = tracks.begin();  t != tracks.end();  t++)
  {
    if (t->getName() == name)
      return &(*t);
  }

  return NULL;
}

size_t Anim3::getTrackCount() const
{
  return tracks.size();
}

AnimTrack3& Anim3::getTrack(size_t index)
{
  return tracks[index];
}

const AnimTrack3& Anim3::getTrack(size_t index) const
{
  return tracks[index];
}

///////////////////////////////////////////////////////////////////////

Anim3Reader::Anim3Reader(ResourceCache& cache):
  ResourceReader(cache)
{
}

Ref<Anim3> Anim3Reader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<Anim3*>(cached);

  std::ifstream stream;
  if (!getCache().openFile(stream, path))
    return NULL;

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load 3D animation \'%s\': %s",
             path.asString().c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("animation");
  if (!root || root.attribute("version").as_uint() != ANIM3_XML_VERSION)
  {
    logError("3D animation file format mismatch in \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  Ref<Anim3> animation = new Anim3(ResourceInfo(getCache(), path));

  for (pugi::xml_node t = root.child("track");  t;  t = t.next_sibling("track"))
  {
    AnimTrack3& track = animation->createTrack(t.attribute("name").value());

    for (pugi::xml_node k = t.child("keyframe");  k;  k = k.next_sibling("keyframe"))
    {
      const Time moment = k.attribute("moment").as_double();

      const Transform3 transform(vec3Cast(k.attribute("position").value()),
                                 quatCast(k.attribute("rotation").value()));

      const vec3 direction = vec3Cast(k.attribute("direction").value());

      track.createKeyFrame(moment, transform, direction);
    }
  }

  return animation;
}

///////////////////////////////////////////////////////////////////////

bool Anim3Writer::write(const Path& path, const Anim3& animation)
{
  pugi::xml_document document;

  pugi::xml_node root = document.append_child("animation");
  root.append_attribute("version") = ANIM3_XML_VERSION;

  for (size_t i = 0;  i < animation.getTrackCount();  i++)
  {
    const AnimTrack3& track = animation.getTrack(i);

    pugi::xml_node tn = root.append_child("track");
    tn.append_attribute("name") = track.getName().c_str();

    for (size_t j = 0;  j < track.getKeyFrameCount();  j++)
    {
      const KeyFrame3& keyframe = track.getKeyFrame(i);
      const Transform3& transform = keyframe.getTransform();

      pugi::xml_node kn = tn.append_child("keyframe");
      kn.append_attribute("moment") = keyframe.getMoment();
      kn.append_attribute("position") = stringCast(transform.position).c_str();
      kn.append_attribute("rotation") = stringCast(transform.rotation).c_str();
      kn.append_attribute("direction") = stringCast(keyframe.getDirection()).c_str();
    }
  }

  std::ofstream stream(path.asString().c_str());
  if (!stream)
  {
    logError("Failed to create 3D animation file \'%s\'",
             path.asString().c_str());
    return false;
  }

  document.save(stream);
  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

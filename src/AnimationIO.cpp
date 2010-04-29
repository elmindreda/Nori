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
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>
#include <wendy/Managed.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/Resource.h>
#include <wendy/Animation.h>
#include <wendy/XML.h>

#include <internal/AnimationIO.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int ANIM3_XML_VERSION = 1;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Anim3CodecXML::Anim3CodecXML(void):
  Anim3Codec("XML 3D animation codec")
{
  addSuffix("anim3");
}

Anim3* Anim3CodecXML::read(const Path& path, const String& name)
{
  return Anim3Codec::read(path, name);
}

Anim3* Anim3CodecXML::read(Stream& stream, const String& name)
{
  animName = name;

  if (!XML::Codec::read(stream))
  {
    animation = NULL;
    return NULL;
  }

  return animation.detachObject();
}

bool Anim3CodecXML::write(const Path& path, const Anim3& animation)
{
  return Anim3Codec::write(path, animation);
}

bool Anim3CodecXML::write(Stream& stream, const Anim3& animation)
{
  try
  {
    setStream(&stream);

    beginElement("animation");
    addAttribute("version", ANIM3_XML_VERSION);

    for (size_t i = 0;  i < animation.getKeyFrameCount();  i++)
    {
      const KeyFrame3& keyframe = animation.getKeyFrame(i);

      beginElement("keyframe");
      addAttribute("moment", keyframe.getMoment());
      addAttribute("position", keyframe.getTransform().position.asString());
      addAttribute("rotation", keyframe.getTransform().rotation.asString());
      addAttribute("direction", keyframe.getDirection().asString());
      endElement();
    }

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write 3D animation %s: %s",
                    animation.getName().c_str(),
		    exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool Anim3CodecXML::onBeginElement(const String& name)
{
  if (name == "animation")
  {
    if (animation)
    {
      Log::writeError("Only one animation per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != ANIM3_XML_VERSION)
    {
      Log::writeError("3D animation XML format version mismatch");
      return false;
    }

    animation = new Anim3(animName);
    if (!animation)
      return false;

    return true;
  }

  if (animation)
  {
    if (name == "keyframe")
    {
      Time moment = readFloat("moment");

      Transform3 transform;
      transform.position = Vec3(readString("position"));
      transform.rotation = Quat(readString("rotation"));
      transform.rotation.normalize();

      Vec3 direction = Vec3(readString("direction"));

      animation->createKeyFrame(moment, transform, direction);
      return true;
    }
  }

  return true;
}

bool Anim3CodecXML::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////

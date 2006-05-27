///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@home.se>
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
#ifndef WENDY_GLCAMERA_H
#define WENDY_GLCAMERA_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Camera : public Managed<Camera>
{
public:
  Camera(const std::string& name = "");
  void begin(void) const;
  void end(void) const;
  float getFOV(void) const;
  float getAspectRatio(void) const;
  void setFOV(float newFOV);
  void setAspectRatio(float newAspectRatio);
  Transform3& getTransform(void);
  const Transform3& getTransform(void) const;
private:
  float FOV;
  float aspectRatio;
  Transform3 transform;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCAMERA_H*/
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERSTYLE_H
#define WENDY_RENDERSTYLE_H
///////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief Multipass render technique.
 *  @ingroup renderer
 */
class Technique
{
public:
  /*! Constructor.
   */
  Technique(const String& name);
  /*! Creates a new render pass in this technique.
   *  @param[in] name The name of the render pass, if it is a pass intended for
   *  a custom render stage, or the empty string to place it in the default
   *  pass group.
   *  @return The newly created render pass.
   *  @remarks The passes are rendered in creation order.
   *  @remarks Non-empty render pass names must be unique.
   *  @remarks Named render passes will be ignored by the default render stage.
   */
  GL::Pass& createPass(const String& name = "");
  void destroyPass(GL::Pass& pass);
  /*! Destroys all render passes in this technique.
   */
  void destroyPasses(void);
  /*! Applies the render pass with the specified index.
   *  @param index The index of the desired render pass.
   */
  void applyPass(unsigned int index) const;
  GL::Pass* findPass(const String& name);
  /*! Comparison operator to enable sorting.
   *  @param other The object to compare to.
   */
  bool operator < (const Technique& other) const;
  bool isCompatible(void) const;
  /*! @return @c true if this technique uses framebuffer blending, otherwise
   *  @c false.
   *  
   *  @note A blending technique is defined as a technique where the first non-named
   *  pass is blending, as it makes little sense to overwrite a blending pass with
   *  an opaque one.
   */
  bool isBlending(void) const;
  /*! @param index The index of the desired render pass.
   *  @return The render pass at the specified index.
   */
  GL::Pass& getPass(unsigned int index);
  /*! @param index The index of the desired render pass.
   *  @return The render pass at the specified index.
   */
  const GL::Pass& getPass(unsigned int index) const;
  /*! @return The number of render passes in this technique.
   */
  unsigned int getPassCount(void) const;
  const String& getName(void) const;
  float getQuality(void) const;
  void setQuality(float newQuality);
private:
  typedef std::list<GL::Pass> List;
  List passes;
  String name;
  float quality;
};

///////////////////////////////////////////////////////////////////////

class Style : public Resource<Style>
{
public:
  Style(const String& name = "");
  ~Style(void);
  Technique& createTechnique(const String& name = "");
  void destroyTechnique(Technique& technique);
  void destroyTechniques(void);
  Technique* findTechnique(const String& name);
  unsigned int getTechniqueCount(void) const;
  Technique& getTechnique(unsigned int index);
  const Technique& getTechnique(unsigned int index) const;
  Technique* getActiveTechnique(void);
  const Technique* getActiveTechnique(void) const;
private:
  bool validateTechniques(void);
  typedef std::vector<Technique*> List;
  List techniques;
  Technique* active;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSTYLE_H*/
///////////////////////////////////////////////////////////////////////

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

/*! @brief Multipass %render technique.
 *  @ingroup renderer
 */
class Technique
{
public:
  /*! Constructor.
   */
  Technique(const String& name);
  /*! Creates a new %render pass in this technique.
   *  @param[in] name The name of the %render pass, if it is a pass intended for
   *  a custom %render stage, or the empty string to place it in the default
   *  pass group.
   *  @return The newly created %render pass.
   *  @remarks The passes are rendered in creation order.
   *  @remarks Non-empty %render pass names must be unique.
   *  @remarks Named %render passes will be ignored by the default %render mechanisms.
   */
  GL::Pass& createPass(const String& name = "");
  /*! Removes the specified %render pass from this technique.
   */
  void destroyPass(GL::Pass& pass);
  /*! Destroys all %render passes in this technique.
   */
  void destroyPasses(void);
  /*! Applies the %render pass with the specified index.
   *  @param index The index of the desired %render pass.
   */
  void applyPass(unsigned int index) const;
  /*! Searches for the pass with the specified name.
   *  @param[in] name The name of the desired %render pass.
   *  @return The desired %render pass, or @c NULL if no such pass exists.
   */
  GL::Pass* findPass(const String& name);
  /*! Searches for the pass with the specified name.
   *  @param[in] name The name of the desired %render pass.
   *  @return The desired %render pass, or @c NULL if no such pass exists.
   */
  const GL::Pass* findPass(const String& name) const;
  /*! Comparison operator to enable sorting.
   *  @param other The object to compare to.
   */
  bool operator < (const Technique& other) const;
  bool isCompatible(void) const;
  /*! @return @c true if this technique uses framebuffer blending, otherwise
   *  @c false.
   *  @note A blending technique is defined as a technique where the first non-named
   *  pass is blending, as it makes little sense to overwrite a blending pass with
   *  an opaque one.
   */
  bool isBlending(void) const;
  /*! @param index The index of the desired %render pass.
   *  @return The %render pass at the specified index.
   */
  GL::Pass& getPass(unsigned int index);
  /*! @param index The index of the desired %render pass.
   *  @return The %render pass at the specified index.
   */
  const GL::Pass& getPass(unsigned int index) const;
  /*! @return The number of %render passes in this technique.
   */
  unsigned int getPassCount(void) const;
  /*! @return The name of this technique.
   */
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

/*! @brief Multi-technique %render style descriptor.
 *  @ingroup renderer
 */
class Style : public Resource<Style>, public RefObject<Style>
{
public:
  /*! Constructor.
   */
  Style(const String& name = "");
  /*! Copy constructor.
   */
  Style(const Style& source);
  /*! Destructor.
   */
  ~Style(void);
  /*! Creates a technique with the specified name in this %render style.
   */
  Technique& createTechnique(const String& name = "");
  /*! Destroys the specified technique.
   */
  void destroyTechnique(Technique& technique);
  /*! Destroys all techniques in this %render style.
   */
  void destroyTechniques(void);
  /*! Searches for the technique with the specified name.
   *  @param[in] name The name of the desired technique.
   *  @return The desired technique, or @c NULL if no such technique exists.
   */
  Technique* findTechnique(const String& name);
  /*! Assignment operator.
   */
  Style& operator = (const Style& source);
  /*! @return The number of techniques in this %render style.
   */
  unsigned int getTechniqueCount(void) const;
  /*! @return The technique at the specified index.
   */
  Technique& getTechnique(unsigned int index);
  /*! @return The technique at the specified index.
   */
  const Technique& getTechnique(unsigned int index) const;
  /*! @return The active technique for this %render style, or @c NULL if no
   *  technique is active.
   */
  Technique* getActiveTechnique(void) const;
private:
  bool validateTechniques(void) const;
  typedef std::vector<Technique*> List;
  List techniques;
  mutable Technique* active;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSTYLE_H*/
///////////////////////////////////////////////////////////////////////

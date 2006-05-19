///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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
#ifndef WEGLTEXTURE_H
#define WEGLTEXTURE_H
///////////////////////////////////////////////////////////////////////

#include <string>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Texture : public Managed<Texture>
{
public:
  /*! Destructor.
   */
  ~Texture(void);
  /*! Locks the image data of the specified mip level for modification.
   *  @param level [in] The desired mip level.
   */
  void* lock(unsigned int level = 0);
  /*! Finishes modification of the image data.
   */
  void unlock(void);
  /*! @return The OpenGL name of this texture.
   */
  GLuint getGLID(void) const;
  /*! @return The OpenGL target of this texture.
   */
  GLenum getTarget(void) const;
  /*! @return The width, in pixels, of the source for specified mip level.
   *  @param level [in] The desired mip level.
   */
  unsigned int getWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the source for specified mip level.
   *  @param level [in] The desired mip level.
   */
  unsigned int getHeight(unsigned int level = 0) const;
  /*! @return The depth, in pixels, of the source for the specified mip level.
   *  @param level [in] The desired mip level.
   */
  unsigned int getDepth(unsigned int level = 0) const;
  /*! @return The width, in pixels, of the specified mip level.
   *  @param level [in] The desired mip level.
   */
  unsigned int getPhysicalWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the specified mip level.
   *  @param level [in] The desired mip level.
   */
  unsigned int getPhysicalHeight(unsigned int level = 0) const;
  /*! @return The depth, in pixels, of the specified mip level.
   *  @param level [in] The desired mip level.
   */
  unsigned int getPhysicalDepth(unsigned int level = 0) const;
  /*! @return The number of mip levels in this texture.
   */
  unsigned int getLevelCount(void) const;
  /*! @return The creation flags for this texture.
   */
  unsigned int getFlags(void) const;
  /*! @return The image format of this texture.
   */
  const ImageFormat& getFormat(void) const;
  /*! Creates a texture from the specified image file.
   *  @param name [in] The desired name of the texture.
   *  @param path [in] The path of the image file to use.
   *  @param flags [in] The creation flags.
   */
  static Texture* createInstance(const std::string& name,
                                 const Path& path,
                                 unsigned int flags = 0);
  /*! Creates a texture from the specified image.
   *  @param name [in] The desired name of the texture.
   *  @param image [in] The image data to use.
   *  @param flags [in] The creation flags.
   */
  static Texture* createInstance(const std::string& name,
				 const Image& image,
				 unsigned int flags = 0);
  enum
  {
    /*! The texture keeps a copy in system memory, for fast modification.
     *  @remarks Not currently implemented.
     */
    SHADOWED = 1,
    /*! The texture will be created with the original image dimensions,
     *  if possible.
     *  @remarks Not currently implemented.
     */
    RECTANGULAR = 2,
    /*! The texture will be created with a mipmap chain.
     */
    MIPMAPPED = 4,
  };
private:
  Texture(const std::string& name);
  bool init(const Image& image, unsigned int initFlags);
  GLuint textureID;
  GLenum textureTarget;
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  unsigned int physicalWidth;
  unsigned int physicalHeight;
  unsigned int physicalDepth;
  unsigned int levelCount;
  unsigned int flags;
  ImageFormat format;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WEGLTEXTURE_H*/
///////////////////////////////////////////////////////////////////////

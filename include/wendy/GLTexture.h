///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLTEXTURE_H
#define WENDY_GLTEXTURE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture image object.
 *  @ingroup opengl
 *
 *  @remarks This class intentionally has no public interface for
 *  making a texture object current.
 */
class Texture : public Resource<Texture>, public RefObject<Texture>
{
  friend class Uniform;
public:
  enum
  {
    /*! The texture will be created with the original image dimensions,
     *  if possible.  Note that such textures have an address range of
     *  [0..w] and [0..h], not [0..1].
     */
    RECTANGULAR = 1,
    /*! The texture will be created with a mipmap chain.
     */
    MIPMAPPED = 2,
    /*! The texture is guaranteed not to be larger than the source image.
     */
    DONT_GROW = 4,
    /*! The default texture creation flags.
     */
    DEFAULT = MIPMAPPED,
  };
  /*! Destructor.
   */
  ~Texture(void);
  /*! Updates the area at the specified coordinates in the specified mipmap
   *  level with the specified image.
   */
  bool copyFrom(const Image& source,
                unsigned int x,
		unsigned int y,
		unsigned int level = 0);
  /*! @return The width, in pixels, of the source for specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getSourceWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the source for specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getSourceHeight(unsigned int level = 0) const;
  /*! @return The depth, in pixels, of the source for the specified mipmap
   *  level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getSourceDepth(unsigned int level = 0) const;
  /*! @return The width, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalHeight(unsigned int level = 0) const;
  /*! @return The depth, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalDepth(unsigned int level = 0) const;
  /*! @return The number of mipmap levels in this texture.
   */
  unsigned int getLevelCount(void) const;
  /*! @return The creation flags for this texture.
   */
  unsigned int getFlags(void) const;
  /*! @return The image format of this texture.
   */
  const ImageFormat& getFormat(void) const;
  /*! @param[in] level The desired mipmap level.
   *  @return The image data of the specified mipmap level.
   */
  Image* getImage(unsigned int level = 0) const;
  /*! Creates a texture from the specified image.
   *  @param[in] image The image data to use.
   *  @param[in] flags The creation flags.
   *  @param[in] name The desired name of the texture, or the empty string to
   *  automatically generate a name.
   */
  static Texture* createInstance(const Image& image,
                                 unsigned int flags = DEFAULT,
				 const String& name = "");
private:
  Texture(const String& name);
  Texture(const Texture& source);
  bool init(const Image& image, unsigned int flags);
  Texture& operator = (const Texture& source);
  unsigned int textureTarget;
  unsigned int textureID;
  int minFilter;
  int magFilter;
  int addressMode;
  unsigned int sourceWidth;
  unsigned int sourceHeight;
  unsigned int sourceDepth;
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
#endif /*WENDY_GLTEXTURE_H*/
///////////////////////////////////////////////////////////////////////

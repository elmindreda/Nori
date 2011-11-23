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

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Rectangle.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>
#include <wendy/Pixel.h>
#include <wendy/Image.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class Image;
class Texture;
class Context;

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture sampler filtering mode enumeration.
 *  @ingroup opengl
 */
enum FilterMode
{
  /*! Select the nearest texel of the nearest mipmap level.
   */
  FILTER_NEAREST,
  /*! Select the four nearest texels of the nearest mipmap level and
   *  interpolate.
   */
  FILTER_BILINEAR,
  /*! Select the four nearest texels of the two nearest mipmap levels and
   *  interpolate.
   */
  FILTER_TRILINEAR,
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture sampler address wrapping mode enumeration.
 *  @ingroup opengl
 */
enum AddressMode
{
  /*! Addresses are wrapped to within the texture.
   */
  ADDRESS_WRAP,
  /*! Addresses are clamped to the edges of the texture.
   */
  ADDRESS_CLAMP,
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture type enumeration.
 *  @ingroup opengl
 */
enum TextureType
{
  /*! %Texture has one-dimensional images with power-of-two dimensions.
   */
  TEXTURE_1D,
  /*! %Texture has two-dimensional images with power-of-two dimensions.
   */
  TEXTURE_2D,
  /*! %Texture has three-dimensional images with power-of-two dimensions.
   */
  TEXTURE_3D,
  /*! %Texture has a single two-dimensional image without power-of-two
   *  dimension restrictions.
   */
  TEXTURE_RECT,
  /*! %Texture has a cube of two-dimensional, square images with power-of-two
   *  dimensions.
   */
  TEXTURE_CUBE,
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture image object.
 *  @ingroup opengl
 */
class TextureImage : public Image
{
  friend class Texture;
public:
  /*! Updates an area within this texture image, at the specified coordinates
   *  and with a size matching the specified image, with the contents of that
   *  image.
   *  @param[in] source The image to copy pixel data from.
   *  @param[in] x The x-coordinate of the area within this image to update.
   *  @param[in] y The y-coordinate of the area within this image to update.
   *  @param[in] z The z-coordinate of the area within this image to update.
   */
  bool copyFrom(const wendy::Image& source,
                unsigned int x = 0,
                unsigned int y = 0,
                unsigned int z = 0);
  /*! Copies the contents of this texture image to the specified image.
   */
  bool copyTo(wendy::Image& result) const;
  unsigned int getWidth() const;
  unsigned int getHeight() const;
  unsigned int getDepth() const;
  /*! @return The cube face this image represents, or @c NO_CUBE_FACE if this
   *  image is not part of a cube map.
   */
  CubeFace getFace() const;
  const PixelFormat& getFormat() const;
  /*! @return The texture containing this texture image.
   */
  Texture& getTexture() const;
private:
  TextureImage(Texture& texture,
               unsigned int level,
               unsigned int width,
               unsigned int height,
               unsigned int depth,
               CubeFace face = NO_CUBE_FACE);
  void attach(int attachment, unsigned int z);
  void detach(int attachment);
  Texture& texture;
  unsigned int level;
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  CubeFace face;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
typedef Ref<TextureImage> TextureImageRef;

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture object.
 *  @ingroup opengl
 */
class Texture : public Resource
{
  friend class Context;
  friend class TextureImage;
public:
  enum
  {
    /*! The texture may have non-POT dimensions and will be created with the
     *  original image dimensions or as close to them as possible.  Note that
     *  textures created with this flag have an address range of [0..w] and
     *  [0..h], not [0..1], whether or not they have POT dimensions.
     */
    RECTANGULAR = 1,
    /*! The texture will be created with a mipmap chain.
     */
    MIPMAPPED = 2,
    /*! The default texture creation flags.
     */
    DEFAULT = MIPMAPPED,
  };
  /*! Destructor.
   */
  ~Texture();
  /*! Generates mipmaps based on the top-level image.
   */
  void generateMipmaps();
  /*! @return @c true if this texture is one-dimensional, otherwise @c false.
   */
  bool is1D() const;
  /*! @return @c true if this texture is two-dimensional, otherwise @c false.
   */
  bool is2D() const;
  /*! @return @c true if this texture is three-dimensional, otherwise @c false.
   */
  bool is3D() const;
  /*! @return @c true if this texture's dimensions are power of two, otherwise
   *  @c false.
   */
  bool isPOT() const;
  /*! @return @c true if this texture is a cubemap, otherwise @c false.
   */
  bool isCube() const;
  /*! @return @c true if this texture is mipmapped, otherwise @c false.
   */
  bool hasMipmaps() const;
  /*! @return The type of this texture.
   */
  TextureType getType() const;
  /*! @param[in] level The desired mipmap level.
   *  @return The width, in pixels, of the specified mipmap level of this texture.
   */
  unsigned int getWidth(unsigned int level = 0) const;
  /*! @param[in] level The desired mipmap level.
   *  @return The height, in pixels, of the specified mipmap level of this texture.
   */
  unsigned int getHeight(unsigned int level = 0) const;
  /*! @param[in] level The desired mipmap level.
   *  @return The depth, in pixels, of the specified mipmap level of this texture.
   */
  unsigned int getDepth(unsigned int level = 0) const;
  /*! @return The number of mipmap levels of this texture.
   */
  unsigned int getLevelCount() const;
  /*! @param[in] level The desired mipmap level.
   *  @param[in] face The desired cube map face if this texture is a cubemap,
   *  or @c NO_CUBE_FACE otherwise.
   *  @return The specified texture image, or @c NULL if the desired image is
   *  not available.
   */
  TextureImage* getImage(unsigned int level = 0, CubeFace face = NO_CUBE_FACE);
  /*! @return The sampler filter mode of this texture.
   */
  FilterMode getFilterMode() const;
  /*! Sets the sampler filter mode of this texture.
   *  @param[in] newMode The desired filter mode.
   */
  void setFilterMode(FilterMode newMode);
  /*! @return The sampler address wrapping mode of this texture.
   */
  AddressMode getAddressMode() const;
  /*! Sets the sampler address wrapping mode of this texture.
   *  @param[in] newMode The desired address wrapping mode.
   */
  void setAddressMode(AddressMode newMode);
  /*! @return The image format of this texture.
   */
  const PixelFormat& getFormat() const;
  /*! @return The context used to create this texture.
   */
  Context& getContext() const;
  /*! Creates a texture from the specified image.
   *  @param[in] info The resource info for the texture.
   *  $param[in] context The OpenGL context within which to create the
   *  texture.
   *  @param[in] image The image data to use.
   *  @param[in] flags The creation flags.
   */
  static Ref<Texture> create(const ResourceInfo& info,
                             Context& context,
                             const wendy::Image& source,
                             unsigned int flags);
  /*! Creates a texture from the specified image cube.
   *  @param[in] info The resource info for the texture.
   *  $param[in] context The OpenGL context within which to create the
   *  texture.
   *  @param[in] image The image cube data to use.
   *  @param[in] flags The creation flags.
   */
  static Ref<Texture> create(const ResourceInfo& info,
                             Context& context,
                             const ImageCube& source,
                             unsigned int flags);
  /*! Creates a texture using the specified texture specification file.
   *  @param[in] context The OpenGL context within which to create the texture.
   *  @param[in] path The path of the texture specification file to use.
   */
  static Ref<Texture> read(Context& context, const Path& path);
private:
  Texture(const ResourceInfo& info, Context& context);
  Texture(const Texture& source);
  bool init(const wendy::Image& source, unsigned int flags);
  bool init(const ImageCube& source, unsigned int flags);
  bool validateProxy() const;
  unsigned int retrieveImages(unsigned int target, CubeFace face);
  void applyDefaults();
  Texture& operator = (const Texture& source);
  typedef std::vector<TextureImageRef> ImageList;
  Context& context;
  TextureType type;
  unsigned int textureID;
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  unsigned int levels;
  FilterMode filterMode;
  AddressMode addressMode;
  PixelFormat format;
  ImageList images;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
typedef Ref<Texture> TextureRef;

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
typedef std::vector<TextureRef> TextureList;

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
class TextureReader : public ResourceReader
{
public:
  TextureReader(Context& context);
  Ref<Texture> read(const Path& path);
private:
  Context& context;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLTEXTURE_H*/
///////////////////////////////////////////////////////////////////////

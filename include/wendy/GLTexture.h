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

class Image;
class Texture;

///////////////////////////////////////////////////////////////////////

/*! @brief Texture sampler filtering mode.
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

/*! @brief Texture sampler address wrapping mode.
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

/*! @brief %Texture image object.
 *  @ingroup opengl
 */
class TextureImage : public Image
{
  friend class Texture;
public:
  unsigned int getWidth(void) const;
  unsigned int getHeight(void) const;
  const ImageFormat& getFormat(void) const;
  Texture& getTexture(void) const;
private:
  TextureImage(Texture& texture, unsigned int width, unsigned int height);
  Texture& texture;
  unsigned int width;
  unsigned int height;
};

///////////////////////////////////////////////////////////////////////

typedef Ref<TextureImage> TextureImageRef;

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture object.
 *  @ingroup opengl
 */
class Texture : public Resource<Texture>, public RefObject
{
  friend class Sampler;
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
  ~Texture(void);
  /*! Updates the area at the specified coordinates in the specified mipmap
   *  level with the contents of the specified image.
   */
  bool copyFrom(const moira::Image& source,
                unsigned int x,
		unsigned int y,
		unsigned int level = 0);
  bool copyFromColorBuffer(unsigned int x,
		           unsigned int y,
		           unsigned int level = 0);
  /*! @return @c true if this texture's dimensions are power of two, otherwise
   *  @c false.
   */
  bool isPOT(void) const;
  /*! @return @c true if this texture is mipmapped, otherwise @c false.
   */
  bool isMipmapped(void) const;
  /*! @return The width, in pixels, of the source for specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getSourceWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the source for specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getSourceHeight(unsigned int level = 0) const;
  /*! @return The width, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalHeight(unsigned int level = 0) const;
  /*! @return The number of mipmap levels in this texture.
   */
  unsigned int getImageCount(void) const;
  TextureImage& getImage(unsigned int level = 0);
  /*! @return The sampler filter mode of this texture.
   */
  FilterMode getFilterMode(void) const;
  /*! Sets the sampler filter mode of this texture.
   *  @param[in] newMode The desired filter mode.
   */
  void setFilterMode(FilterMode newMode);
  /*! @return The sampler address wrapping mode of this texture.
   */
  AddressMode getAddressMode(void) const;
  /*! Sets the sampler address wrapping mode of this texture.
   *  @param[in] newMode The desired address wrapping mode.
   */
  void setAddressMode(AddressMode newMode);
  /*! @return The image format of this texture.
   */
  const ImageFormat& getFormat(void) const;
  Context& getContext(void) const;
  /*! Creates a texture from the specified image.
   *  @param[in] image The image data to use.
   *  @param[in] flags The creation flags.
   *  @param[in] name The desired name of the texture, or the empty string to
   *  automatically generate a name.
   */
  static Texture* createInstance(Context& context,
                                 const moira::Image& image,
                                 unsigned int flags,
				 const String& name = "");
private:
  Texture(Context& context, const String& name);
  Texture(const Texture& source);
  bool init(const moira::Image& image, unsigned int flags);
  Texture& operator = (const Texture& source);
  typedef std::vector<TextureImageRef> ImageList;
  Context& context;
  unsigned int textureTarget;
  unsigned int textureID;
  unsigned int sourceWidth;
  unsigned int sourceHeight;
  unsigned int physicalWidth;
  unsigned int physicalHeight;
  unsigned int flags;
  FilterMode filterMode;
  AddressMode addressMode;
  ImageFormat format;
  ImageList images;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLTEXTURE_H*/
///////////////////////////////////////////////////////////////////////

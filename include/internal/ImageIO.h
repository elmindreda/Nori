///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#ifndef WENDY_IMAGEIO_H
#define WENDY_IMAGEIO_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

typedef ResourceCodec<Image> ImageCodec;

///////////////////////////////////////////////////////////////////////

typedef ResourceCodec<ImageCube> ImageCubeCodec;

///////////////////////////////////////////////////////////////////////

class ImageCodecPNG : public ImageCodec
{
public:
  ImageCodecPNG(void);
  Image* read(const Path& path, const String& name = "");
  Image* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Image& image);
  bool write(Stream& stream, const Image& image);
};

///////////////////////////////////////////////////////////////////////

class ImageCubeCodecXML : public ImageCubeCodec, public XML::Codec
{
public:
  ImageCubeCodecXML(void);
  ImageCube* read(const Path& path, const String& name = "");
  ImageCube* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const ImageCube& cube);
  bool write(Stream& stream, const ImageCube& cube);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<ImageCube> cube;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_IMAGEIO_H*/
///////////////////////////////////////////////////////////////////////

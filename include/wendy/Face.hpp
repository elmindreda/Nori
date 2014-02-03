///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2013 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_FACE_HPP
#define WENDY_FACE_HPP
///////////////////////////////////////////////////////////////////////

typedef struct stbtt_fontinfo stbtt_fontinfo;

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief TrueType typeface.
 */
class Face : public Resource, public RefObject
{
public:
  ~Face();
  float scale(uint height) const;
  float ascender(float scale) const;
  float descender(float scale) const;
  float leading(float scale) const;
  float width(float scale) const;
  float height(float scale) const;
  int indexForCodePoint(uint32 codepoint);
  float advance(int index, float scale) const;
  vec2 bearing(int index, float scale) const;
  float advance(int first, int second, float scale) const;
  float width(int index, float scale) const;
  float height(int index, float scale) const;
  Ref<Image> glyph(int index, float scale) const;
  static Ref<Face> create(const ResourceInfo& info, const char* data, size_t size);
  static Ref<Face> read(ResourceCache& cache, const String& name);
private:
  Face(const ResourceInfo& info);
  Face(const Face&) = delete;
  bool init(const char* data, size_t size);
  Face& operator = (const Face&) = delete;
  std::vector<char> m_data;
  stbtt_fontinfo* m_info;
};

///////////////////////////////////////////////////////////////////////

class FaceReader : public ResourceReader<Face>
{
public:
  FaceReader(ResourceCache& cache);
  using ResourceReader<Face>::read;
  Ref<Face> read(const String& name, const Path& path);
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_FACE_HPP*/
///////////////////////////////////////////////////////////////////////

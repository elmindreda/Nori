///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLQUERY_H
#define WENDY_GLQUERY_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class Context;

///////////////////////////////////////////////////////////////////////

/*! @brief Occlusion query.
 *  @ingroup opengl
 */
class OcclusionQuery
{
public:
  /*! Destructor.
   *  @note You should not destroy active queries.
   */
  ~OcclusionQuery();
  /*! Makes this occlusion query active.  As long as it is active, it will
   *  record the number of rendered samples that pass the depth test.
   *  @note You may only have one active query at any given time.
   */
  void begin();
  /*! Deactivates this query object, making its result available.
   */
  void end();
  /*! @return @c true if this query is active, otherwise @c false.
   */
  bool isActive() const;
  /*! @return @c true if the result of this query is available, otherwise @c
   *  false.
   */
  bool hasResultAvailable() const;
  /*! @return The latest results of this query, or zero if it is active or has
   *  never been active.
   */
  unsigned int getResult() const;
  /*! Creates an occlusion query.
   *  @param[in] context The context within which to create the query.
   *  @return The newly created query object, or @c NULL if an error occurred.
   */
  static OcclusionQuery* create(Context& context);
private:
  OcclusionQuery(Context& context);
  bool init();
  Context& context;
  unsigned int queryID;
  bool active;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLQUERY_H*/
///////////////////////////////////////////////////////////////////////

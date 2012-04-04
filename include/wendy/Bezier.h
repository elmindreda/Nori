///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_BEZIER_H
#define WENDY_BEZIER_H
///////////////////////////////////////////////////////////////////////

#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Control point template for n-dimensional cubic Bézier curves.
 */
template <typename T>
class BezierPoint
{
public:
  /*! Constructor.
   */
  BezierPoint()
  {
  }
  /*! Constructor.
   */
  BezierPoint(const T& initPosition, const T& initDirection):
    position(initPosition),
    direction(initDirection)
  {
  }
  /*! Sets the position and direction of this control point.
   */
  void set(const T& newPosition, const T& newDirection)
  {
    position = newPosition;
    direction = newDirection;
  }
  /*! The position of this control point.
   */
  T position;
  /*! The direction of this control point.
   */
  T direction;
};

///////////////////////////////////////////////////////////////////////

typedef BezierPoint<vec2> BezierPoint2;
typedef BezierPoint<vec3> BezierPoint3;

///////////////////////////////////////////////////////////////////////

/*! @brief N-dimensional cubic Bézier curve template.
 */
template <typename T>
class BezierCurve
{
public:
  typedef std::vector<T> PointList;
  /*! @return The length of this Bézier curve, calculated through tessellation.
   *  @param[in] tolerance The error tolerance to use.
   */
  float length(float tolerance = 0.5f) const
  {
    const float arcLength = glm::length(P[1] - P[0]) +
                            glm::length(P[2] - P[1]) +
                            glm::length(P[3] - P[2]);

    const float directLength = glm::length(P[3] - P[0]);

    if ((arcLength - directLength) / directLength > tolerance)
    {
      BezierCurve<T> one, two;
      split(one, two);
      return one.length(tolerance) + two.length(tolerance);
    }

    return (arcLength + directLength) / 2.f;
  }
  /*! @return The center of this Bézier curve.
   */
  T center() const
  {
    return (P[0] + 3.f * P[1] + 3.f * P[2] + P[3]) / 8.f;
  }
  /*! Splits this Bézier curve into two curves with second order continuity.
   *  @param[out] one The first curve.
   *  @param[out] one The second curve.
   */
  void split(BezierCurve<T>& one, BezierCurve<T>& two) const
  {
    one.P[0] = P[0];
    two.P[3] = P[3];

    one.P[1] = (P[0] + P[1]) / 2.f;
    two.P[2] = (P[2] + P[3]) / 2.f;

    T peak = (P[1] + P[2]) / 2.f;

    one.P[2] = (one.P[1] + peak) / 2.f;
    two.P[1] = (two.P[2] + peak) / 2.f;

    one.P[3] = two.P[0] = (one.P[2] + two.P[1]) / 2.f;
  }
  /*! Tessellates this Bézier curve.
   *  @param[out] result The resulting list of points.
   *  @param[in] tolerance The error tolerance to use.
   */
  void tessellate(PointList& result, float tolerance = 0.5f) const
  {
    if (result.empty())
      result.push_back(P[0]);

    const float arcLength = glm::length(P[1] - P[0]) +
                            glm::length(P[2] - P[1]) +
                            glm::length(P[3] - P[2]);

    const float directLength = glm::length(P[3] - P[0]);

    if ((arcLength - directLength) / directLength > tolerance)
    {
      BezierCurve<T> one, two;
      split(one, two);
      one.tessellate(result, tolerance);
      two.tessellate(result, tolerance);
    }
    else
      result.push_back(P[3]);
  }
  /*! Evaluates this Bézier curve at the specified @i t.
   */
  T operator () (float t)
  {
    return P[0] * pow(1.f - t, 3.f) +
           P[1] * 3.f * t * pow(1.f - t, 2.f) +
           P[2] * 3.f * pow(t, 2.f) * (1.f - t) +
           P[3] * pow(t, 3.f);
  }
  /*! The control points of this Bézier curve.
   */
  T P[4];
};

///////////////////////////////////////////////////////////////////////

typedef BezierCurve<vec2> BezierCurve2;
typedef BezierCurve<vec3> BezierCurve3;

///////////////////////////////////////////////////////////////////////

/*! @brief N-dimensional cubic Bézier spline with first order continuity.
 */
template <typename T>
class BezierSpline
{
public:
  typedef typename BezierCurve<T>::PointList PointList;
  /*! Tessellates this Bézier spline.
   *  @param[out] result The resulting list of points.
   *  @param[in] tolerance The error tolerance to use.
   */
  void tessellate(PointList& result, float tolerance = 0.5f) const
  {
    if (points.empty())
      return;

    if (points.size() == 1)
    {
      result.push_back(points.front().position);
      return;
    }

    for (size_t i = 1;  i < points.size();  i++)
    {
      BezierCurve<T> segment;
      segment.P[0] = points[i - 1].position;
      segment.P[1] = points[i - 1].position + points[i - 1].direction;
      segment.P[2] = points[i].position - points[i].direction;
      segment.P[3] = points[i].position;

      segment.tessellate(result, tolerance);
    }
  }
  /*! Evaluates this Bézier spline at the specified @i t.
   */
  T operator () (float t) const
  {
    if (points.empty())
    {
      logError("Cannot evaluate Bézier spline with no points");
      return T();
    }

    if (t <= 0.f || points.size() == 1)
      return points.front().position;

    if (t >= 1.f)
      return points.back().position;

    size_t source = (size_t) (t * (points.size() - 1));
    size_t target = source + 1;

    BezierCurve<T> segment;
    segment.P[0] = points[source].position;
    segment.P[1] = points[source].position + points[source].direction;
    segment.P[2] = points[target].position - points[target].direction;
    segment.P[3] = points[target].position;

    return segment((t * (points.size() - 1)) - (float) source);
  }
  /*! The control points of this Bézier spline.
   */
  PointList points;
};

///////////////////////////////////////////////////////////////////////

typedef BezierSpline<vec2> BezierSpline2;
typedef BezierSpline<vec3> BezierSpline3;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_BEZIER_H*/
///////////////////////////////////////////////////////////////////////

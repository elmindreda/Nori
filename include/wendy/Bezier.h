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

#include <cmath>
#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief N-dimensional cubic Bézier curve template.
 */
template <typename T>
class BezierCurve
{
public:
  typedef std::vector<T> PointList;
  inline BezierCurve(void);
  inline void evaluate(float t, T& result) const;
  inline float length(float tolerance = 0.5f) const;
  inline T center(void) const;
  inline void split(BezierCurve<T>& one, BezierCurve<T>& two) const;
  inline void tessellate(PointList& result, float tolerance = 0.5f) const;
  inline T operator () (float t);
  inline void setDefaults(void);
  T P[4];
};

///////////////////////////////////////////////////////////////////////

typedef BezierCurve<Vec2> BezierCurve2;
typedef BezierCurve<Vec3> BezierCurve3;

///////////////////////////////////////////////////////////////////////

/*! @brief Control point template for n-dimensional cubic Bézier curve.
 */
template <typename T>
class BezierPoint
{
public:
  inline BezierPoint(void);
  inline BezierPoint(const T& position, const T& direction);
  inline void set(const T& position, const T& direction);
  T position;
  T direction;
};

///////////////////////////////////////////////////////////////////////

typedef BezierPoint<Vec2> BezierPoint2;
typedef BezierPoint<Vec3> BezierPoint3;

///////////////////////////////////////////////////////////////////////

/*! @brief N-dimensional cubic Bézier segment path with first order continuity.
 */
template <typename T>
class BezierSpline
{
public:
  typedef std::vector<BezierPoint<T> > PointList;
  void evaluate(float t, T& result) const;
  inline void tessellate(typename BezierCurve<T>::PointList& result,
                         float tolerance = 0.5f) const;
  T operator () (float t) const;
  PointList points;
};

///////////////////////////////////////////////////////////////////////

typedef BezierSpline<Vec2> BezierSpline2;
typedef BezierSpline<Vec3> BezierSpline3;

///////////////////////////////////////////////////////////////////////

template <typename T>
inline BezierCurve<T>::BezierCurve(void)
{
  setDefaults();
}

template <typename T>
inline void BezierCurve<T>::evaluate(float t, T& result) const
{
  result = P[0] * powf(1.f - t, 3.f) +
           P[1] * 3.f * t * powf(1.f - t, 2.f) +
           P[2] * 3.f * powf(t, 2.f) * (1.f - t) +
           P[3] * powf(t, 3.f);
}

template <typename T>
inline float BezierCurve<T>::length(float tolerance) const
{
  const float arcLength = (P[1] - P[0]).length() +
                          (P[2] - P[1]).length() +
                          (P[3] - P[2]).length();

  const float directLength = (P[3] - P[0]).length();

  if ((arcLength - directLength) / directLength > tolerance)
  {
    BezierCurve<T> one, two;
    split(one, two);
    return one.length(tolerance) + two.length(tolerance);
  }

  return (arcLength + directLength) / 2.f;
}

template <typename T>
inline T BezierCurve<T>::center(void) const
{
  return (P[0] + 3.f * P[1] + 3.f * P[2] + P[3]) / 8.f;
}

template <typename T>
inline void BezierCurve<T>::split(BezierCurve<T>& one,
                                  BezierCurve<T>& two) const
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

template <typename T>
inline void BezierCurve<T>::tessellate(PointList& result, float tolerance) const
{
  if (result.empty())
    result.push_back(P[0]);

  const float arcLength = (P[1] - P[0]).length() +
                          (P[2] - P[1]).length() +
                          (P[3] - P[2]).length();

  const float directLength = (P[3] - P[0]).length();

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

template <typename T>
inline T BezierCurve<T>::operator () (float t)
{
  T result;
  evaluate(t, result);
  return result;
}

template <typename T>
inline void BezierCurve<T>::setDefaults(void)
{
}

template <>
inline void BezierCurve<Vec2>::setDefaults(void)
{
  P[0].set(0.f, 0.f);
  P[1].set(1.f, 0.f);
  P[2].set(0.f, 1.f);
  P[3].set(1.f, 1.f);
}

template <>
inline void BezierCurve<Vec3>::setDefaults(void)
{
  P[0].set(0.f, 0.f, 0.f);
  P[1].set(1.f, 0.f, 0.f);
  P[2].set(0.f, 1.f, 1.f);
  P[3].set(1.f, 1.f, 1.f);
}

///////////////////////////////////////////////////////////////////////

template <typename T>
inline BezierPoint<T>::BezierPoint(void)
{
}

template <typename T>
inline BezierPoint<T>::BezierPoint(const T& initPosition,
                                   const T& initDirection):
  position(initPosition),
  direction(initDirection)
{
}

template <typename T>
inline void BezierPoint<T>::set(const T& newPosition, const T& newDirection)
{
  position = newPosition;
  direction = newDirection;
}

///////////////////////////////////////////////////////////////////////

template <typename T>
inline void BezierSpline<T>::evaluate(float t, T& result) const
{
  if (points.size() == 0)
  {
    result.setDefaults();
    return;
  }

  if (t <= 0.f || points.size() == 1)
  {
    result = points.front().position;
    return;
  }

  if (t >= 1.f)
  {
    result = points.back().position;
    return;
  }

  size_t source = (size_t) (t * (points.size() - 1));
  size_t target = source + 1;

  BezierCurve<T> segment;
  segment.P[0] = points[source].position;
  segment.P[1] = points[source].position + points[source].direction;
  segment.P[2] = points[target].position - points[target].direction;
  segment.P[3] = points[target].position;

  segment.evaluate((t * (points.size() - 1)) - (float) source, result);
}

template <typename T>
inline T BezierSpline<T>::operator () (float t) const
{
  T result;
  evaluate(t, result);
  return result;
}

template <typename T>
inline void BezierSpline<T>::tessellate(typename BezierCurve<T>::PointList& result,
                                        float tolerance) const
{
  if (points.empty())
    return;

  if (points.size() == 1)
  {
    result.push_back(points[0].position);
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

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_BEZIER_H*/
///////////////////////////////////////////////////////////////////////

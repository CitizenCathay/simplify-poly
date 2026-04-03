/* Start Header *****************************************************************/
/*!
\file       collapse.cpp
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Cerrero Nicole Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Implements geometric segment intersection tests used by the polygon
    simplification pipeline.

    This source file provides helper routines for orientation tests,
    epsilon-based comparisons, collinearity checks, and interval overlap
    detection. Its main responsibility is to determine whether two line
    segments intersect under the project’s geometric rules.

    The intersection logic treats proper crossings, non-shared endpoint
    touches, and interior collinear overlap as intersections, while exact
    shared endpoints are ignored. These rules are useful for topology
    validation during candidate collapse and ring intersection checks.
*/
/* End Header *******************************************************************/

#include "intersect.h"
#include <cmath>
#include <algorithm>

namespace
{
    constexpr double EPS = 1e-12;

    /*!
    \brief
        Compares two floating-point values using an epsilon tolerance.

    \details
        This helper returns true when the absolute difference between
        the two values is less than or equal to the global tolerance.
        It is used to make geometric comparisons more robust in the
        presence of floating-point rounding error.

    \param[in] a
        The first value to compare.
    \param[in] b
        The second value to compare.

    \return
        Returns true if the values are approximately equal and false
        otherwise.
    */
    static bool almost_equal(double a, double b)
    {
        return std::abs(a - b) <= EPS;
    }


    /*!
    \brief
        Tests whether two points are approximately the same.

    \details
        Two points are considered equal if both their x coordinates and
        y coordinates are equal within epsilon tolerance.

    \param[in] ax
        The x coordinate of the first point.
    \param[in] ay
        The y coordinate of the first point.
    \param[in] bx
        The x coordinate of the second point.
    \param[in] by
        The y coordinate of the second point.

    \return
        Returns true if the two points are approximately equal and false
        otherwise.
    */
    static bool same_point(double ax, double ay, double bx, double by)
    {
        return almost_equal(ax, bx) && almost_equal(ay, by);
    }

    /*!
    \brief
        Computes the orientation determinant of three points.

    \details
        The returned value is positive if the sequence of points forms a
        counterclockwise turn, negative if it forms a clockwise turn, and
        zero if the points are collinear.

    \param[in] ax
        The x coordinate of the first point.
    \param[in] ay
        The y coordinate of the first point.
    \param[in] bx
        The x coordinate of the second point.
    \param[in] by
        The y coordinate of the second point.
    \param[in] cx
        The x coordinate of the third point.
    \param[in] cy
        The y coordinate of the third point.

    \return
        Returns the orientation determinant for the three points.
    */
    static double orient(double ax, double ay,
                         double bx, double by,
                         double cx, double cy)
    {
        return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    }

    /*!
    \brief
        Classifies a floating-point value by sign using epsilon tolerance.

    \details
        Values greater than EPS are treated as positive, values less than
        -EPS are treated as negative, and values within the tolerance band
        are treated as zero.

    \param[in] x
        The value to classify.

    \return
        Returns 1 for positive values, -1 for negative values, and 0 for
        approximately zero values.
    */
    static int sign_eps(double x)
    {
        if (x > EPS) return  1;
        if (x < -EPS) return -1;
        return 0;
    }

    /*!
    \brief
        Tests whether two 1D intervals overlap in their interiors.

    \details
        The interval endpoints may be given in either order. This routine
        normalizes both intervals and then checks whether their overlap has
        positive length, excluding overlap that occurs only at an endpoint.

    \param[in] a0
        The first endpoint of the first interval.
    \param[in] a1
        The second endpoint of the first interval.
    \param[in] b0
        The first endpoint of the second interval.
    \param[in] b1
        The second endpoint of the second interval.

    \return
        Returns true if the intervals overlap with positive interior length
        and false otherwise.
    */
    static bool interval_overlap_strict(double a0, double a1, double b0, double b1)
    {
        if (a0 > a1) std::swap(a0, a1);
        if (b0 > b1) std::swap(b0, b1);
        return std::max(a0, b0) < std::min(a1, b1) - EPS;
    }

    /*!
    \brief
        Tests whether two collinear segments overlap in their interiors.

    \details
        Since the segments are already known to be collinear, overlap is
        tested by projecting them onto the dominant axis of the first
        segment. This avoids failure cases that can occur if both axes are
        tested directly for nearly vertical or nearly horizontal segments.

    \param[in] ax
        The x coordinate of the first endpoint of segment AB.
    \param[in] ay
        The y coordinate of the first endpoint of segment AB.
    \param[in] bx
        The x coordinate of the second endpoint of segment AB.
    \param[in] by
        The y coordinate of the second endpoint of segment AB.
    \param[in] cx
        The x coordinate of the first endpoint of segment CD.
    \param[in] cy
        The y coordinate of the first endpoint of segment CD.
    \param[in] dx
        The x coordinate of the second endpoint of segment CD.
    \param[in] dy
        The y coordinate of the second endpoint of segment CD.

    \return
        Returns true if the two collinear segments overlap in their interiors
        and false otherwise.
    */
    static bool collinear_overlap_interior(double ax, double ay, double bx, double by,
                                        double cx, double cy, double dx, double dy)
    {
        // Project onto the dominant axis.
        // Using both x and y fails for vertical/horizontal collinear overlaps.
        if (std::abs(bx - ax) >= std::abs(by - ay))
            return interval_overlap_strict(ax, bx, cx, dx);
        else
            return interval_overlap_strict(ay, by, cy, dy);
    }
    /*!
    \brief
        Tests whether a point lies on a segment using epsilon tolerance.

    \details
        A point is considered to lie on the segment if it is collinear with
        the segment endpoints and its coordinates fall within the segment’s
        bounding box, expanded by epsilon tolerance.

    \param[in] ax
        The x coordinate of the first endpoint of the segment.
    \param[in] ay
        The y coordinate of the first endpoint of the segment.
    \param[in] bx
        The x coordinate of the second endpoint of the segment.
    \param[in] by
        The y coordinate of the second endpoint of the segment.
    \param[in] px
        The x coordinate of the query point.
    \param[in] py
        The y coordinate of the query point.

    \return
        Returns true if the point lies on the segment and false otherwise.
    */
    static bool on_segment_eps(double ax, double ay,
                           double bx, double by,
                           double px, double py)
    {
        return sign_eps(orient(ax, ay, bx, by, px, py)) == 0 &&
            px >= std::min(ax, bx) - EPS && px <= std::max(ax, bx) + EPS &&
            py >= std::min(ay, by) - EPS && py <= std::max(ay, by) + EPS;
    }
}

/*!
\brief
    Tests whether two line segments intersect.

\details
    This function applies orientation-based segment intersection logic with
    epsilon tolerance. It treats the following as intersections:
    - proper crossings,
    - endpoint-on-segment contact when the touching point is not an exact
      shared endpoint,
    - collinear overlap with positive interior length.

    Exact shared endpoints are intentionally ignored because adjacent edges
    in a polygonal chain may legally meet at a common vertex and should not
    be classified as self-intersections under this project’s rules.

\param[in] ax
    The x coordinate of the first endpoint of segment AB.
\param[in] ay
    The y coordinate of the first endpoint of segment AB.
\param[in] bx
    The x coordinate of the second endpoint of segment AB.
\param[in] by
    The y coordinate of the second endpoint of segment AB.
\param[in] cx
    The x coordinate of the first endpoint of segment CD.
\param[in] cy
    The y coordinate of the first endpoint of segment CD.
\param[in] dx
    The x coordinate of the second endpoint of segment CD.
\param[in] dy
    The y coordinate of the second endpoint of segment CD.

\return
    Returns true if the two segments intersect under the project’s rules
    and false otherwise.
*/
bool segments_intersect(double ax, double ay, double bx, double by,
                        double cx, double cy, double dx, double dy)
{
    // Still ignore exact shared endpoints.
    if (same_point(ax, ay, cx, cy) || same_point(ax, ay, dx, dy) ||
        same_point(bx, by, cx, cy) || same_point(bx, by, dx, dy))
        return false;

    double d1 = orient(ax, ay, bx, by, cx, cy);
    double d2 = orient(ax, ay, bx, by, dx, dy);
    double d3 = orient(cx, cy, dx, dy, ax, ay);
    double d4 = orient(cx, cy, dx, dy, bx, by);

    int s1 = sign_eps(d1);
    int s2 = sign_eps(d2);
    int s3 = sign_eps(d3);
    int s4 = sign_eps(d4);

    // Proper crossing
    if (s1 * s2 < 0 && s3 * s4 < 0)
        return true;

    // Collinear case
    if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0)
        return collinear_overlap_interior(ax, ay, bx, by, cx, cy, dx, dy);

    // Non-shared touching / endpoint-on-segment should count as intersection
    if (s1 == 0 && on_segment_eps(ax, ay, bx, by, cx, cy)) return true;
    if (s2 == 0 && on_segment_eps(ax, ay, bx, by, dx, dy)) return true;
    if (s3 == 0 && on_segment_eps(cx, cy, dx, dy, ax, ay)) return true;
    if (s4 == 0 && on_segment_eps(cx, cy, dx, dy, bx, by)) return true;

    return false;
}
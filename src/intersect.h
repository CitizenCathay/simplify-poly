/* Start Header *****************************************************************/
/*!
\file       intersect.h
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Nicole Cerrero Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Declares the line segment intersection test used by the polygon
    simplification pipeline.

    This header provides the interface for testing whether two line
    segments intersect under the project’s geometric rules. The function
    is used during topology validation to detect invalid crossings that
    may be introduced during segment collapse or other polygon updates.

    The intersection test treats proper crossings, non-shared endpoint
    touches, and interior collinear overlap as intersections, while exact
    shared endpoints are ignored.
*/
/* End Header *******************************************************************/
#pragma once

/*!
\brief
    Tests whether two line segments intersect.

\details
    This function determines whether segment AB intersects segment CD
    according to the project’s topology rules. Exact shared endpoints are
    ignored, but proper crossings, endpoint-on-segment contact, and
    interior collinear overlap are treated as intersections.

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
                        double cx, double cy, double dx, double dy);
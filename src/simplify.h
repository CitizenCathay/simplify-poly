/* Start Header *****************************************************************/
/*!
\file       simplify.h
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Cerrero Nicole Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Declares the main polygon simplification routine.

    This header provides the interface for simplifying a polygon stored in
    a VertexPool to a target maximum number of vertices across all rings.
    The simplification is performed in place and preserves the polygon’s
    area and topology subject to the validity rules enforced by the
    implementation.

    The declared function returns the total accumulated areal displacement
    caused by all accepted collapse operations.
*/
/* End Header *******************************************************************/

#pragma once
#include "polygon.h"

/*!
\brief
    Simplifies a polygon to a target number of vertices.

\details
    This function reduces the total number of vertices across all rings in
    the given vertex pool to at most the specified target. The polygon is
    modified in place as valid collapse operations are applied.

    The function returns the total areal displacement accumulated over the
    entire simplification process.

\param[in,out] pool
    The vertex pool containing the polygon data to simplify.
\param[in] target_vertices
    The target maximum number of vertices across all rings.

\return
    Returns the total areal displacement produced by the simplification.
*/
double simplify(VertexPool& pool, int target_vertices);
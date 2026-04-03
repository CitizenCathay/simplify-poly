/* Start Header *****************************************************************/
/*!
\file       collapse.cpp
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Nicole Cerrero Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Declares the Candidate structure and collapse candidate construction
    routine used by the area-preserving segment collapse algorithm.

    This header defines the data stored for a single collapse candidate,
    including the four consecutive vertices involved in the collapse,
    the position of the replacement vertex E, the computed areal
    displacement, and whether the candidate is valid.

    It also declares the `make_candidate` function, which constructs a
    candidate for replacing the polyline A -> B -> C -> D with
    A -> E -> D while preserving area whenever a valid paper-defined
    placement exists.
*/
/* End Header *******************************************************************/

#pragma once
#include "polygon.h"

/*!
\struct Candidate
\brief
    Stores the data for a single segment-collapse candidate.

\details
    A Candidate represents a possible simplification of four consecutive
    vertices A, B, C, and D into the shorter sequence A, E, and D, where
    E is a replacement point chosen to preserve area.

    The structure stores the indices of the original four vertices, the
    coordinates of the replacement point E, the unsigned areal displacement
    caused by the collapse, and a validity flag indicating whether a legal
    area-preserving candidate could be constructed.
*/
struct Candidate
{
    int a, b, c, d;        // four consecutive vertices
    double displacement;   // areal displacement of this collapse
    double ex, ey;         // position of replacement point E
    bool valid;            // false if any index has been deactivated
};

/*!
\brief
    Constructs a collapse candidate for a four-vertex sequence.

\details
    This function attempts to build a valid area-preserving collapse
    candidate for the consecutive vertices A, B, C, and D referenced
    by the given indices. If successful, it computes the replacement
    point E and the associated areal displacement for replacing
    A -> B -> C -> D with A -> E -> D.

    If no valid area-preserving placement exists, the returned
    Candidate is marked as invalid.

\param[in] pool
    The vertex pool containing the vertices referenced by the indices.
\param[in] a
    The index of vertex A.
\param[in] b
    The index of vertex B.
\param[in] c
    The index of vertex C.
\param[in] d
    The index of vertex D.

\return
    Returns a Candidate describing the collapse. If construction fails,
    the returned candidate has `valid` set to false.
*/
Candidate make_candidate(const VertexPool& pool, int a, int b, int c, int d);
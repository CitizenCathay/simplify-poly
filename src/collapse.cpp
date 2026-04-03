/* Start Header *****************************************************************/
/*!
\file       collapse.cpp
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Cerrero Nicole Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Implements helper routines and candidate construction logic for the
    area-preserving segment collapse algorithm.

    This source file computes the replacement vertex E for a four-vertex
    sequence A -> B -> C -> D using the area-preserving placement rules
    derived from the APSC method. It also computes the associated areal
    displacement for the collapse candidate and handles both regular and
    singular geometric configurations.

    Internal helper functions are used for line-line intersection,
    segment-segment intersection, and signed or unsigned triangle area
    calculations. The main public function defined in this file is
    `make_candidate`, which creates a collapse candidate from four
    consecutive vertices in the vertex pool.
*/
/* End Header *******************************************************************/

#include "collapse.h"
#include <cmath>
#include <limits>

namespace
{
    constexpr double EPS = 1e-12;

    /*!
    \brief
        Computes the intersection between an implicit line and the line
        passing through two points.

    \details
        The implicit line is given by the equation
        a * x + b * y + c = 0, while the second line is defined by the
        points P and Q. If the two lines are not parallel, the function
        computes their intersection point and stores it in the output
        references.

        This routine works with the infinite line through P and Q rather
        than restricting the solution to the segment itself.

    \param[in]  a
        The x coefficient of the implicit line equation.
    \param[in]  b
        The y coefficient of the implicit line equation.
    \param[in]  c
        The constant term of the implicit line equation.
    \param[in]  px
        The x coordinate of point P.
    \param[in]  py
        The y coordinate of point P.
    \param[in]  qx
        The x coordinate of point Q.
    \param[in]  qy
        The y coordinate of point Q.
    \param[out] ix
        The x coordinate of the computed intersection point.
    \param[out] iy
        The y coordinate of the computed intersection point.

    \return
        Returns true if the lines intersect and false if they are parallel.
    */
    static bool line_line_intersect(double a, double b, double c,
                                    double px, double py,
                                    double qx, double qy,
                                    double& ix, double& iy)
    {
        double dp = a * px + b * py + c;
        double dq = a * qx + b * qy + c;
        double denom = dp - dq;
        if (std::abs(denom) < EPS) return false;

        double t = dp / denom;
        ix = px + t * (qx - px);
        iy = py + t * (qy - py);
        return true;
    }

    /*!
    \brief
        Computes the unsigned area of a triangle.

    \details
        This helper returns the absolute area of the triangle formed by
        the three input points. It is used when summing the magnitude of
        swept regions or lobe areas without regard to orientation.

    \param[in]  ax
        The x coordinate of the first vertex.
    \param[in]  ay
        The y coordinate of the first vertex.
    \param[in]  bx
        The x coordinate of the second vertex.
    \param[in]  by
        The y coordinate of the second vertex.
    \param[in]  cx
        The x coordinate of the third vertex.
    \param[in]  cy
        The y coordinate of the third vertex.

    \return
        Returns the absolute area of the triangle.
    */
    static double tri_abs(double ax, double ay,
                          double bx, double by,
                          double cx, double cy)
    {
        return 0.5 * std::abs((bx - ax) * (cy - ay) - (by - ay) * (cx - ax));
    }


    /*!
    \brief
        Computes the signed area of a triangle.

    \details
        The result is positive or negative depending on the orientation
        of the three input points. This is useful for determining local
        turning direction and detecting whether swept subregions have
        consistent orientation.

    \param[in]  ax
        The x coordinate of the first vertex.
    \param[in]  ay
        The y coordinate of the first vertex.
    \param[in]  bx
        The x coordinate of the second vertex.
    \param[in]  by
        The y coordinate of the second vertex.
    \param[in]  cx
        The x coordinate of the third vertex.
    \param[in]  cy
        The y coordinate of the third vertex.

    \return
        Returns the signed area of the triangle.
    */
    static double tri_signed(double ax, double ay,
                              double bx, double by,
                              double cx, double cy)
    {
        return 0.5 * ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax));
    }

    /*!
    \brief
        Tests whether two line segments properly intersect.

    \details
        This function checks whether segment P1 -> P2 and segment
        Q1 -> Q2 cross at a proper interior intersection point.
        Shared endpoints and parallel or collinear overlaps are not
        considered valid intersections by this routine.

        If a proper crossing exists, the intersection point is written
        to the output coordinates.

    \param[in]  p1x
        The x coordinate of the first endpoint of segment P.
    \param[in]  p1y
        The y coordinate of the first endpoint of segment P.
    \param[in]  p2x
        The x coordinate of the second endpoint of segment P.
    \param[in]  p2y
        The y coordinate of the second endpoint of segment P.
    \param[in]  q1x
        The x coordinate of the first endpoint of segment Q.
    \param[in]  q1y
        The y coordinate of the first endpoint of segment Q.
    \param[in]  q2x
        The x coordinate of the second endpoint of segment Q.
    \param[in]  q2y
        The y coordinate of the second endpoint of segment Q.
    \param[out] ix
        The x coordinate of the intersection point.
    \param[out] iy
        The y coordinate of the intersection point.

    \return
        Returns true if the two segments properly intersect and false
        otherwise.
    */
    static bool seg_seg_intersect(double p1x, double p1y,
                                  double p2x, double p2y,
                                  double q1x, double q1y,
                                  double q2x, double q2y,
                                  double& ix, double& iy)
    {
        double dp = tri_signed(p1x, p1y, p2x, p2y, q1x, q1y) * 2.0;
        double dq = tri_signed(p1x, p1y, p2x, p2y, q2x, q2y) * 2.0;
        double den = dp - dq;
        if (std::abs(den) < EPS) return false;
        // Only a proper crossing (opposite signs) counts.
        if (dp * dq >= 0.0) return false;
        double t = dp / den;
        ix = q1x + t * (q2x - q1x);
        iy = q1y + t * (q2y - q1y);
        return true;
    }
}

/*!
\brief
    Constructs a collapse candidate for a four-vertex sequence.

\details
    This function builds a Candidate corresponding to the sequence
    A -> B -> C -> D in the given vertex pool. It computes the
    paper-defined area-preserving line for the new replacement vertex E,
    chooses whether E should lie on AB or CD according to the geometric
    configuration, and then calculates the unsigned areal displacement
    produced by replacing A -> B -> C -> D with A -> E -> D.

    If the configuration is degenerate or no valid placement can be found
    using the rules defined by the method, the returned candidate is
    marked as invalid.

    The displacement calculation supports both simple swept regions and
    self-intersecting figure-eight cases by detecting crossings and
    summing the appropriate unsigned lobe areas.

\param[in] pool
    The vertex pool containing all vertices referenced by the candidate.
\param[in] a
    The index of vertex A in the vertex pool.
\param[in] b
    The index of vertex B in the vertex pool.
\param[in] c
    The index of vertex C in the vertex pool.
\param[in] d
    The index of vertex D in the vertex pool.

\return
    Returns a fully initialized Candidate. If no valid placement exists,
    the returned candidate has its `valid` field set to false.
*/
Candidate make_candidate(const VertexPool& pool, int a, int b, int c, int d)
{
    Candidate cand;
    cand.a = a;
    cand.b = b;
    cand.c = c;
    cand.d = d;
    cand.valid = true;
    cand.ex = 0.0;
    cand.ey = 0.0;
    cand.displacement = std::numeric_limits<double>::infinity();

    const Vertex& A = pool.verts[a];
    const Vertex& B = pool.verts[b];
    const Vertex& C = pool.verts[c];
    const Vertex& D = pool.verts[d];

    // Paper eq. (1b): area-preserving line for E
    double a_coeff = D.y - A.y;
    double b_coeff = A.x - D.x;
    double c_coeff = -B.y * A.x
                   + (A.y - C.y) * B.x
                   + (B.y - D.y) * C.x
                   + C.y * D.x;

    double len_ad_sq = a_coeff * a_coeff + b_coeff * b_coeff;
    if (len_ad_sq < EPS)
    {
        cand.valid = false;
        return cand;
    }

    // Side values relative to directed line AD
    double val_AD = a_coeff * A.x + b_coeff * A.y;
    double side_B = a_coeff * B.x + b_coeff * B.y - val_AD;
    double side_C = a_coeff * C.x + b_coeff * C.y - val_AD;
    double side_E = -c_coeff - val_AD; // any point on E-line satisfies a*x+b*y=-c

    // Figure 6a: E-line coincident with AD.
    // Any point on AD is optimal; use D.
    if (std::abs(side_E) < EPS)
    {
        cand.ex = D.x;
        cand.ey = D.y;
        cand.displacement = 0.0;
        return cand;
    }

    bool bc_same_side_of_AD = (side_B * side_C > 0.0);

    bool use_AB = true;

    if (bc_same_side_of_AD)
    {
        // Figure 4b / 4c:
        // if B farther from AD -> use CD, if C farther -> use AB,
        // tie case (Figure 6c, BC || AD): either is optimal; choose AB.
        double dist_B = std::abs(side_B);
        double dist_C = std::abs(side_C);

        if (dist_B > dist_C + EPS)
            use_AB = false;   // use CD
        else if (dist_C > dist_B + EPS)
            use_AB = true;    // use AB
        else
            use_AB = true;    // tie: choose AB
    }
    else
    {
        // Figure 4a:
        // use AB if B is on same side of AD as E-line, else CD.
        use_AB = (side_B * side_E > 0.0);
    }

    double ex = 0.0, ey = 0.0;
    bool ok = false;

    if (use_AB)
    {
        ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                 A.x, A.y, B.x, B.y, ex, ey);
    }
    else
    {
        ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                 C.x, C.y, D.x, D.y, ex, ey);
    }

    if (!ok)
    {
        // Singular BC || AD case from Figure 6c:
        // either AB or CD intersection is optimal.
        // Try the other one, but do NOT invent a midpoint/perpendicular fallback.
        if (use_AB)
        {
            ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                     C.x, C.y, D.x, D.y, ex, ey);
        }
        else
        {
            ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                     A.x, A.y, B.x, B.y, ex, ey);
        }
    }

    if (!ok)
    {
        // No valid paper-defined placement found.
        cand.valid = false;
        return cand;
    }

    cand.ex = ex;
    cand.ey = ey;

    // Areal displacement = unsigned area swept by the boundary change
    // A->B->C->D becomes A->E->D.  The swept region is the closed loop
    // A->B->C->D->E->A.  When new edges AE or ED cross old edge BC the
    // loop self-intersects (figure-8), forming two lobes whose unsigned
    // areas must be summed separately.
    //
    // Detection: if all three fan-triangle signed areas have the same sign,
    // there is no self-intersection and the abs-sum is correct.  Otherwise
    // find the crossing point X on BC and sum the two lobe triangles.
    {
        double s_abe = tri_signed(A.x, A.y, B.x, B.y, ex, ey);
        double s_bce = tri_signed(B.x, B.y, C.x, C.y, ex, ey);
        double s_cde = tri_signed(C.x, C.y, D.x, D.y, ex, ey);
        double signed_sum = s_abe + s_bce + s_cde;
        double abs_sum    = std::abs(s_abe) + std::abs(s_bce) + std::abs(s_cde);

        if (std::abs(abs_sum - std::abs(signed_sum)) < EPS)
        {
            // No self-intersection: all fan triangles same sign.
            cand.displacement = abs_sum;
        }
        else
        {
            // Self-intersecting swept region.
            // Try ED x BC, then AE x BC to find the crossing point X.
            double ix = 0.0, iy = 0.0;
            bool found = seg_seg_intersect(ex,  ey,  D.x, D.y,
                                           B.x, B.y, C.x, C.y,
                                           ix, iy);
            if (!found)
                found = seg_seg_intersect(A.x, A.y, ex,  ey,
                                          B.x, B.y, C.x, C.y,
                                          ix, iy);

            if (found)
            {
                // Two lobe triangles: (B, X, E) and (X, C, D).
                cand.displacement =
                      tri_abs(B.x, B.y, ix, iy, ex, ey)
                    + tri_abs(ix, iy, C.x, C.y, D.x, D.y);
            }
            else
            {
                // Fallback (should not occur for well-formed input).
                cand.displacement = abs_sum;
            }
        }
    }

    return cand;
}
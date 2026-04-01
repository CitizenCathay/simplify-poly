#include "collapse.h"
#include <cmath>
#include <limits>

namespace
{
    constexpr double EPS = 1e-12;

    // intersect line (a*x + b*y + c = 0) with segment line through P->Q
    // returns true if not parallel, sets (ix, iy)
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

    static double tri_abs(double ax, double ay,
                          double bx, double by,
                          double cx, double cy)
    {
        return 0.5 * std::abs((bx - ax) * (cy - ay) - (by - ay) * (cx - ax));
    }
}

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
        // if B farther from AD -> AB, else if C farther -> CD,
        // tie case (Figure 6c, BC || AD): either is optimal; choose AB.
        double dist_B = std::abs(side_B);
        double dist_C = std::abs(side_C);

        if (dist_B > dist_C + EPS)
            use_AB = false;   // use CD
        else if (dist_C > dist_B + EPS)
            use_AB = true;    // use AB
        else
            use_AB = true;    // tie: B is not farther than C -> use AB
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

    // Areal displacement = total area enclosed by ABCD and AED.
    // For this local collapse, compute it as the three triangles formed with E.
    cand.displacement =
          tri_abs(A.x, A.y, B.x, B.y, ex, ey)
        + tri_abs(B.x, B.y, C.x, C.y, ex, ey)
        + tri_abs(C.x, C.y, D.x, D.y, ex, ey);

    return cand;
}
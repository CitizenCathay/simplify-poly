#include "collapse.h"
#include <cmath>
#include <limits>

static double cross2(double ax, double ay, double bx, double by)
{
    return ax * by - bx * ay;
}

// intersect line (a*x + b*y + c = 0) with line through P->Q
// returns true if not parallel, sets (ix, iy)
static bool line_line_intersect(double a, double b, double c,
                                double px, double py,
                                double qx, double qy,
                                double& ix, double& iy)
{
    double dp = a * px + b * py + c;
    double dq = a * qx + b * qy + c;
    double denom = dp - dq;
    if (std::abs(denom) < 1e-15) return false;
    double t = dp / denom;
    ix = px + t * (qx - px);
    iy = py + t * (qy - py);
    return true;
}

Candidate make_candidate(const VertexPool& pool, int a, int b, int c, int d)
{
    Candidate cand;
    cand.a = a; cand.b = b; cand.c = c; cand.d = d;
    cand.valid = true;

    const Vertex& A = pool.verts[a];
    const Vertex& B = pool.verts[b];
    const Vertex& C = pool.verts[c];
    const Vertex& D = pool.verts[d];

    // From the paper (eq. 1b), the area-preserving line E_line is:
    //   a_coeff * xE + b_coeff * yE + c_coeff = 0
    // This line is parallel to AD; any E on it preserves area.
    double a_coeff = D.y - A.y;
    double b_coeff = A.x - D.x;
    double c_coeff = -B.y * A.x + (A.y - C.y) * B.x + (B.y - D.y) * C.x + C.y * D.x;

    double len_ad_sq = a_coeff * a_coeff + b_coeff * b_coeff;
    if (len_ad_sq < 1e-24)
    {
        cand.valid = false;
        cand.displacement = std::numeric_limits<double>::infinity();
        return cand;
    }

    // Signed distance from a point to line AD is proportional to:
    //   a_coeff * x + b_coeff * y - val_AD
    // where val_AD = a_coeff * A.x + b_coeff * A.y (since A is on AD).
    double val_AD = a_coeff * A.x + b_coeff * A.y;
    double side_B = a_coeff * B.x + b_coeff * B.y - val_AD;
    double side_C = a_coeff * C.x + b_coeff * C.y - val_AD;

    // Side of E_line relative to AD:
    // A point on E_line satisfies a*x+b*y = -c, so its "side" value is -c - val_AD
    double side_E = -c_coeff - val_AD;

    bool B_C_same_side = (side_B * side_C > 0);

    bool use_AB;
    if (B_C_same_side)
    {
        // Paper cases 4b/4c: B,C on same side of AD.
        // Use AB if B is farther from AD than C, else CD.
        use_AB = (std::abs(side_B) >= std::abs(side_C));
    }
    else
    {
        // Paper case 4a: B,C on opposite sides of AD.
        // Use AB if B is on the same side of AD as E_line.
        use_AB = (side_B * side_E > 0);
    }

    double ex, ey;
    bool ok;
    if (use_AB)
        ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                 A.x, A.y, B.x, B.y, ex, ey);
    else
        ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                 C.x, C.y, D.x, D.y, ex, ey);

    if (!ok)
    {
        // Chosen segment is parallel to E_line; try the other
        if (use_AB)
            ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                     C.x, C.y, D.x, D.y, ex, ey);
        else
            ok = line_line_intersect(a_coeff, b_coeff, c_coeff,
                                     A.x, A.y, B.x, B.y, ex, ey);
    }

    if (!ok)
    {
        // Both parallel (BC parallel to AD) -- fallback: midpoint perpendicular
        double len_ad = std::sqrt(len_ad_sq);
        double s_abcd = 0.5 * (cross2(A.x, A.y, B.x, B.y)
                             + cross2(B.x, B.y, C.x, C.y)
                             + cross2(C.x, C.y, D.x, D.y)
                             + cross2(D.x, D.y, A.x, A.y));
        double h = -(2.0 * s_abcd) / len_ad;
        double mid_x = 0.5 * (A.x + D.x);
        double mid_y = 0.5 * (A.y + D.y);
        double perp_x = -(D.y - A.y) / len_ad;
        double perp_y =  (D.x - A.x) / len_ad;
        ex = mid_x + h * perp_x;
        ey = mid_y + h * perp_y;
    }

    cand.ex = ex;
    cand.ey = ey;

    // Areal displacement = sum of unsigned triangle areas (fan from E to old path)
    auto tri_abs = [](double ax, double ay, double bx, double by, double cx, double cy) {
        return 0.5 * std::abs((bx - ax) * (cy - ay) - (by - ay) * (cx - ax));
    };

    cand.displacement = tri_abs(A.x, A.y, B.x, B.y, ex, ey)
                      + tri_abs(B.x, B.y, C.x, C.y, ex, ey)
                      + tri_abs(C.x, C.y, D.x, D.y, ex, ey);
    return cand;
}

#include "intersect.h"
#include <cmath>
#include <algorithm>

namespace
{
    constexpr double EPS = 1e-12;

    static bool almost_equal(double a, double b)
    {
        return std::abs(a - b) <= EPS;
    }

    static bool same_point(double ax, double ay, double bx, double by)
    {
        return almost_equal(ax, bx) && almost_equal(ay, by);
    }

    // > 0 CCW, < 0 CW, = 0 collinear
    static double orient(double ax, double ay,
                         double bx, double by,
                         double cx, double cy)
    {
        return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    }

    static int sign_eps(double x)
    {
        if (x > EPS) return  1;
        if (x < -EPS) return -1;
        return 0;
    }

    static bool interval_overlap_strict(double a0, double a1, double b0, double b1)
    {
        if (a0 > a1) std::swap(a0, a1);
        if (b0 > b1) std::swap(b0, b1);
        return std::max(a0, b0) < std::min(a1, b1) - EPS;
    }

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
}

bool segments_intersect(double ax, double ay, double bx, double by,
                        double cx, double cy, double dx, double dy)
{
    // Ignore shared endpoints exactly as the header says.
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

    // Collinear overlap with interior overlap counts as intersection.
    if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0)
        return collinear_overlap_interior(ax, ay, bx, by, cx, cy, dx, dy);

    // Touching at a non-shared endpoint or lying on the other segment is not
    // treated as a strict crossing for your topology check.
    return false;
}
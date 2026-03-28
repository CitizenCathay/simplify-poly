#include "intersect.h"
#include <cmath>
#include <algorithm>

static double sign(double x)
{
    if (x > 0) return  1.0;
    if (x < 0) return -1.0;
    return 0.0;
}

// orientation of triplet (ax,ay)->(bx,by)->(cx,cy)
// > 0 CCW, < 0 CW, = 0 collinear
static double orient(double ax, double ay,
                     double bx, double by,
                     double cx, double cy)
{
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

static bool on_segment(double ax, double ay,
                        double bx, double by,
                        double px, double py)
{
    return std::min(ax,bx) <= px && px <= std::max(ax,bx)
        && std::min(ay,by) <= py && py <= std::max(ay,by);
}

bool segments_intersect(double ax, double ay, double bx, double by,
                        double cx, double cy, double dx, double dy)
{
    double d1 = orient(cx, cy, dx, dy, ax, ay);
    double d2 = orient(cx, cy, dx, dy, bx, by);
    double d3 = orient(ax, ay, bx, by, cx, cy);
    double d4 = orient(ax, ay, bx, by, dx, dy);

    if (sign(d1) != sign(d2) && sign(d3) != sign(d4))
        return true;

    // collinear cases — treat as non-intersecting unless they overlap
    // (shared endpoints between adjacent segments are not intersections)
    if (d1 == 0 && on_segment(cx, cy, dx, dy, ax, ay)) return true;
    if (d2 == 0 && on_segment(cx, cy, dx, dy, bx, by)) return true;
    if (d3 == 0 && on_segment(ax, ay, bx, by, cx, cy)) return true;
    if (d4 == 0 && on_segment(ax, ay, bx, by, dx, dy)) return true;

    return false;
}
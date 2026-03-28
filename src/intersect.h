#pragma once

// returns true if segment AB strictly intersects segment CD
// (ignores shared endpoints)
bool segments_intersect(double ax, double ay, double bx, double by,
                        double cx, double cy, double dx, double dy);
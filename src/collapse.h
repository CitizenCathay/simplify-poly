#pragma once
#include "polygon.h"

struct Candidate
{
    int a, b, c, d;        // four consecutive vertices
    double displacement;   // areal displacement of this collapse
    double ex, ey;         // position of replacement point E
    bool valid;            // false if any index has been deactivated
};

// compute candidate for collapsing A->B->C->D to A->E->D
// returns valid=false if no area-preserving E exists
Candidate make_candidate(const VertexPool& pool, int a, int b, int c, int d);
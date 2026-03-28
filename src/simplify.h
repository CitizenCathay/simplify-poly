#pragma once
#include "polygon.h"

// reduces pool to at most target_vertices total across all rings
// modifies pool in place, returns total areal displacement
double simplify(VertexPool& pool, int target_vertices);
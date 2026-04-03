/* Start Header *****************************************************************/
/*!
\file       simplify.cpp
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Nicole Cerrero Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Implements the main area-preserving polygon simplification algorithm.

    This source file contains the core APSC-based simplification routine.
    It constructs and maintains a priority queue of collapse candidates,
    selects the minimum-displacement candidate, verifies topology
    constraints using a spatial index, and applies valid collapses until
    the target vertex count is reached.

    The algorithm incrementally updates the polygon structure, spatial
    index, and candidate queue while preserving area and preventing
    self-intersections.
*/
/* End Header *******************************************************************/

#include "simplify.h"
#include "collapse.h"
#include "spatial_index.h"
#include <queue>
#include <vector>

/*!
\struct CandCmp
\brief
    Comparator for ordering collapse candidates in a min-heap.

\details
    Candidates are ordered primarily by increasing areal displacement.
    Ties are broken deterministically using vertex indices to ensure
    stable and reproducible behavior.
*/
struct CandCmp
{
    bool operator()(const Candidate& lhs, const Candidate& rhs) const
    {
        constexpr double EPS = 1e-12;

        if (lhs.displacement > rhs.displacement + EPS) return true;
        if (rhs.displacement > lhs.displacement + EPS) return false;

        if (lhs.a != rhs.a) return lhs.a > rhs.a;
        if (lhs.b != rhs.b) return lhs.b > rhs.b;
        if (lhs.c != rhs.c) return lhs.c > rhs.c;
        return lhs.d > rhs.d;
    }
};

/*!
\brief
    Computes the total number of active vertices across all rings.

\param[in] pool
    The vertex pool containing all rings.

\return
    Returns the total number of active vertices.
*/
static int total_vertices(const VertexPool& pool)
{
    int n = 0;
    for (int r = 0; r < pool.num_rings; ++r)
        n += pool.ring_size(r);
    return n;
}

/*!
\brief
    Constructs a collapse candidate centered at vertex b.

\details
    This function identifies the four consecutive vertices
    A -> B -> C -> D using the ring connectivity and constructs
    the corresponding collapse candidate.

\param[in] pool
    The vertex pool containing the polygon data.
\param[in] b
    The index of the central vertex B.

\return
    Returns the constructed Candidate.
*/
static Candidate candidate_at(const VertexPool& pool, int b)
{
    int a = pool.prev_of(b);
    int c = pool.next_of(b);
    int d = pool.next_of(c);
    return make_candidate(pool, a, b, c, d);
}

/*!
\brief
    Simplifies the polygon to a target number of vertices.

\details
    This function implements the APSC simplification loop. It builds an
    initial priority queue of collapse candidates, repeatedly selects the
    candidate with minimum areal displacement, verifies its validity and
    topology constraints, and applies the collapse if valid.

    The algorithm maintains a spatial index to efficiently detect
    intersections introduced by new edges. After each collapse, affected
    candidates are updated and reinserted into the priority queue.

    The process continues until the total number of vertices is reduced
    to the specified target or no valid candidates remain.

\param[in,out] pool
    The vertex pool representing the polygon. It is modified in place.
\param[in] target_vertices
    The desired maximum number of vertices after simplification.

\return
    Returns the total accumulated areal displacement.
*/
double simplify(VertexPool& pool, int target_vertices)
{
    double total_displacement = 0.0;

    if (total_vertices(pool) <= target_vertices)
        return total_displacement;

    // build spatial index
    UniformGrid grid;
    grid.build(pool);

    // build initial priority queue — one candidate per pair (b,c)
    std::priority_queue<Candidate, std::vector<Candidate>, CandCmp> pq;

    for (int r = 0; r < pool.num_rings; ++r)
    {
        // need at least 4 vertices to collapse (ring stays valid with 3)
        if (pool.ring_size(r) < 4) continue;

        int start = pool.ring_heads[r];
        int b = start;
        do {
            pq.push(candidate_at(pool, b));
            b = pool.next_of(b);
        } while (b != start);
    }

    while (total_vertices(pool) > target_vertices && !pq.empty())
    {
        Candidate cand = pq.top();
        pq.pop();

        auto& verts = pool.verts;

        // skip stale candidates
        if (!verts[cand.a].active || !verts[cand.b].active ||
            !verts[cand.c].active || !verts[cand.d].active)
            continue;

        // verify connectivity
        if (pool.next_of(cand.a) != cand.b) continue;
        if (pool.next_of(cand.b) != cand.c) continue;
        if (pool.next_of(cand.c) != cand.d) continue;

        int r = verts[cand.b].ring_id;
        if (pool.ring_size(r) <= 3) continue;

        // recompute candidate
        cand = make_candidate(pool, cand.a, cand.b, cand.c, cand.d);
        if (!cand.valid) continue;

        // topology check
        int prev_a = pool.prev_of(cand.a);
        int next_d = pool.next_of(cand.d);

        std::vector<std::pair<int,int>> skip_ae = {
            {cand.a, cand.b}, {cand.b, cand.c}, {cand.c, cand.d},
            {prev_a, cand.a},
            {cand.d, next_d}
        };

        std::vector<std::pair<int,int>> skip_ed = {
            {cand.a, cand.b}, {cand.b, cand.c}, {cand.c, cand.d},
            {cand.d, next_d},
            {prev_a, cand.a}
        };

        bool ok = true;
        ok = ok && !grid.has_intersection(pool,
                        verts[cand.a].x, verts[cand.a].y,
                        cand.ex, cand.ey,
                        skip_ae);

        ok = ok && !grid.has_intersection(pool,
                        cand.ex, cand.ey,
                        verts[cand.d].x, verts[cand.d].y,
                        skip_ed);

        if (!ok) continue;

        // perform collapse

        grid.remove_segment(pool, cand.a, cand.b);
        grid.remove_segment(pool, cand.b, cand.c);
        grid.remove_segment(pool, cand.c, cand.d);

        // reuse b as E
        verts[cand.b].x = cand.ex;
        verts[cand.b].y = cand.ey;
        verts[cand.c].active = false;

        if (pool.ring_heads[r] == cand.c)
            pool.ring_heads[r] = cand.d;

        // relink
        verts[cand.a].next = cand.b;
        verts[cand.b].prev = cand.a;
        verts[cand.b].next = cand.d;
        verts[cand.d].prev = cand.b;

        pool.ring_sizes[r]--;
        total_displacement += cand.displacement;

        grid.insert_segment(pool, cand.a, cand.b);
        grid.insert_segment(pool, cand.b, cand.d);

        // update affected candidates
        if (pool.ring_size(r) >= 4)
        {
            pq.push(candidate_at(pool, cand.a));
            pq.push(candidate_at(pool, cand.b));
            pq.push(candidate_at(pool, pool.prev_of(cand.a)));
            pq.push(candidate_at(pool, cand.d));
        }
    }

    return total_displacement;
}
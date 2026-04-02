#include "simplify.h"
#include "collapse.h"
#include "spatial_index.h"
#include <queue>
#include <vector>

// min-heap comparator
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

static int total_vertices(const VertexPool& pool)
{
    int n = 0;
    for (int r = 0; r < pool.num_rings; ++r)
        n += pool.ring_size(r);
    return n;
}

// returns a,b,c,d for the candidate starting at vertex b
// (b's prev = a, b's next = c, c's next = d)
static Candidate candidate_at(const VertexPool& pool, int b)
{
    int a = pool.prev_of(b);
    int c = pool.next_of(b);
    int d = pool.next_of(c);
    return make_candidate(pool, a, b, c, d);
}

double simplify(VertexPool& pool, int target_vertices)
{
    double total_displacement = 0.0;

    if (total_vertices(pool) <= target_vertices)
        return total_displacement;

    // build spatial index
    UniformGrid grid;
    grid.build(pool);

    // build initial priority queue — one candidate per pair (b,c)
    // we index candidates by b (the first of the two removed vertices)
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

        // stale check: any of a,b,c,d deactivated since this was pushed?
        auto& verts = pool.verts;
        if (!verts[cand.a].active || !verts[cand.b].active ||
            !verts[cand.c].active || !verts[cand.d].active)
            continue;

        // verify a->b->c->d are still consecutive
        if (pool.next_of(cand.a) != cand.b) continue;
        if (pool.next_of(cand.b) != cand.c) continue;
        if (pool.next_of(cand.c) != cand.d) continue;

        // ring must keep at least 3 vertices
        int r = verts[cand.b].ring_id;
        if (pool.ring_size(r) <= 3) continue;

        // recompute candidate with current vertex positions
        // (prior collapses may have moved vertices a, b, c, or d)
        cand = make_candidate(pool, cand.a, cand.b, cand.c, cand.d);
        if (!cand.valid) continue;

        // topology check: do new segments AE and ED intersect anything?
        // segments being removed: AB, BC, CD
        // segments adjacent (sharing endpoint): prev_A->A for A->E, D->next_D for E->D
        int prev_a = pool.prev_of(cand.a);
        int next_d = pool.next_of(cand.d);

        // Each skip list includes the three removed segments plus BOTH adjacent
        // segments (the one sharing A and the one sharing D). This prevents
        // false intersection hits from segments that share the new edge's
        // far endpoint after the grid has been updated by prior collapses.
        std::vector<std::pair<int,int>> skip_ae = {
            {cand.a, cand.b}, {cand.b, cand.c}, {cand.c, cand.d},  // removed
            {prev_a, cand.a},  // shares endpoint A with new edge A->E
            {cand.d, next_d}   // shares endpoint E (far end of A->E) with E->D
        };
        std::vector<std::pair<int,int>> skip_ed = {
            {cand.a, cand.b}, {cand.b, cand.c}, {cand.c, cand.d},  // removed
            {cand.d, next_d},  // shares endpoint D with new edge E->D
            {prev_a, cand.a}   // shares endpoint E (near end of E->D) with A->E
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

        // ── perform collapse ──────────────────────────────────────────

        // remove old segments from grid
        grid.remove_segment(pool, cand.a, cand.b);
        grid.remove_segment(pool, cand.b, cand.c);
        grid.remove_segment(pool, cand.c, cand.d);

        // reuse vertex b as E, deactivate c
        verts[cand.b].x = cand.ex;
        verts[cand.b].y = cand.ey;
        verts[cand.c].active = false;

        // update ring head if it pointed to the deactivated vertex
        if (pool.ring_heads[r] == cand.c)
            pool.ring_heads[r] = cand.d;

        // relink: a <-> b <-> d
        verts[cand.a].next = cand.b;
        verts[cand.b].prev = cand.a;
        verts[cand.b].next = cand.d;
        verts[cand.d].prev = cand.b;

        pool.ring_sizes[r]--;
        total_displacement += cand.displacement;

        // insert new segments into grid
        grid.insert_segment(pool, cand.a, cand.b);
        grid.insert_segment(pool, cand.b, cand.d);

        // push updated candidates for affected neighbors
        // affected: the new b (was b, now E) and a and d
        if (pool.ring_size(r) >= 4)
        {
            pq.push(candidate_at(pool, cand.a));
            pq.push(candidate_at(pool, cand.b));
            // also the vertex before a and after d
            pq.push(candidate_at(pool, pool.prev_of(cand.a)));
            pq.push(candidate_at(pool, cand.d));
        }
    }
    return total_displacement;
}